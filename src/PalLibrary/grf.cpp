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

#include "pallib.h"
using namespace Pal::Tools;

Pal::Tools::CGameResourceFile::CGameResourceFile()	:
m_iErrorCode(GRF_ERROR_CODE_SUCCESSFUL),
m_pszDataFileName(NULL), m_pszIndexFileName(NULL)
{
	memset(&m_Header, 0, sizeof(GRF_HEADER));
}

Pal::Tools::CGameResourceFile::~CGameResourceFile()
{
	if (m_pszIndexFileName)
		delete [] m_pszIndexFileName;
	if (m_pszDataFileName)
		delete [] m_pszDataFileName;
}

bool Pal::Tools::CGameResourceFile::SetResourceFilePathAndName(const char* pszFilePath,
															   const char* pszFileName,
															   bool bCreateOnNotExist)
{
	char* fulldataname;
	char* fullfilename;
	int iFileHandle;
	GRF_HEADER header;
	size_t pathlen, namelen, addlen = 0;

	// 检查传入的文件名是否为空
	if (pszFileName == NULL || (namelen = strlen(pszFileName)) == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}

	// 如果传入了路径
	if (pszFilePath != NULL && (pathlen = strlen(pszFilePath)) > 0)
	{
		char* curpath;
		char* newpath;
		char* pch;

		// 对传入的路径进行检查
		if ((newpath = new char [pathlen + 2]) == NULL)
		{
			// 没有足够的内存用于分配缓冲区
			m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
			return false;
		}
		strcpy(newpath, pszFilePath);
		newpath[pathlen + 1] = '\0';

		// 取当前工作目录以备用
		if ((curpath = getcwd(NULL, 0)) == NULL)
		{
			// 没有足够的内存用于分配缓冲区
			delete [] newpath;

			m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
			return false;
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
				bool flag = true;

				if (bCreateOnNotExist)
				{
					// 尝试创建目录
#					if	defined(WIN32)
					if (mkdir(newpath) == 0)
#					else
					if (mkdir(newpath, 0777) == 0)
#					endif
						flag = false;
				}
				
				if (flag)
				{
					// 不创建目录，或创建目录时发生错误，则应当返回错误
					m_iErrorCode = GRF_ERROR_CODE_PATHNOTEXIST;
					free(curpath);
					delete [] newpath;
					return false;
				}
			}

			// 该层可达，或者已经创建了该层目录
			*(pch + 1) = ch1;

			// 是否已经检查到传入路径的结尾
			if (ch0 == '\0')
				break;
		}

		// 重新新当前目录设置为之前的目录
		chdir(curpath);

		free(curpath);
		delete [] newpath;
	}
	else
		pathlen = 0;

	// 如果传入了路径，则检查路径是否以分隔符结尾
	if (pathlen > 0)
		addlen = (pszFilePath[pathlen - 1] == '\\' || pszFilePath[pathlen - 1] == '/') ? 0 : 1;

	// 为文件的完整路径名分配空间
	if ((fullfilename = new char [pathlen + namelen + addlen + 1]) == NULL)
	{
		m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
		return false;
	}

	// 首先复制路径名
	if (pathlen > 0)
		strcpy(fullfilename, pszFilePath);
	// 其次决定是否补上分隔符
	if (addlen > 0)
#		if	defined(WIN32)
		fullfilename[pathlen] = '\\';
#		else
		fullfilename[pathlen] = '/';
