/*
 * PAL library GRF format base class
 * 
 * Author: Lou Yihua <louyihua@21cn.com>
 *
 * Copyright 2007 Lou Yihua
 *
 * This file is part of PAL library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

 *
 *《仙剑奇侠传》库 GRF 格式基础类
 *
 * 作者： 楼奕华 <louyihua@21cn.com>
 *
 * 版权所有 2007 楼奕华
 *
 * 本文件是《仙剑奇侠传》库的一部分。
 *
 * 这个库属于自由软件，你可以遵照自由软件基金会出版的GNU次通用公共许可证条
 * 款来修改和重新发布这一程序。或者用许可证2.1版，或者（根据你的选择）用任
 * 何较新的版本。发布这一库的目的是希望它有用，但没有任何担保。甚至没有适合
 * 特定目的隐含的担保。更详细的情况请参阅GNU次通用公共许可证。
 * 
 * 你应该已经和库一起收到一份GNU次通用公共许可证的拷贝。如果还没有，写信给
 * 自由软件基金会：51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#if	defined(WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "pallib.h"

#define	GRF_FLAG_EOF			0x1L	//到达EOF
#define	GRF_FLAG_STANDALONE		0x2L	//独立索引&存储
#define	GRF_FLAG_CREATENEW		0x4L	//创建新文件
#define	GRF_FLAG_MODIFIED		0x8L	//有修改

#if	defined(WIN32)
#	define	commit(x)	_commit(x)
#else
#	define	commit(x)	fdatasync(x)
#endif

//内部使用函数

static inline char* _icheckpath(const char* pszFilePath)
{
	size_t pathlen;
	char* newpath;

	if (pszFilePath != NULL && (pathlen = strlen(pszFilePath)) > 0)
	{
		char* curpath;
		char* pch;

		// 对传入的路径进行检查
		if ((newpath = (char*)malloc(pathlen + 2)) == NULL)
			return NULL;
		strcpy(newpath, pszFilePath);
		newpath[pathlen + 1] = '\0';

		// 取当前工作目录以备用
		if ((curpath = getcwd(NULL, 0)) == NULL)
		{
			// 没有足够的内存用于分配缓冲区
			free(newpath);
			return NULL;
		}	

		// 检查路径的每一层
		for(pch = newpath; ; pch++)
		{
			char ch0 = *pch, ch1 = *(pch + 1);

			// 遇到的不是路径分隔符或结束符，则继续下一个字符
			if (ch0 != '\\' && ch0 != '/' && ch0 != '\0')
				continue;

			// 将路径暂时限定到当前检查的位置
			*(pch + 1) = '\0';

			// 检查该层是否可达
			if (chdir(newpath) != 0)
			{
				free(curpath);
				free(newpath);
				return NULL;
			}

			// 该层可达，或者已经创建了该层目录
			*(pch + 1) = ch1;

			// 是否已经检查到传入路径的结尾
			if (ch0 == '\0')
				break;
		}

		// 重新将当前目录设置为之前的目录
		chdir(curpath);
		free(curpath);

		//为路径末尾补分隔符
		if (newpath[pathlen - 1] != '\\' && newpath[pathlen - 1] != '/')
		{
#if	defined(WIN32) || defined(DOS)
			newpath[pathlen] = '\\';
#else
			newpath[pathlen] = '/';
#endif
		}
		return newpath;
	}
	else
	{
		if ((newpath = (char*)malloc(1)) == NULL)
			return NULL;
		else
		{
			newpath[0] = '\0';
			return newpath;
		}
	}
}

static bool _iGRFseekfile(GRFFILE* stream, const char* name)
{
	INDEX_ENTRY* ptr;
	uint32 len;

	//检查参数
	if (stream->pie == NULL || (len = (uint32)strlen(name)) == 0)
	{
		stream->error = EINVAL;
		return false;
	}

	//移动指针至起始
	ptr = (INDEX_ENTRY*)((GRF_HEADER*)stream->pie + 1);

	//开始查找指定的项
	for(uint32 index = 0; index < ((GRF_HEADER*)stream->pie)->EntryCount; index++)
	{
		//比较名字
		if (len == ptr->PathLength && strncmp(ptr->EntryPath, name, len) == 0)
		{
			stream->ieptr = ptr;
			return true;
		}
		else
			ptr = (INDEX_ENTRY*)((uint8*)(ptr + 1) + ptr->PathLength);
	}
	return false;
}

//公共函数

bool Pal::Tools::GRF::GRFopen(const char* grffile, const char* base, int mode, GRFFILE*& stream)
{
	GRFFILE* grf;
	char* _base;
	void* ptr;
	int fd, _mode = O_BINARY | O_RDWR;
	long flen;
	uint32 flag;

	//检查输入参数
	if (grffile == NULL || (_base = _icheckpath(base)) == NULL)
		return false;

	//设置打开模式
	if ((mode & O_CREAT) != 0)
		_mode &= O_CREAT;
	if ((mode & O_TRUNC) != 0)
		_mode &= O_TRUNC;
	//尝试打开索引文件
	if ((fd = open(grffile, _mode, S_IREAD | S_IWRITE)) == -1)
	{
		free(_base);
		return false;
	}
	//取文件长度
	if (lseek(fd, 0, SEEK_END) == -1 ||
		(flen = tell(fd)) == -1)
	{
		close(fd);
		free(_base);
		return false;
	}
	if (flen == 0)
	{
		//长度为 0，表明新建或截断
		//分配空间
		if ((ptr = malloc(sizeof(GRF_HEADER))) == NULL)
		{
			close(fd);
			free(_base);
			return false;
		}
		//填充内容
		memset(ptr, 0, sizeof(GRF_HEADER));
		strcpy((char*)ptr, "GRF");
		((GRF_HEADER*)ptr)->FileLength = sizeof(GRF_HEADER);
		//写回文件
		if (write(fd, ptr, sizeof(GRF_HEADER)) < sizeof(GRF_HEADER))
		{
			free(ptr);
			close(fd);
			free(_base);
			return false;
		}
		commit(fd);
		//设置标志
		flag = GRF_FLAG_STANDALONE | GRF_FLAG_CREATENEW;
	}
	else
	{
		//打开已有文件
		GRF_HEADER hdr;
		uint32 len;

		//判断格式并分配空间
		if (read(fd, &hdr, sizeof(GRF_HEADER)) < sizeof(GRF_HEADER) ||
			strcmp(hdr.Signature, "GRF") != 0 || lseek(fd, 0, SEEK_SET) == -1)
		{
			close(fd);
			free(_base);
			return false;
		}
		len = hdr.DataOffset == 0 ? hdr.FileLength : hdr.DataOffset;
		if ((ptr = malloc(len)) == NULL)
		{
			close(fd);
			free(_base);
			return false;
		}
		//读取内容
		if ((uint32)read(fd, ptr, len) < len)
		{
			free(ptr);
			close(fd);
			free(_base);
			return false;
		}
		if (hdr.DataOffset == 0)
			flag = GRF_FLAG_STANDALONE;
		else
			flag = 0;
	}

	//为结构分配空间并填充
	if ((grf = (GRFFILE*)malloc(sizeof(GRFFILE))) == NULL)
	{
		free(ptr);
		close(fd);
		free(_base);
		return false;
	}
	else
	{
		memset(grf, 0, sizeof(GRFFILE));
		grf->fd = fd;
		grf->base = _base;
		grf->pie = ptr;
		grf->ieptr = (INDEX_ENTRY*)((GRF_HEADER*)ptr + 1);
		grf->flag = flag;
		grf->pos = ((GRF_HEADER*)ptr)->DataOffset;
		stream = grf;
		return true;
	}
}

bool Pal::Tools::GRF::GRFclose(GRFFILE* stream)
{
	int fd;

	if (stream == NULL)
		return false;
	GRFflush(stream);
	if (stream->base)
		free(stream->base);
	if (stream->pie)
		free(stream->pie);
	fd = stream->fd;
	free(stream);
	if (fd != -1)
		return (close(fd) != -1);
	else
		return true;
}

bool Pal::Tools::GRF::GRFflush(GRFFILE* stream)
{
	if (stream == NULL)
		return false;
	if (stream->fd == -1)
	{
		stream->error = EBADF;
		return false;
	}
	if (stream->pie == NULL)
	{
		stream->error = EINVAL;
		return false;
	}

	if ((stream->flag & GRF_FLAG_MODIFIED) != 0)
	{
		uint32 len = ((GRF_HEADER*)stream->pie)->FileLength;

		if (lseek(stream->fd, 0, SEEK_SET) == -1 ||
			(uint32)write(stream->fd, stream->pie, len) < len)
			return false;
		stream->flag &= ~GRF_FLAG_MODIFIED;
	}

	return true;
}

bool Pal::Tools::GRF::GRFgettype(GRFFILE* stream, int& type)
{
	if (stream == NULL)
		return false;

	if ((stream->flag & GRF_FLAG_STANDALONE) == 0)
		type = GRF_TYPE_INTEGRATE;
	else
		type = GRF_TYPE_STANDALONE;
	return true;
}

bool Pal::Tools::GRF::GRFenumname(GRFFILE* stream, const char* prevname, char*& nextname)
{
	INDEX_ENTRY* ptr;
	char* name;

	//检查参数
	if (stream == NULL)
		return false;

	//查找项
	if (prevname == NULL)
		ptr = (INDEX_ENTRY*)((GRF_HEADER*)stream->pie + 1);
	else
	{
		INDEX_ENTRY* oldptr = stream->ieptr;
		if (!_iGRFseekfile(stream, prevname))
			return false;
		else
		{
			ptr = (INDEX_ENTRY*)((uint8*)(stream->ieptr + 1) + stream->ieptr->PathLength);
			stream->ieptr = oldptr;
		}
	}

	if ((uint32)((uint8*)ptr - (uint8*)stream->pie) >= ((GRF_HEADER*)stream->pie)->FileLength)
	{
		nextname = NULL;
		return true;
	}
	else if ((name = (char*)malloc(ptr->PathLength + 1)) == NULL)
		return false;
	else
	{
		memcpy(name, ptr->EntryPath, stream->ieptr->PathLength);
		name[ptr->PathLength] = '\0';
		nextname = name;
		return true;
	}
}

int Pal::Tools::GRF::GRFerror(GRFFILE* stream)
{
	if (stream == NULL)
		return 0;
	return stream->error;
}

void Pal::Tools::GRF::GRFclearerr(GRFFILE* stream)
{
	if (stream == NULL)
		return;
	stream->error = 0;
	stream->flag &= ~GRF_FLAG_EOF;
}

//独立版本函数

bool Pal::Tools::GRF::GRFopenfile(GRFFILE* stream, const char* name, const char* mode, FILE*& fpout)
{
	FILE* fp;
	char* path;

	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL || mode == NULL)
	{
		stream->error = EINVAL;
		return false;
	}
	//查找项
	if (!_iGRFseekfile(stream, name))
		return false;
	//找到项，为路径分配空间
	if ((path = (char*)malloc(strlen(stream->base) + strlen(name) + 1)) == NULL)
	{
		stream->error = errno;
		return false;
	}

	//构造路径
	strcpy(path, stream->base);
	strcat(path, name);
	//打开相应文件
	if (stream->flag & GRF_FLAG_CREATENEW)
		fp = fopen(path, "wb+");
	else
		fp = fopen(path, mode);
	free(path);
	if (fp == NULL)
	{
		stream->error = errno;
		return false;
	}
	else
	{
		fpout = fp;
		return true;
	}
}

bool Pal::Tools::GRF::GRFappendfile(GRFFILE* stream, const char* name)
{
	void* ptr;
	GRF_HEADER* hdr;
	size_t len;

	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL ||
		(len = strlen(name)) > 0xffff)
	{
		stream->error = EINVAL;
		return false;
	}
	//检查名字是否已存在
	if (_iGRFseekfile(stream, name))
	{
		stream->error = EEXIST;
		return false;
	}
	else if (stream->error != ENOENT)
		return false;

	//分配空间
	hdr = (GRF_HEADER*)stream->pie;
	if ((ptr = realloc(stream->pie,	hdr->FileLength + sizeof(INDEX_ENTRY) + len)) == NULL)
		return false;
	else
		stream->pie = ptr;
	//设置值
	stream->ieptr = (INDEX_ENTRY*)((uint8*)stream->pie + hdr->FileLength);
	stream->ieptr->Offset = stream->ieptr->Length = 0;
	stream->ieptr->ResourceType = GRF_RESOURCE_TYPE_NONE;
	stream->ieptr->CompressAlgorithm= GRF_COMPRESS_ALGORITHM_NONE;
	stream->ieptr->PathLength = (uint16)len;
	hdr->FileLength += sizeof(INDEX_ENTRY) + (uint32)len;
	hdr->EntryCount++;
	stream->flag |= GRF_FLAG_MODIFIED;

	return true;
}

bool Pal::Tools::GRF::GRFremovefile(GRFFILE* stream, const char* name)
{
	GRF_HEADER* hdr;
	INDEX_ENTRY* ptr;
	void* buf;
	size_t len, count;

	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL ||
		(len = strlen(name)) > 0xffff)
	{
		stream->error = EINVAL;
		return false;
	}
	//检查名字是否存在
	if (!_iGRFseekfile(stream, name))
		return false;

	//移除该项
	hdr = (GRF_HEADER*)stream->pie;
	ptr = (INDEX_ENTRY*)((uint8*)(stream->ieptr + 1) + len);
	count = (size_t)hdr->FileLength - (size_t)((uint8*)ptr - (uint8*)stream->pie);
	memmove(stream->ieptr, ptr, count);
	hdr->FileLength -= sizeof(INDEX_ENTRY) + (uint32)len;
	hdr->EntryCount--;
	stream->flag |= GRF_FLAG_MODIFIED;

	//调整分配的空间
	if ((buf = realloc(stream->pie, hdr->FileLength)) != NULL)
		stream->pie = buf;

	return true;
}

bool Pal::Tools::GRF::GRFrenamefile(GRFFILE* stream, const char* oldname, const char* newname)
{
	INDEX_ENTRY* ptr;
	void* buf;
	size_t count, len, len0, len1, len2;

	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || oldname == NULL || newname == NULL ||
		(len1 = strlen(oldname)) > 0xffff || (len2 = strlen(newname)) > 0xffff || len2 == 0)
	{
		stream->error = EINVAL;
		return false;
	}
	//检查原先的名字是否存在
	if (!_iGRFseekfile(stream, oldname))
		return false;
	//检查新的名字是否存在
	if (_iGRFseekfile(stream, newname))
	{
		stream->error = EEXIST;
		return false;
	}
	else if (stream->error != ENOENT)
		return false;

	//调整分配的空间
	len = ((GRF_HEADER*)stream->pie)->FileLength;
	len0 = len - len1 + len2;
	if ((buf = realloc(stream->pie, len0)) == NULL)
		return false;
	else
		stream->pie = buf;

	//为新名字移出空间
	ptr = stream->ieptr + 1;
	count = len - len1 - (size_t)((uint8*)ptr - (uint8*)stream->pie);
	memmove((uint8*)ptr + len2, (uint8*)ptr + len1, count);
	//复制新名字
	memcpy(ptr, newname, len2);
	((GRF_HEADER*)stream->pie)->FileLength = (uint32)len0;
	stream->flag |= GRF_FLAG_MODIFIED;

	return true;
}

bool Pal::Tools::GRF::GRFgetfileattr(GRFFILE* stream, const char* name, int attr, void* value)
{
	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL || value == NULL ||
		attr < GRF_ATTRIBUTE_MINIMUM || attr > GRF_ATTRIBUTE_MAXIMUM)
	{
		stream->error = EINVAL;
		return false;
	}
	//检查名字是否存在
	if (!_iGRFseekfile(stream, name))
		return false;

	//根据属性类型确定行为
	switch(attr)
	{
	case GRF_ATTRIBUTE_RESOURCETYPE:
		*(uint8*)value = stream->ieptr->ResourceType;
		break;
	case GRF_ATTRIBUTE_COMPRESSALGORITHM:
		*(uint8*)value = stream->ieptr->CompressAlgorithm;
		break;
	case GRF_ATTRIBUTE_NAMELENGTH:
		*(uint16*)value = stream->ieptr->PathLength;
		break;
	default:
		return false;
	}

	return true;
}

bool Pal::Tools::GRF::GRFsetfileattr(GRFFILE* stream, const char* name, int attr, const void* value)
{
	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL || value == NULL ||
		attr < GRF_ATTRIBUTE_MINIMUM || attr > GRF_ATTRIBUTE_MAXIMUM)
	{
		stream->error = EINVAL;
		return false;
	}
	//检查名字是否存在
	if (!_iGRFseekfile(stream, name))
		return false;

	//根据属性类型确定行为
	switch(attr)
	{
	case GRF_ATTRIBUTE_RESOURCETYPE:
		stream->ieptr->ResourceType = *(uint8*)value;
		break;
	case GRF_ATTRIBUTE_COMPRESSALGORITHM:
		stream->ieptr->CompressAlgorithm = *(uint8*)value;
		break;
	default:
		return false;
	}

	return true;
}

//集成版本函数

bool Pal::Tools::GRF::GRFseekfile(GRFFILE* stream, const char* name)
{
	INDEX_ENTRY* ptr;

	//检查参数
	if (stream == NULL)
		return false;
	if (name == NULL || (stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return false;
	}

	ptr = stream->ieptr;
	//查找项
	if (_iGRFseekfile(stream, name))
	{		
		if (lseek(stream->fd, stream->ieptr->Offset, SEEK_SET) == -1)
		{
			stream->ieptr = ptr;
			return false;
		}
		else
		{
			stream->pos = stream->ieptr->Offset;
			return true;
		}
	}
	else
		return false;
}

bool Pal::Tools::GRF::GRFeof(GRFFILE* stream)
{
	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return false;
	}
	return ((stream->flag & GRF_FLAG_EOF) != 0);
}

bool Pal::Tools::GRF::GRFseek(GRFFILE* stream, long offset, int origin, long& newpos)
{
	long pos, npos;

	//检查参数
	if (stream == NULL)
		return false;
	if (stream->fd == -1)
	{
		stream->error = EBADF;
		return false;
	}
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return false;
	}

	//设置起始点
	if (origin == SEEK_SET)
		pos = (long)((GRF_HEADER*)stream->pie)->DataOffset + offset;
	else if (origin == SEEK_CUR)
		pos = (long)stream->pos + offset;
	else if (origin == SEEK_END)
		pos = (long)(stream->ieptr->Offset + stream->ieptr->Length) + offset;
	else
		return false;

	//调整位置值
	if (pos > (long)(stream->ieptr->Offset + stream->ieptr->Length))
		pos = (long)(stream->ieptr->Offset + stream->ieptr->Length);
	else if (pos < (long)(stream->ieptr->Offset))
		pos = (long)(stream->ieptr->Offset);

	//移动文件指针
	if ((npos = lseek(stream->fd, pos, SEEK_SET)) == -1)
		return false;
	else
	{
		stream->pos = npos;
		newpos = (long)(npos - stream->ieptr->Offset);
		return true;
	}
}

bool Pal::Tools::GRF::GRFtell(GRFFILE* stream, long& pos)
{
	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return false;
	}

	pos = (long)(stream->pos - stream->ieptr->Offset);
	return true;
}

bool Pal::Tools::GRF::GRFread(GRFFILE* stream, void* buffer, uint32 size, uint32& actual)
{
	bool flag;
	uint32 ret;

	//检查参数
	if (stream == NULL)
		return false;
	if (stream->fd == -1)
	{
		stream->error = EBADF;
		return false;
	}
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return false;
	}

	if (flag = (size + stream->pos > stream->ieptr->Offset + stream->ieptr->Length))
		size = stream->ieptr->Offset + stream->ieptr->Length - stream->pos;
	if ((ret = (uint32)read(stream->fd, buffer, size)) == -1)
		return false;
	else
	{
		if (ret <= size && flag)
			stream->flag |= GRF_FLAG_EOF;
		else
			stream->flag &= ~GRF_FLAG_EOF;
		stream->pos += size;
		actual = ret;
		return true;
	}
}

bool Pal::Tools::GRF::GRFgetattr(GRFFILE* stream, int attr, void* value)
{
	//检查参数
	if (stream == NULL)
		return false;
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0 || value == NULL ||
		attr < GRF_ATTRIBUTE_MINIMUM || attr > GRF_ATTRIBUTE_MAXIMUM)
	{
		stream->error = EINVAL;
		return false;
	}

	//根据属性类型确定行为
	switch(attr)
	{
	case GRF_ATTRIBUTE_OFFSET:
		*(uint32*)value = stream->ieptr->Offset;
		break;
	case GRF_ATTRIBUTE_LENGTH:
		*(uint32*)value = stream->ieptr->Length;
		break;
	case GRF_ATTRIBUTE_RESOURCETYPE:
		*(uint8*)value = stream->ieptr->ResourceType;
		break;
	case GRF_ATTRIBUTE_COMPRESSALGORITHM:
		*(uint8*)value = stream->ieptr->CompressAlgorithm;
		break;
	case GRF_ATTRIBUTE_NAMELENGTH:
		*(uint16*)value = stream->ieptr->PathLength;
		break;
	case GRF_ATTRIBUTE_NAME:
		memcpy(value, stream->ieptr->EntryPath, stream->ieptr->PathLength);
		((char*)value)[stream->ieptr->PathLength] = '\0';
		break;
	}

	return true;
}
/*
bool Pal::Tools::GRF::GRFPackage(const char* pszGRF, const char* pszBasePath, const char* pszNewFile)
{
	GRFFILE* grf;
	int grftype, fd;
	char* name = NULL;
	void* buf;
	GRF_HEADER* hdr;
	INDEX_ENTRY* ptr;
	INDEX_ENTRY* ptrend;

	//检查输入参数
	if (pszGRF == NULL || pszNewFile == NULL || strlen(pszGRF) == 0 || strlen(pszNewFile) == 0)
		return false;

	//打开 GRF 文件
	if (!Pal::Tools::GRF::GRFopen(pszGRF, pszBasePath, 0, grf))
		return false;
	//检查 GRF 文件类型
	hdr = (GRF_HEADER*)grf->pie;
	Pal::Tools::GRF::GRFgettype(grf, grftype);
	if (grftype != GRF_TYPE_STANDALONE || hdr->EntryCount == 0)
	{
		Pal::Tools::GRF::GRFclose(grf);
		return true;
	}
	else
	{
		ptr = (INDEX_ENTRY*)(hdr + 1);
		ptrend = (INDEX_ENTRY*)((uint8*)grf->pie + hdr->FileLength);
	}

	//创建新的 GRF 文件
	if ((fd = open(pszNewFile, O_CREAT | O_TRUNC | O_BINARY | O_RDWR, S_IREAD | S_IWRITE)) == -1)
	{
		Pal::Tools::GRF::GRFclose(grf);
		return false;
	}

	//移动文件指针到数据区
	if (lseek(fd, hdr->FileLength, SEEK_SET) == -1)
	{
		close(fd);
		Pal::Tools::GRF::GRFclose(grf);
		return false;
	}
	//更新文件头
	hdr->DataOffset = hdr->FileLength;
	ptr->Offset = hdr->DataOffset;

	//开辟数据缓冲区
	if ((buf = malloc(0x4000)) == NULL)
	{
		close(fd);
		Pal::Tools::GRF::GRFclose(grf);
		return false;
	}
	//开辟名字缓冲区
	if ((name = malloc(ptr->PathLength + 1)) == NULL)
	{
		free(buf);
		close(fd);
		Pal::Tools::GRF::GRFclose(grf);
		return false;
	}

	while(ptr < ptrend)
	{
		//取得名字
		FILE* fp;
		void* temp;
		int dfd;
		uint32 ret, len = 0;

		if ((temp = realloc(name, ptr->PathLength + 1)) == NULL)
		{
			free(name);
			free(buf);
			close(fd);
			Pal::Tools::GRF::GRFclose(grf);
			return false;
		}
		name = (char*)temp;
		memcpy(name, ptr->EntryPath, ptr->PathLength);
		name[ptr->PathLength] = '\0';
		//打开文件
		if (!Pal::Tools::GRF::GRFopenfile(grf, name, "r+", fp))
			continue;
		setbuf(fp, NULL);
		dfd = fileno(fp);
		while(true)
		{
			if ((ret = (uint32)read(dfd, buf, 0x4000)) == -1)
				break;
			if (ret > 0)
			{
				if (write(fd, buf, ret) == -1)
					break;
				len += ret;
			}
			if (eof(fd))
				break;
		}

	}

	return true;
}

bool Pal::Tools::GRF::GRFExtract(const char* pszGRF, const char* pszBasePath, const char* pszNewFile)
{
	char* pszOldLocale;
	char* pszFile;
	size_t pathlen, namelen;

	//检查输入参数
	if (pszGRF == NULL || pszNewFile == NULL || strlen(pszGRF) == 0 || strlen(pszNewFile) == 0)
		return false;

	//首先将路径名复制到临时变量中
	if (pszBasePath == NULL || (pathlen = strlen(pszBasePath)) == 0)
	{
		pszFile = NULL;
		pathlen = 0;
	}
	else
	{
		if ((pszFile = (char*)malloc(pathlen + 1)) == NULL)
			return false;
		strcpy(pszFile, pszBasePath);
	}

	return true;
}
*/
