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

static inline errno_t _icheckpath(const char* pszFilePath, char*& pszPath)
{
	size_t pathlen;
	char* newpath;

	if (pszFilePath != NULL && (pathlen = strlen(pszFilePath)) > 0)
	{
		char* curpath;
		char* pch;

		// 对传入的路径进行检查
		if ((newpath = (char*)malloc(pathlen + 2)) == NULL)
			return ENOMEM;
		strcpy(newpath, pszFilePath);
		newpath[pathlen + 1] = '\0';

		// 取当前工作目录以备用
		if ((curpath = getcwd(NULL, 0)) == NULL)
		{
			// 没有足够的内存用于分配缓冲区
			free(newpath);
			return ENOMEM;
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
				return ENOENT;
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
		pszPath = newpath;
		return 0;
	}
	else
	{
		if ((newpath = (char*)malloc(1)) == NULL)
			return ENOMEM;
		else
		{
			newpath[0] = '\0';
			pszPath = newpath;
			return 0;
		}
	}
}

static errno_t _iGRFseekfile(GRFFILE* stream, const char* name)
{
	INDEX_ENTRY* ptr;
	uint32 len;

	//检查参数
	if (stream->pie == NULL || (len = (uint32)strlen(name)) == 0)
	{
		stream->error = EINVAL;
		return EINVAL;
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
			return 0;
		}
		else
			ptr = (INDEX_ENTRY*)((uint8*)(ptr + 1) + ptr->PathLength);
	}
	stream->error = ENOENT;
	return ENOENT;
}

//公共函数

errno_t Pal::Tools::GRF::GRFopen(const char* grffile, const char* base, bool create, bool truncate, GRFFILE*& stream)
{
	GRFFILE* grf;
	char* _base;
	void* ptr;
	int fd, _mode = O_BINARY;
	long flen;
	bool oflag;
	uint32 flag;
	errno_t err;

	//检查输入参数
	if (grffile == NULL)
		return EINVAL;
	if ((err = _icheckpath(base, _base)) != 0)
		return err;

	//尝试打开文件，以确定打开方式
	if ((fd = open(grffile, O_BINARY | O_RDONLY)) == -1)
		oflag = true;
	else
	{
		GRF_HEADER hdr;
		memset(&hdr, 0, sizeof(GRF_HEADER));
		read(fd, &hdr, sizeof(GRF_HEADER));
		if (hdr.DataOffset == 0)
			oflag = true;
		else
			oflag = false;
		close(fd);
	}

	//设置打开模式
	if (create)
		_mode |= O_CREAT;
	if (truncate)
	{
		_mode |= O_TRUNC;
		oflag = true;
	}
	_mode |= oflag ? O_RDWR : O_RDONLY;

	//尝试打开索引文件
	if ((fd = open(grffile, _mode, S_IREAD | S_IWRITE)) == -1)
	{
		err = errno;
		free(_base);
		return err;
	}
	//取文件长度
	if (lseek(fd, 0, SEEK_END) == -1 ||
		(flen = tell(fd)) == -1)
	{
		err = errno;
		close(fd);
		free(_base);
		return err;
	}
	if (flen == 0)
	{
		//长度为 0，表明新建或截断
		//分配空间
		if ((ptr = malloc(sizeof(GRF_HEADER))) == NULL)
		{
			err = errno;
			close(fd);
			free(_base);
			return err;
		}
		//填充内容
		memset(ptr, 0, sizeof(GRF_HEADER));
		strcpy((char*)ptr, "GRF");
		((GRF_HEADER*)ptr)->FileLength = sizeof(GRF_HEADER);
		//写回文件
		if (write(fd, ptr, sizeof(GRF_HEADER)) < sizeof(GRF_HEADER))
		{
			err = errno;
			free(ptr);
			close(fd);
			free(_base);
			return err;
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
			err = errno;
			close(fd);
			free(_base);
			return err;
		}
		len = hdr.DataOffset == 0 ? hdr.FileLength : hdr.DataOffset;
		if ((ptr = malloc(len)) == NULL)
		{
			err = errno;
			close(fd);
			free(_base);
			return err;
		}
		//读取内容
		if ((uint32)read(fd, ptr, len) < len)
		{
			err = errno;
			free(ptr);
			close(fd);
			free(_base);
			return err;
		}
		if (hdr.DataOffset == 0)
			flag = GRF_FLAG_STANDALONE;
		else
			flag = 0;
	}

	//为结构分配空间并填充
	if ((grf = (GRFFILE*)malloc(sizeof(GRFFILE))) == NULL)
	{
		err = errno;
		free(ptr);
		close(fd);
		free(_base);
		return err;
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
		return 0;
	}
}

