/*
 * PAL library GRF format Reader class
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
 *《仙剑奇侠传》库 GRF 格式 Reader 类
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

extern "C" static inline int CompareIndexEntry(const void* pElem1, const void* pElem2)
{
	uint32 len;
	INDEX_ENTRY* ptr1 = *((INDEX_ENTRY**)pElem1);
	INDEX_ENTRY* ptr2 = *((INDEX_ENTRY**)pElem2);

	if (ptr1->PathLength < ptr2->PathLength)
		len = ptr1->PathLength;
	else
		len = ptr2->PathLength;
	return wcsncmp(ptr1->EntryPath, ptr2->EntryPath, len);
}

Pal::Tools::CGameResourceFileReader::CGameResourceFileReader()	:
CGameResourceFile(), m_pvIndices(NULL), m_uiIndexLength(0),
m_uiCurrentEntry(0), m_pEntries(NULL)
{}

Pal::Tools::CGameResourceFileReader::~CGameResourceFileReader()
{
	if (m_pvIndices)
		free(m_pvIndices);
	if (m_pEntries)
		delete [] m_pEntries;
}

Pal::Tools::CGameResourceFileReader* Pal::Tools::CGameResourceFileReader::CreateReader(const char* pszFilePath,
																					   const char* pszFileName)
{
	INDEX_ENTRY* ptr;
	Pal::Tools::CGameResourceFileReader* reader = new Pal::Tools::CGameResourceFileReader();

	// 无法创建类
	if (reader == NULL)
		return NULL;

	// 没有找到资源
	if (!reader->SetResourceFilePathAndName(pszFilePath, pszFileName, false))
	{
		delete reader;
		return NULL;
	}

	// 没有项
	if (reader->m_Header.EntryCount == 0)
	{
		delete reader;
		return NULL;
	}

	// 无法读入索引
	if (!reader->LoadIndices(reader->m_pvIndices, reader->m_uiIndexLength))
	{
		delete reader;
		return NULL;
	}

	// 无法分配空间
	if ((reader->m_pEntries = new INDEX_ENTRY* [reader->m_Header.EntryCount]) == NULL)
	{
		delete reader;
		return NULL;
	}

	// 将索引项排序
	ptr = (INDEX_ENTRY*)reader->m_pvIndices;
	for(uint32 i = 0; i < reader->m_Header.EntryCount; i++)
	{
		reader->m_pEntries[i] = ptr;
		ptr = (INDEX_ENTRY*)((uint8*)ptr + sizeof(INDEX_ENTRY) + ptr->PathLength * sizeof(wchar_t));
	}
	qsort(reader->m_pEntries, reader->m_Header.EntryCount, sizeof(INDEX_ENTRY*), CompareIndexEntry);

	return reader;
}

bool Pal::Tools::CGameResourceFileReader::Seek(const wchar_t* pwszEntryName)
{
	uint32 len, bottom, top, ptr;
	int ret;

	// 检查参数
	if (pwszEntryName == NULL || ((len = (uint32)wcslen(pwszEntryName)) == 0))
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}

	// 搜索内容（二分法）
	for(bottom = 0, top = m_Header.EntryCount - 1; bottom <= top; )
	{
		ptr = (bottom + top) >> 1;
		ret = wcsncmp(pwszEntryName, m_pEntries[ptr]->EntryPath, m_pEntries[ptr]->PathLength);
		if (ret == 0)
		{
			if (len == m_pEntries[ptr]->PathLength)
			{
				m_uiCurrentEntry = ptr;

				m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
				return true;
			}
			else
			{
				m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTEXIST;
				return false;
			}
		}
		else if (ret < 0)
			top = ptr - 1;
		else
			bottom = ptr + 1;
	}

	m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTEXIST;
	return false;
}

bool Pal::Tools::CGameResourceFileReader::GetResourceType(uint8& type)
{
	type = m_pEntries[m_uiCurrentEntry]->ResourceType;

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileReader::GetCompressAlgorithm(uint8& algo)
{
	algo = m_pEntries[m_uiCurrentEntry]->CompressAlgorithm;

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileReader::GetNameLength(uint32& uiLength)
{
	uiLength = m_pEntries[m_uiCurrentEntry]->PathLength;

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileReader::GetEntryName(wchar_t* pwszEntryName, uint32 uiLength)
{
	// 参数非法
	if (pwszEntryName == NULL || uiLength == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}

	// 复制名字，确保以'\0'结尾
	if (uiLength > m_pEntries[m_uiCurrentEntry]->PathLength)
	{
		wcsncpy(pwszEntryName, m_pEntries[m_uiCurrentEntry]->EntryPath, m_pEntries[m_uiCurrentEntry]->PathLength);
		pwszEntryName[m_pEntries[m_uiCurrentEntry]->PathLength] = L'\0';
	}
	else
	{
		wcsncpy(pwszEntryName, m_pEntries[m_uiCurrentEntry]->EntryPath, uiLength - 1);
		pwszEntryName[uiLength - 1] = L'\0';
	}

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileReader::GetDataLength(uint32& uiLength)
{
	uiLength = m_pEntries[m_uiCurrentEntry]->Length;

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileReader::GetData(uint32 uiOffset, void* pvData, uint32 uiLength)
{
	return LoadEntryData(*m_pEntries[m_uiCurrentEntry], uiOffset, pvData, uiLength);
}
