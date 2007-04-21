/*
 * PAL library GRF format Editor class
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
 *《仙剑奇侠传》库 GRF 格式 Editor 类
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

Pal::Tools::CGameResourceFileEditor::CGameResourceFileEditor()	:
CGameResourceFile(), m_uiMaxFileIndex(0)
{}

Pal::Tools::CGameResourceFileEditor::~CGameResourceFileEditor()
{
	for(std::vector<std::list<IndexEntry> >::size_type i = 0;
		i < m_IndexVector.size(); i++)
		for(std::list<IndexEntry>::iterator iter = m_IndexVector[i].begin();
			iter != m_IndexVector[i].end(); iter++)
		{
			free(iter->pEntry);
			if (iter->pvData)
				free(iter->pvData);
		}
}

Pal::Tools::CGameResourceFileEditor* Pal::Tools::CGameResourceFileEditor::CreateEditor(const char* pszFilePath,
																						const char* pszFileName)
{
	void* pvIndices;
	uint32 uiIndexLength;
	INDEX_ENTRY* ptr;
	INDEX_ENTRY* pentry;
	IndexEntry entry;
	Pal::Tools::CGameResourceFileEditor* editor = new Pal::Tools::CGameResourceFileEditor();

	// 无法创建类
	if (editor == NULL)
		return NULL;

	// 没有找到资源
	if (!editor->SetResourceFilePathAndName(pszFilePath, pszFileName, true))
	{
		delete editor;
		return NULL;
	}

	// 不是可编辑的内容
	if (editor->m_Header.DataOffset != 0)
	{
		delete editor;
		return NULL;
	}

	// 无法读入索引
	if (!editor->LoadIndices(pvIndices, uiIndexLength))
	{
		delete editor;
		return NULL;
	}

	// 设置哈希表大小
	editor->m_IndexVector.resize(0x1000);
	entry.pvData = NULL;
	ptr = (INDEX_ENTRY*)pvIndices;
	for(uint32 i = 0; i < editor->m_Header.EntryCount; i++)
	{
		uint32 code;
		std::map<uint32, std::list<IndexEntry> >::iterator iter;

		// 得到最大的存储文件编号
		if (editor->m_uiMaxFileIndex < ptr->FileIndex)
			editor->m_uiMaxFileIndex = ptr->FileIndex;

		// 分配空间
		if ((pentry = (INDEX_ENTRY*)malloc(sizeof(INDEX_ENTRY) + ptr->PathLength * sizeof(wchar_t))) == NULL)
		{
			free(pvIndices);
			delete editor;
			return NULL;
		}
		memcpy(pentry, ptr, sizeof(INDEX_ENTRY) + ptr->PathLength * sizeof(wchar_t));
		entry.pEntry = pentry;

		// 存入哈希表
		code = CalculateHashCode(ptr->EntryPath, ptr->PathLength) & 0xFFFL;
		editor->m_IndexVector[code].push_back(entry);

		// 存入存储文件号
		if ((iter = editor->m_StoreMap.find(ptr->FileIndex)) == editor->m_StoreMap.end())
		{
			editor->m_StoreMap.insert(std::pair<uint32, std::list<IndexEntry> >(ptr->FileIndex, std::list<IndexEntry>()));
			iter = editor->m_StoreMap.find(ptr->FileIndex);
		}
		iter->second.push_back(entry);
		ptr = (INDEX_ENTRY*)((uint8*)(ptr + 1) + ptr->PathLength * sizeof(wchar_t));
	}
	editor->m_iCurrentEntry = editor->m_EmptyList.end();

	free(pvIndices);

	return editor;
}

bool Pal::Tools::CGameResourceFileEditor::CreateEntry(const wchar_t* pwszEntryName)
{
	INDEX_ENTRY* pentry;
	IndexEntry entry;
	uint32 code, length;

	// 检查输入参数
	if (pwszEntryName == NULL || (length = (uint32)wcslen(pwszEntryName)) == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}
	code = CalculateHashCode(pwszEntryName) & 0xFFFL;

	// 检查是否存在同名项
	if (!m_IndexVector[code].empty())
	{
		for(std::list<IndexEntry>::iterator iter =
			m_IndexVector[code].begin(); iter != m_IndexVector[code].end(); iter++)
			if (length = iter->pEntry->PathLength &&
				wcsncmp(iter->pEntry->EntryPath, pwszEntryName, iter->pEntry->PathLength) == 0)
			{
				m_iErrorCode = GRF_ERROR_CODE_ENTRYEXISTED;
				return false;
			}
	}

	// 分配空间
	if ((pentry = (INDEX_ENTRY*)malloc(sizeof(INDEX_ENTRY) + length * sizeof(wchar_t))) == NULL)
	{
		m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
		return false;
	}
	memset(pentry, 0, sizeof(INDEX_ENTRY));

	// 设置项值
	wcsncpy(pentry->EntryPath, pwszEntryName, length);
	pentry->PathLength = length;
	m_Header.EntryCount++;

	// 插入表中
	entry.pvData = NULL;
	entry.pEntry = pentry;
	m_IndexVector[code].push_back(entry);

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileEditor::RemoveEntry(const wchar_t* pwszEntryName)
{
	uint32 code, length;

	// 检查输入参数
	if (pwszEntryName == NULL || (length = (uint32)wcslen(pwszEntryName)) == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}
	code = CalculateHashCode(pwszEntryName) & 0xFFFL;

	if (!m_IndexVector[code].empty())
	{
		for(std::list<IndexEntry>::iterator iter =
			m_IndexVector[code].begin(); iter != m_IndexVector[code].end(); iter++)
			if (length = iter->pEntry->PathLength &&
				wcsncmp(iter->pEntry->EntryPath, pwszEntryName, iter->pEntry->PathLength) == 0)
			{
				std::map<uint32, std::list<IndexEntry> >::iterator slist;

				// 检查被删除项是否当前项
				if (m_iCurrentEntry->pEntry == iter->pEntry)
					m_iCurrentEntry = m_EmptyList.end();

				// 从存储文件引用表中删除该项
				if ((slist = m_StoreMap.find(iter->pEntry->FileIndex)) != m_StoreMap.end())
				{
					for(std::list<IndexEntry>::iterator iter1 = slist->second.begin();
						iter1 != slist->second.end(); iter1++)
						if (iter1->pEntry == iter->pEntry)
						{
							slist->second.erase(iter1);
							break;
						}

					// 检查相应的存储文件是否应当被删除
					if (slist->second.empty())
						m_RemoveSet.insert(iter->pEntry->FileIndex);
				}

				// 释放该项所占空间
				free(iter->pEntry);

				// 将该项从哈希表中删除
				m_IndexVector[code].erase(iter);

				m_Header.EntryCount--;

				m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
				return true;
			}
	}

	m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTEXIST;
	return false;
}

bool Pal::Tools::CGameResourceFileEditor::Seek(const wchar_t* pwszEntryName)
{
	uint32 code, length;

	// 检查输入参数
	if (pwszEntryName == NULL || (length = (uint32)wcslen(pwszEntryName)) == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}
	code = CalculateHashCode(pwszEntryName) & 0xFFFL;

	if (!m_IndexVector[code].empty())
	{
		for(std::list<IndexEntry>::iterator iter =
			m_IndexVector[code].begin(); iter != m_IndexVector[code].end(); iter++)
			if (length == iter->pEntry->PathLength &&
				wcsncmp(iter->pEntry->EntryPath, pwszEntryName, iter->pEntry->PathLength) == 0)
			{
				m_iCurrentEntry = iter;
				m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
				return true;
			}
	}

	m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTEXIST;
	return false;
}

bool Pal::Tools::CGameResourceFileEditor::GetResourceType(uint8& type)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		type = m_iCurrentEntry->pEntry->ResourceType;

		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::GetCompressAlgorithm(uint8& algo)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		algo = m_iCurrentEntry->pEntry->CompressAlgorithm;

		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::GetNameLength(uint32& uiLength)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		uiLength = m_iCurrentEntry->pEntry->PathLength;

		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::GetEntryName(wchar_t* pwszEntryName, uint32 uiLength)
{
	// 参数非法
	if (pwszEntryName == NULL || uiLength == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}

	// 还没有指定当前项
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}

	// 复制名字，确保以'\0'结尾
	if (uiLength > m_iCurrentEntry->pEntry->PathLength)
	{
		wcsncpy(pwszEntryName, m_iCurrentEntry->pEntry->EntryPath, m_iCurrentEntry->pEntry->PathLength);
		pwszEntryName[m_iCurrentEntry->pEntry->PathLength] = L'\0';
	}
	else
	{
		wcsncpy(pwszEntryName, m_iCurrentEntry->pEntry->EntryPath, uiLength - 1);
		pwszEntryName[uiLength - 1] = L'\0';
	}

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileEditor::GetDataLength(uint32& uiLength)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		uiLength = m_iCurrentEntry->pEntry->Length;

		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::GetData(uint32 uiOffset, void* pvData, uint32 uiLength)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		if (m_iCurrentEntry->pvData != NULL)
		{
			uint32 len;
			if (pvData == NULL || uiOffset >= m_iCurrentEntry->pEntry->Length)
			{
				m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
				return false;
			}
			if (uiOffset + uiLength > m_iCurrentEntry->pEntry->Length)
				len = m_iCurrentEntry->pEntry->Length - uiOffset;
			else
				len = uiLength;
			memcpy(pvData, (uint8*)m_iCurrentEntry->pvData + uiOffset, len);

			m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
			return true;
		}
		else
			return LoadEntryData(*(m_iCurrentEntry->pEntry), uiOffset, pvData, uiLength);
	}
}

bool Pal::Tools::CGameResourceFileEditor::SetResourceType(uint8 type)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		m_iCurrentEntry->pEntry->ResourceType = type;

		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::SetCompressAlgorithm(uint8 algo)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		m_iCurrentEntry->pEntry->CompressAlgorithm = algo;

		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::SetEntryName(const wchar_t* pwszEntryName)
{
	INDEX_ENTRY* pentry;
	uint32 code, length;

	// 参数非法
	if (pwszEntryName == NULL || (length = (uint32)wcslen(pwszEntryName)) == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}
	code = CalculateHashCode(pwszEntryName) & 0xFFFL;

	// 还没有指定当前项
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}

	// 重新分配空间
	if ((pentry = (INDEX_ENTRY*)realloc(m_iCurrentEntry->pEntry, sizeof(INDEX_ENTRY) + length * sizeof(wchar_t))) == NULL)
	{
		m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
		return false;
	}

	// 设置名字
	pentry->PathLength = length;
	wcsncpy(pentry->EntryPath, pwszEntryName, length);
	m_iCurrentEntry->pEntry = pentry;

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

bool Pal::Tools::CGameResourceFileEditor::SetDataLength(uint32 uiLength)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else
	{
		std::map<uint32, std::list<IndexEntry> >::iterator slist;
		void* pvNewData;

		// 分配空间
		if ((pvNewData = realloc(m_iCurrentEntry->pvData, uiLength)) == NULL && uiLength != 0)
		{
			m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
			return false;
		}

		// 更新所有引用同一存储文件的项
		if ((slist = m_StoreMap.find(m_iCurrentEntry->pEntry->FileIndex)) != m_StoreMap.end())
		{
			std::list<IndexEntry>::iterator iter;
			for(iter = slist->second.begin(); iter != slist->second.end(); iter++)
			{
				iter->pvData = pvNewData;
				iter->pEntry->Length = uiLength;
			}
		}
		
		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::SetData(uint32 uiOffset, const void* pvData, uint32 uiLength)
{
	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else if (pvData == NULL || uiLength == 0 || uiOffset > m_iCurrentEntry->pEntry->Length)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}
	else
	{
		// 检查空间是否够用，不够先行分配
		if (uiOffset + uiLength > m_iCurrentEntry->pEntry->Length &&
			!SetDataLength(uiOffset + uiLength))
		{
			m_iErrorCode = GRF_ERROR_CODE_INTERNALERROR;
			return false;
		}

		// 复制数据
		memcpy((uint8*)m_iCurrentEntry->pvData + uiOffset, pvData, uiLength);

		m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
		return true;
	}
}

bool Pal::Tools::CGameResourceFileEditor::SetData(const wchar_t* pwszEntryName)
{
	uint32 length;

	if (m_iCurrentEntry == m_EmptyList.end())
	{
		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTSPECIFY;
		return false;
	}
	else if (pwszEntryName == NULL || (length = (uint32)wcslen(pwszEntryName)) == 0)
	{
		m_iErrorCode = GRF_ERROR_CODE_INVAILDPARAMETER;
		return false;
	}
	else
	{
		uint32 code = CalculateHashCode(pwszEntryName) & 0xFFFL;

		// 查找对应项
		if (!m_IndexVector[code].empty())
		{
			for(std::list<IndexEntry>::iterator iter =
				m_IndexVector[code].begin(); iter != m_IndexVector[code].end(); iter++)
				if (length == iter->pEntry->PathLength &&
					wcsncmp(iter->pEntry->EntryPath, pwszEntryName, iter->pEntry->PathLength) == 0)
				{
					// 找到对应项，检查引用存储文件是否相同
					if (m_iCurrentEntry->pEntry->FileIndex != iter->pEntry->FileIndex)
					{
						std::map<uint32, std::list<IndexEntry> >::iterator slist;

						// 更新引用的存储文件
						if ((slist = m_StoreMap.find(m_iCurrentEntry->pEntry->FileIndex)) != m_StoreMap.end())
						{
							for(std::list<IndexEntry>::iterator iter1 = slist->second.begin();
								iter1 != slist->second.end(); iter1++)
								if (iter1->pEntry == m_iCurrentEntry->pEntry)
								{
									// 找到存储项，从列表中删除
									slist->second.erase(iter1);
									break;
								}

							// 检查相应的存储文件是否应当被删除
							if (slist->second.empty())
								m_RemoveSet.insert(m_iCurrentEntry->pEntry->FileIndex);
						}

						// 检查是否已经写入了数据
						if (m_iCurrentEntry->pvData != NULL)
							free(m_iCurrentEntry->pvData);
						m_iCurrentEntry->pvData = iter->pvData;
						m_iCurrentEntry->pEntry->Length = iter->pEntry->Length;
						m_iCurrentEntry->pEntry->FileIndex = iter->pEntry->FileIndex;

						// 是否需要将存储文件号记录
						if ((slist = m_StoreMap.find(iter->pEntry->FileIndex)) != m_StoreMap.end())
							slist->second.push_back(*m_iCurrentEntry);
					}

					m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
					return true;
				}
		}

		m_iErrorCode = GRF_ERROR_CODE_ENTRYNOTEXIST;
		return false;
	}
}

bool Pal::Tools::CGameResourceFileEditor::SaveChanges()
{
	std::vector<std::list<IndexEntry> >::size_type i;
	std::list<IndexEntry>::iterator iter, iter1;
	int iFileHandle;
	size_t len = strlen(m_pszDataFileName);

	// 首先写入数据
	for(i = 0; i < m_IndexVector.size(); i++)
		for(iter = m_IndexVector[i].begin(); iter != m_IndexVector[i].end(); iter++)
		{
			std::map<uint32, std::list<IndexEntry> >::iterator slist;
			void* pvData = iter->pvData;

			// 如果有数据需要写入
			if (pvData)
			{
				// 生成文件名，对于新创建的项则向后编号
				if (iter->pEntry->FileIndex == 0)
					IntegerToHexString(iter->pEntry->FileIndex, m_pszDataFileName + len - 8);
				else
					IntegerToHexString(m_uiMaxFileIndex + 1, m_pszDataFileName + len - 8);

				// 尝试打开数据文件
				if ((iFileHandle = open(m_pszDataFileName, O_BINARY | O_WRONLY | O_CREAT | O_TRUNC)) != -1)
				{
					m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
					return false;
				}

				// 写入数据
				if (write(iFileHandle, pvData, iter->pEntry->Length) < (int)iter->pEntry->Length)
				{
					close(iFileHandle);
					m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
					return false;
				}

				// 关闭文件
				close(iFileHandle);
			}

			// 设置文件存储编号
			if (iter->pEntry->FileIndex == 0)
				iter->pEntry->FileIndex = ++m_uiMaxFileIndex;

			// 将所有引用同一存储文件的数据项置空
			if ((slist = m_StoreMap.find(iter->pEntry->FileIndex)) != m_StoreMap.end())
				for(iter1 = slist->second.begin(); iter1 != slist->second.end(); iter1++)
					iter1->pvData = NULL;

			// 释放数据空间
			free(pvData);
		}

	// 其次写入索引，首先尝试打开索引文件
	if ((iFileHandle = open(m_pszIndexFileName, O_BINARY | O_WRONLY | O_CREAT | O_TRUNC)) == -1)
	{
		m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
		return false;
	}

	// 从文件头起，逐项写入索引内容
	if (write(iFileHandle, &m_Header, sizeof(GRF_HEADER)) < sizeof(GRF_HEADER))
	{
		close(iFileHandle);
		m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
		return false;
	}
	for(i = 0; i < m_IndexVector.size(); i++)
		for(iter = m_IndexVector[i].begin(); iter != m_IndexVector[i].end(); iter++)
			if (write(iFileHandle, iter->pEntry, sizeof(INDEX_ENTRY) + iter->pEntry->Length * sizeof(wchar_t)) <
				(int)(sizeof(INDEX_ENTRY) + iter->pEntry->Length * sizeof(wchar_t)))
			{
				close(iFileHandle);
				m_iErrorCode = GRF_ERROR_CODE_FILEERROR;
				return false;
			}

	// 关闭索引文件
	close(iFileHandle);

	// 最后删除那些无用的存储文件
	for(std::set<uint32>::iterator iter2 = m_RemoveSet.begin(); iter2 != m_RemoveSet.end(); iter2++)
	{
		int ret;

		IntegerToHexString(*iter2, m_pszDataFileName + len - 8);
		ret = unlink(m_pszDataFileName);
	}

	m_iErrorCode = GRF_ERROR_CODE_SUCCESSFUL;
	return true;
}

uint32 Pal::Tools::CGameResourceFileEditor::CalculateHashCode(const wchar_t* pwszString)
{
	size_t len;
	wchar_t code;

	if (pwszString == NULL || (len = wcslen(pwszString)) == 0)
		return 0;

	code = pwszString[0];
	for(size_t i = 1; i < len; i++)
		code ^= pwszString[i];

	return (uint32)code;
}

uint32 Pal::Tools::CGameResourceFileEditor::CalculateHashCode(const wchar_t* pwszString, uint32 uiLength)
{
	wchar_t code;

	if (pwszString == NULL || uiLength == 0)
		return 0;

	code = pwszString[0];
	for(uint32 i = 1; i < uiLength; i++)
		code ^= pwszString[i];

	return (uint32)code;
}