errno_t Pal::Tools::GRF::GRFclose(GRFFILE* stream)
{
	int fd;

	if (stream == NULL)
		return EINVAL;
	GRFflush(stream);
	if (stream->base)
		free(stream->base);
	if (stream->pie)
		free(stream->pie);
	fd = stream->fd;
	free(stream);
	if (fd != -1)
	{
		close(fd);
		return errno;
	}
	else
		return 0;
}

errno_t Pal::Tools::GRF::GRFflush(GRFFILE* stream)
{
	if (stream == NULL)
		return EINVAL;
	if (stream->fd == -1)
	{
		stream->error = EBADF;
		return EBADF;
	}
	if (stream->pie == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}

	if ((stream->flag & GRF_FLAG_MODIFIED) != 0)
	{
		uint32 len = ((GRF_HEADER*)stream->pie)->FileLength;

		if (lseek(stream->fd, 0, SEEK_SET) == -1 ||
			(uint32)write(stream->fd, stream->pie, len) < len)
		{
			stream->error = errno;
			return stream->error;
		}
		stream->flag &= ~GRF_FLAG_MODIFIED;
	}

	return 0;
}

errno_t Pal::Tools::GRF::GRFgettype(GRFFILE* stream, int& type)
{
	if (stream == NULL)
		return EINVAL;

	if ((stream->flag & GRF_FLAG_STANDALONE) == 0)
		type = GRF_TYPE_INTEGRATE;
	else
		type = GRF_TYPE_STANDALONE;
	return 0;
}

errno_t Pal::Tools::GRF::GRFenumname(GRFFILE* stream, const char* prevname, char*& nextname)
{
	INDEX_ENTRY* ptr;
	char* name;
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;

	//查找项
	if (prevname == NULL)
		ptr = (INDEX_ENTRY*)((GRF_HEADER*)stream->pie + 1);
	else
	{
		INDEX_ENTRY* oldptr = stream->ieptr;
		if ((err = _iGRFseekfile(stream, prevname)) != 0)
		{
			stream->error = err;
			return err;
		}
		else
		{
			ptr = (INDEX_ENTRY*)((uint8*)(stream->ieptr + 1) + stream->ieptr->PathLength);
			stream->ieptr = oldptr;
		}
	}

	if ((uint32)((uint8*)ptr - (uint8*)stream->pie) >= ((GRF_HEADER*)stream->pie)->FileLength)
	{
		if (nextname)
			free(nextname);
		nextname = NULL;
		return 0;
	}
	else if ((name = (char*)realloc(nextname, ptr->PathLength + 1)) == NULL)
	{
		stream->error = ENOMEM;
		return ENOMEM;
	}
	else
	{
		memcpy(name, ptr->EntryPath, stream->ieptr->PathLength);
		name[ptr->PathLength] = '\0';
		nextname = name;
		return 0;
	}
}