#		endif
	// 最后补上文件名
	strcpy(fullfilename + pathlen + addlen, pszFileName);

	// 通过打开文件来检查文件是否存在
	if ((iFileHandle = open(fullfilename, O_BINARY | O_RDONLY)) == -1)
	{
		bool flag = true;

		if (bCreateOnNotExist)
		{
			// 尝试创建文件
			if ((iFileHandle = open(fullfilename, O_BINARY | O_WRONLY | O_CREAT, S_IREAD | S_IWRITE)) != -1)
			{
				strcpy(header.Signature, "GRF");
				header.FileLength = sizeof(GRF_HEADER);
				header.EntryCount = header.DataOffset = 0;
				if (write(iFileHandle, &header, sizeof(GRF_HEADER)) == sizeof(GRF_HEADER))
					flag = false;
			}
		}

		if (flag)
		{
			// 文件不存在或者是创建失败或者写入失败
			delete [] fullfilename;

			m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
			return false;
		}
	}
	else
	{
		// 文件存在，检查文件是否合法
		if (read(iFileHandle, &header, sizeof(GRF_HEADER)) < sizeof(GRF_HEADER) ||
			strncmp(header.Signature, "GRF", 4) != 0)
		{
			// 文件存在但不合法
			m_iErrorCode = GRF_ERROR_CODE_INVAILDFILE;
			close(iFileHandle);
			delete [] fullfilename;
			return false;
		}
	}
	close(iFileHandle);

	// 为完整的数据文件名分配空间
	if ((fulldataname = new char [pathlen + addlen + 9]) == NULL)
	{
		m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
		delete [] fullfilename;
		return false;
	}
	if (pathlen > 0)
		strncpy(fulldataname, fullfilename, pathlen + addlen);
	strcpy(fulldataname + pathlen + addlen, "00000000");

	// 如果原先已经有存储的文件名，则先行释放其空间
	if (m_pszIndexFileName)
		delete [] m_pszIndexFileName;
	if (m_pszDataFileName)
		delete [] m_pszDataFileName;

	// 将文件名存储于成员变量中
	m_pszIndexFileName = fullfilename;
	m_pszDataFileName = fulldataname;
	// 将文件头内容存储于成员变量中
	m_Header = header;

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

void Pal::Tools::CGameResourceFile::IntegerToHexString(uint32 ui, char* pchBuffer)
{
	static const char* pchNumbers = "0123456789ABCDEF";
	char* pch = pchBuffer + 7;
	for(int i = 0; i < 8; i++, ui >>= 4)
		*pch-- = pchNumbers[ui & 0xF];
}

bool Pal::Tools::CGameResourceFile::LoadIndices(void*& pvIndices, uint32& uiLength)
{
	void* buf;
	uint32 len;
	int iFileHandle;

	// 项数为零
	if (m_Header.EntryCount == 0)
	{
		uiLength = 0;
		pvIndices = NULL;
		return true;
	}

	// 分配空间
	if (m_Header.DataOffset > 0)
		len = m_Header.DataOffset - sizeof(GRF_HEADER);
	else
		len = m_Header.FileLength - sizeof(GRF_HEADER);
	if ((buf = malloc(len)) == NULL)
	{
		m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
		return false;
	}

	// 读取内容
	if ((iFileHandle = open(m_pszIndexFileName, O_BINARY | O_RDONLY)) == -1)
	{
		m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
		free(buf);
		return false;
	}
	if (lseek(iFileHandle, sizeof(GRF_HEADER), SEEK_SET) < sizeof(GRF_HEADER) ||
		read(iFileHandle, buf, len) < (int)len)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDFILE;
		close(iFileHandle);
		free(buf);
		return false;
	}
	close(iFileHandle);

	pvIndices = buf;
	uiLength = len;

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFile::LoadEntryData(const INDEX_ENTRY& entry, uint32 uiOffset, void* pvData, uint32 uiLength)
{
	uint32 length;
	int iFileHandle;

	// 项数为零
	if (m_Header.EntryCount == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}

	// 参数错误
	if (uiOffset >= entry.Length || uiLength == 0 || pvData == NULL)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}

	// 计算长度
	if (uiOffset + uiLength > entry.Length)
		length = entry.Length - uiOffset;
	else
		length = uiLength;

	// 读取内容
	if (m_Header.DataOffset > 0)
	{
		if ((iFileHandle = open(m_pszIndexFileName, O_BINARY | O_RDONLY)) == -1)
		{
			m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
			return false;
		}
		if (lseek(iFileHandle, entry.Offset + uiOffset, SEEK_SET) < (long)(entry.Offset + uiOffset) ||
			read(iFileHandle, pvData, length) < (int)length)
		{
			m_iErrorCode = GRF_ERROR_CODE_INVAILDFILE;
			close(iFileHandle);
			return false;
		}
	}
	else
	{
		size_t len = strlen(m_pszDataFileName);
		IntegerToHexString(entry.FileIndex, m_pszDataFileName + len - 8);
		if ((iFileHandle = open(m_pszDataFileName, O_BINARY | O_RDONLY)) == -1)
		{
			m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
			return false;
		}
		if (lseek(iFileHandle, uiOffset, SEEK_SET) < (long)uiOffset ||
			read(iFileHandle, pvData, length) < (int)length)
		{
			m_iErrorCode = GRF_ERROR_CODE_INVAILDFILE;
			close(iFileHandle);
			return false;
		}
	}
	close(iFileHandle);

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}