errno_t Pal::Tools::GRF::GRFerror(GRFFILE* stream)
{
	if (stream == NULL)
		return EINVAL;
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

errno_t Pal::Tools::GRF::GRFgetfilename(GRFFILE* stream, const char* name, char*& filename, bool& forcenew)
{
	char* path;
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	//查找项
	if ((err = _iGRFseekfile(stream, name)) != 0)
	{
		stream->error = err;
		return err;
	}
	//找到项，为路径分配空间
	if ((path = (char*)malloc(strlen(stream->base) + strlen(name) + 1)) == NULL)
	{
		stream->error = errno;
		return stream->error;
	}

	//构造路径
	strcpy(path, stream->base);
	strcat(path, name);
	filename = path;
	forcenew = ((stream->flag & GRF_FLAG_CREATENEW) != 0);
	return 0;
}

errno_t Pal::Tools::GRF::GRFopenfile(GRFFILE* stream, const char* name, const char* mode, FILE*& fpout)
{
	FILE* fp;
	char* path;
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL || mode == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	//查找项
	if ((err = _iGRFseekfile(stream, name)) != 0)
	{
		stream->error = err;
		return err;
	}
	//找到项，为路径分配空间
	if ((path = (char*)malloc(strlen(stream->base) + strlen(name) + 1)) == NULL)
	{
		stream->error = errno;
		return stream->error;
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
		return stream->error;
	}
	else
	{
		fpout = fp;
		return 0;
	}
}

errno_t Pal::Tools::GRF::GRFappendfile(GRFFILE* stream, const char* name)
{
	void* ptr;
	GRF_HEADER* hdr;
	size_t len;
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	if ((len = strlen(name)) > 0xffff)
	{
		stream->error = ERANGE;
		return ERANGE;
	}
	//检查名字是否已存在
	if ((err = _iGRFseekfile(stream, name)) == 0)
	{
		stream->error = EEXIST;
		return EEXIST;
	}
	else if (err != ENOENT)
		return err;

	//分配空间
	hdr = (GRF_HEADER*)stream->pie;
	if ((ptr = realloc(stream->pie,	hdr->FileLength + sizeof(INDEX_ENTRY) + len)) == NULL)
	{
		stream->error = ENOMEM;
		return ENOMEM;
	}
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

	return 0;
}

errno_t Pal::Tools::GRF::GRFremovefile(GRFFILE* stream, const char* name)
{
	GRF_HEADER* hdr;
	INDEX_ENTRY* ptr;
	void* buf;
	size_t len, count;
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	if ((len = strlen(name)) > 0xffff)
	{
		stream->error = ERANGE;
		return ERANGE;
	}
	//检查名字是否存在
	if ((err = _iGRFseekfile(stream, name)) != 0)
		return err;

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

	return 0;
}

errno_t Pal::Tools::GRF::GRFrenamefile(GRFFILE* stream, const char* oldname, const char* newname)
{
	INDEX_ENTRY* ptr;
	void* buf;
	size_t count, len, len0, len1, len2;
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || oldname == NULL || newname == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	if ((len1 = strlen(oldname)) > 0xffff || (len2 = strlen(newname)) > 0xffff || len2 == 0)
	{
		stream->error = ERANGE;
		return ERANGE;
	}
	//检查原先的名字是否存在
	if ((err = _iGRFseekfile(stream, oldname)) != 0)
		return err;
	//检查新的名字是否存在
	if ((err = _iGRFseekfile(stream, newname)) == 0)
	{
		stream->error = EEXIST;
		return EEXIST;
	}
	else if (err != ENOENT)
		return err;

	//调整分配的空间
	len = ((GRF_HEADER*)stream->pie)->FileLength;
	len0 = len - len1 + len2;
	if ((buf = realloc(stream->pie, len0)) == NULL)
	{
		stream->error = ENOMEM;
		return ENOMEM;
	}
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

	return 0;
}

errno_t Pal::Tools::GRF::GRFgetfileattr(GRFFILE* stream, const char* name, int attr, void* value)
{
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL || value == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	if (attr < GRF_ATTRIBUTE_MINIMUM || attr > GRF_ATTRIBUTE_MAXIMUM)
	{
		stream->error = ERANGE;
		return ERANGE;
	}
	//检查名字是否存在
	if ((err = _iGRFseekfile(stream, name)) != 0)
		return err;

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
		stream->error = ERANGE;
		return ERANGE;
	}

	return 0;
}

errno_t Pal::Tools::GRF::GRFsetfileattr(GRFFILE* stream, const char* name, int attr, const void* value)
{
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) == 0 || name == NULL || value == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	if (attr < GRF_ATTRIBUTE_MINIMUM || attr > GRF_ATTRIBUTE_MAXIMUM)
	{
		stream->error = ERANGE;
		return ERANGE;
	}
	//检查名字是否存在
	if ((err = _iGRFseekfile(stream, name)) != 0)
		return err;

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
		stream->error = ERANGE;
		return ERANGE;
	}

	return 0;
}

//集成版本函数

errno_t Pal::Tools::GRF::GRFseekfile(GRFFILE* stream, const char* name)
{
	INDEX_ENTRY* ptr;
	errno_t err;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if (name == NULL || (stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return EINVAL;
	}

	ptr = stream->ieptr;
	//查找项
	if ((err = _iGRFseekfile(stream, name)) == 0)
	{		
		if (lseek(stream->fd, stream->ieptr->Offset, SEEK_SET) == -1)
		{
			stream->ieptr = ptr;
			stream->error = ENOENT;
			return ENOENT;
		}
		else
		{
			stream->pos = stream->ieptr->Offset;
			return 0;
		}
	}
	else
		return err;
}

errno_t Pal::Tools::GRF::GRFeof(GRFFILE* stream)
{
	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	return ((stream->flag & GRF_FLAG_EOF) != 0) ? EOF : 0;
}

errno_t Pal::Tools::GRF::GRFseek(GRFFILE* stream, long offset, int origin, long& newpos)
{
	long pos, npos;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if (stream->fd == -1)
	{
		stream->error = EBADF;
		return EBADF;
	}
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return EINVAL;
	}

	//设置起始点
	if (origin == SEEK_SET)
		pos = (long)((GRF_HEADER*)stream->pie)->DataOffset + offset;
	else if (origin == SEEK_CUR)
		pos = (long)stream->pos + offset;
	else if (origin == SEEK_END)
		pos = (long)(stream->ieptr->Offset + stream->ieptr->Length) + offset;
	else
	{
		stream->error = ERANGE;
		return ERANGE;
	}

	//调整位置值
	if (pos > (long)(stream->ieptr->Offset + stream->ieptr->Length))
		pos = (long)(stream->ieptr->Offset + stream->ieptr->Length);
	else if (pos < (long)(stream->ieptr->Offset))
		pos = (long)(stream->ieptr->Offset);

	//移动文件指针
	if ((npos = lseek(stream->fd, pos, SEEK_SET)) == -1)
	{
		stream->error = errno;
		return stream->error;
	}
	else
	{
		stream->pos = npos;
		newpos = (long)(npos - stream->ieptr->Offset);
		return 0;
	}
}

errno_t Pal::Tools::GRF::GRFtell(GRFFILE* stream, long& pos)
{
	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return EINVAL;
	}

	pos = (long)(stream->pos - stream->ieptr->Offset);
	return 0;
}

errno_t Pal::Tools::GRF::GRFread(GRFFILE* stream, void* buffer, uint32 size, uint32& actual)
{
	bool flag;
	uint32 ret;

	//检查参数
	if (stream == NULL)
		return EINVAL;
	if (stream->fd == -1)
	{
		stream->error = EBADF;
		return EBADF;
	}
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0)
	{
		stream->error = EINVAL;
		return EINVAL;
	}

	if (flag = (size + stream->pos > stream->ieptr->Offset + stream->ieptr->Length))
		size = stream->ieptr->Offset + stream->ieptr->Length - stream->pos;
	if ((ret = (uint32)read(stream->fd, buffer, size)) == -1)
	{
		stream->error = errno;
		return stream->error;
	}
	else
	{
		if (ret <= size && flag)
			stream->flag |= GRF_FLAG_EOF;
		else
			stream->flag &= ~GRF_FLAG_EOF;
		stream->pos += size;
		actual = ret;
		return 0;
	}
}

errno_t Pal::Tools::GRF::GRFgetattr(GRFFILE* stream, int attr, void* value)
{
	//检查参数
	if (stream == NULL)
		return EINVAL;
	if ((stream->flag & GRF_FLAG_STANDALONE) != 0 || value == NULL)
	{
		stream->error = EINVAL;
		return EINVAL;
	}
	if (attr < GRF_ATTRIBUTE_MINIMUM || attr > GRF_ATTRIBUTE_MAXIMUM)
	{
		stream->error = ERANGE;
		return ERANGE;
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

	return 0;
}

errno_t Pal::Tools::GRF::GRFPackage(const char* pszGRF, const char* pszBasePath, const char* pszNewFile)
{
	int fdold, fdnew;
	size_t pathlen;
	char* name;
	void* buf;
	bool flag;
	long idxpos;
	GRF_HEADER hdr;
	INDEX_ENTRY cur, old;
	errno_t err;

	//检查输入参数
	if (pszGRF == NULL || pszNewFile == NULL || strlen(pszGRF) == 0 || strlen(pszNewFile) == 0)
		return EINVAL;
	if ((err = _icheckpath(pszBasePath, name)) != 0)
		return err;

	//打开原 GRF 文件
	if ((fdold = open(pszGRF, O_BINARY | O_RDONLY)) == -1)
	{
		err = errno;
		free(name);
		return err;
	}
	else
	{
		//检查 GRF 文件
		if (read(fdold, &hdr, sizeof(GRF_HEADER)) < sizeof(GRF_HEADER) ||
			memcmp(hdr.Signature, "GRF", 4) != 0 ||
			hdr.DataOffset != 0 || hdr.EntryCount == 0)
		{
			err = errno;
			close(fdold);
			free(name);
			return err;
		}
		else
			hdr.DataOffset = hdr.FileLength;
	}
	//创建新的 GRF 文件
	if ((fdnew = open(pszNewFile, O_CREAT | O_TRUNC | O_BINARY | O_RDWR, S_IREAD | S_IWRITE)) == -1)
	{
		err = errno;
		close(fdold);
		free(name);
		return err;
	}
	memset(&old, 0, sizeof(INDEX_ENTRY));
	old.Offset = hdr.DataOffset;
	pathlen = strlen(name);
	idxpos = sizeof(GRF_HEADER);

	//开辟数据缓冲区
	if ((buf = malloc(0x4000)) == NULL)
	{
		err = errno;
		close(fdnew);
		close(fdold);
		free(name);
		return err;
	}
	else
		flag = true;

	for(uint32 i = 0; i < hdr.EntryCount; i++, old = cur)
	{
		void* temp;
		int fddat;
		long ret, datalen = 0;

		//读取索引项
		if (read(fdold, &cur, sizeof(INDEX_ENTRY)) < sizeof(INDEX_ENTRY))
		{
			err = errno;
			flag = false;
			break;
		}
		else
			cur.Offset = old.Offset + old.Length;
		//分配名字空间
		if ((temp = realloc(name, pathlen + cur.PathLength + 1)) == NULL)
		{
			err = errno;
			flag = false;
			break;
		}
		else
			name = (char*)temp;
		//读取名字并拼接到路径中
		if (read(fdold, name + pathlen, cur.PathLength) < cur.PathLength)
		{
			err = errno;
			flag = false;
			break;
		}
		name[pathlen + cur.PathLength] = '\0';
		//移动文件指针到数据区
		if (lseek(fdnew, cur.Offset, SEEK_SET) == -1)
		{
			err = errno;
			flag = false;
			break;
		}
		//打开数据文件
		if ((fddat = open(name, O_BINARY | O_RDONLY)) == -1)
			continue;
		while((ret = read(fddat, buf, 0x4000)) > 0)
		{
			if (write(fdnew, buf, ret) < ret)
			{
				err = errno;
				flag = false;
				break;
			}
			datalen += ret;
		}
		close(fddat);
		cur.Length = datalen;
		//移动文件指针到索引区
		if (lseek(fdnew, idxpos, SEEK_SET) == -1)
		{
			err = errno;
			flag = false;
			break;
		}
		//写入索引
		if (write(fdnew, &cur, sizeof(INDEX_ENTRY)) < sizeof(INDEX_ENTRY) ||
			write(fdnew, name + pathlen, cur.PathLength) < cur.PathLength)
		{
			err = errno;
			flag = false;
			break;
		}
		idxpos += sizeof(INDEX_ENTRY) + cur.PathLength;
	}
	//更新文件头
	hdr.FileLength = cur.Offset + cur.Length;
	if (lseek(fdnew, 0, SEEK_SET) != -1)
		write(fdnew, &hdr, sizeof(GRF_HEADER));

	//收尾工作
	free(buf);
	close(fdnew);
	close(fdold);
	free(name);
	return flag ? 0 : err;
}

errno_t Pal::Tools::GRF::GRFExtract(const char* pszGRF, const char* pszBasePath, const char* pszNewFile)
{
	int fdold, fdnew;
	size_t pathlen;
	char* name;
	void* buf;
	bool flag;
	long idxpos;
	GRF_HEADER hdr;
	INDEX_ENTRY cur;
	errno_t err;

	//检查输入参数
	if (pszGRF == NULL || pszNewFile == NULL || strlen(pszGRF) == 0 || strlen(pszNewFile) == 0)
		return EINVAL;
	if ((err = _icheckpath(pszBasePath, name)) != 0)
		return err;

	//打开原 GRF 文件
	if ((fdold = open(pszGRF, O_BINARY | O_RDONLY)) == -1)
	{
		err = errno;
		free(name);
		return err;
	}
	else
	{
		//检查 GRF 文件
		if (read(fdold, &hdr, sizeof(GRF_HEADER)) < sizeof(GRF_HEADER) ||
			memcmp(hdr.Signature, "GRF", 4) != 0 ||
			hdr.DataOffset == 0 || hdr.EntryCount == 0)
		{
			err = errno;
			close(fdold);
			free(name);
			return err;
		}
		else
			hdr.DataOffset = hdr.FileLength;
	}
	//创建新的 GRF 文件
	if ((fdnew = open(pszNewFile, O_CREAT | O_TRUNC | O_BINARY | O_RDWR, S_IREAD | S_IWRITE)) == -1)
	{
		err = errno;
		close(fdold);
		free(name);
		return err;
	}
	pathlen = strlen(name);
	idxpos = sizeof(GRF_HEADER);

	//开辟数据缓冲区
	if ((buf = malloc(0x4000)) == NULL)
	{
		err = errno;
		close(fdnew);
		close(fdold);
		free(name);
		return err;
	}
	else
		flag = true;

	for(uint32 i = 0; i < hdr.EntryCount; i++)
	{
		void* temp;
		int fddat;
		long ret, datalen;

		//移动文件指针到索引区
		if (lseek(fdold, idxpos, SEEK_SET) == -1)
		{
			err = errno;
			flag = false;
			break;
		}
		//读取索引项
		if (read(fdold, &cur, sizeof(INDEX_ENTRY)) < sizeof(INDEX_ENTRY))
		{
			err = errno;
			flag = false;
			break;
		}
		//分配名字空间
		if ((temp = realloc(name, pathlen + cur.PathLength + 1)) == NULL)
		{
			err = errno;
			flag = false;
			break;
		}
		else
			name = (char*)temp;
		//读取名字并拼接到路径中
		if (read(fdold, name + pathlen, cur.PathLength) < cur.PathLength)
		{
			err = errno;
			flag = false;
			break;
		}
		name[pathlen + cur.PathLength] = '\0';
		//移动文件指针到数据区
		if (lseek(fdold, cur.Offset, SEEK_SET) == -1)
		{
			err = errno;
			flag = false;
			break;
		}
		//打开数据文件
		if ((fddat = open(name, O_BINARY | O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE)) == -1)
			continue;
		//写入数据
		for(datalen = cur.Length; datalen > 0;)
		{
			if (datalen >= 0x4000)
				ret = read(fdold, buf, 0x4000);
			else
				ret = read(fdold, buf, datalen);
			if (ret > 0)
			{
				if (write(fddat, buf, ret) < ret)
				{
					err = errno;
					flag = false;
					break;
				}
				datalen -= ret;
			}
			else
				break;
		}
		close(fddat);
		cur.Offset = cur.Length = 0;
		//移动文件指针到索引区
		if (lseek(fdnew, idxpos, SEEK_SET) == -1)
		{
			err = errno;
			flag = false;
			break;
		}
		//写入索引
		if (write(fdnew, &cur, sizeof(INDEX_ENTRY)) < sizeof(INDEX_ENTRY) ||
			write(fdnew, name + pathlen, cur.PathLength) < cur.PathLength)
		{
			err = errno;
			flag = false;
			break;
		}
		idxpos += sizeof(INDEX_ENTRY) + cur.PathLength;
	}
	//更新文件头
	hdr.FileLength = idxpos;
	hdr.DataOffset = 0;
	if (lseek(fdnew, 0, SEEK_SET) != -1)
		write(fdnew, &hdr, sizeof(GRF_HEADER));

	//收尾工作
	free(buf);
	close(fdnew);
	close(fdold);
	free(name);
	return flag ? 0 : err;
}
