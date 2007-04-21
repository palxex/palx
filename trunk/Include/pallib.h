/*
 * PAL library common include file
 * 
 * Author: Lou Yihua <louyihua@21cn.com>
 *
 * Copyright 2006 - 2007 Lou Yihua
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
 *《仙剑奇侠传》库公共头文件
 *
 * 作者： 楼奕华 <louyihua@21cn.com>
 *
 * 版权所有 2006 - 2007 楼奕华
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

#pragma once

#ifndef	PAL_LIBRARY_H
#	define	PAL_LIBRARY_H
#endif

#include <io.h>
#include <fcntl.h>
#include <errno.h>
#if	defined(WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <wchar.h>
#include <stddef.h>
#include <locale.h>

#include <map>
#include <set>
#include <list>
#include <vector>

#include "config.h"

namespace Pal
{
	namespace Tools
	{
		#include "grf.h"

		class CGameResourceFile
		{
		public:
			virtual ~CGameResourceFile();
			int GetErrorCode()	{ return m_iErrorCode; }

			virtual bool Seek(const wchar_t* pwszEntryName) = 0;

		protected:
			CGameResourceFile();

			void IntegerToHexString(uint32 ui, char* pchBuffer);
			bool SetResourceFilePathAndName(const char* pszFilePath, const char* pszFileName, bool bCreateOnNotExist);
			bool LoadIndices(void*& pvIndices, uint32& uiLength);
			bool LoadEntryData(const INDEX_ENTRY& entry, uint32 uiOffset, void* pvData, uint32 uiLength);

			int			m_iErrorCode;
			char*		m_pszDataFileName;
			char*		m_pszIndexFileName;
			GRF_HEADER	m_Header;
		};

		class IGameResourceFileReadable
		{
		public:
			virtual bool GetResourceType(uint8& type) = 0;
			virtual bool GetCompressAlgorithm(uint8& algo) = 0;
			virtual bool GetNameLength(uint32& uiLength) = 0;
			virtual bool GetEntryName(wchar_t* pwszEntryName, uint32 uiLength) = 0;
			virtual bool GetDataLength(uint32& uiLength) = 0;
			virtual bool GetData(uint32 uiOffset, void* pvData, uint32 uiLength) = 0;
		};

		class IGameResourceFileWritable
		{
		public:
			virtual bool SetResourceType(uint8 type) = 0;
			virtual bool SetCompressAlgorithm(uint8 algo) = 0;
			virtual bool SetEntryName(const wchar_t* pwszEntryName) = 0;
			virtual bool SetDataLength(uint32 uiLength) = 0;
			virtual bool SetData(uint32 uiOffset, const void* pvData, uint32 uiLength) = 0;
			virtual bool SetData(const wchar_t* pwszEntryName) = 0;
		};

		class CGameResourceFileReader: public CGameResourceFile, public IGameResourceFileReadable
		{
		public:
			~CGameResourceFileReader();
			static CGameResourceFileReader* CreateReader(const char* pszFilePath, const char* pszFileName);

			bool Seek(const wchar_t* pwszEntryName);
			bool GetResourceType(uint8& type);
			bool GetCompressAlgorithm(uint8& algo);
			bool GetNameLength(uint32& uiLength);
			bool GetEntryName(wchar_t* pwszEntryName, uint32 uiLength);
			bool GetDataLength(uint32& uiLength);
			bool GetData(uint32 uiOffset, void* pvData, uint32 uiLength);

		private:
			CGameResourceFileReader();

			void*	m_pvIndices;
			uint32	m_uiIndexLength;
			uint32	m_uiCurrentEntry;
			INDEX_ENTRY** m_pEntries;
		};

		class CGameResourceFileEditor: public CGameResourceFile, public IGameResourceFileReadable, public IGameResourceFileWritable
		{
			typedef struct
			{
				INDEX_ENTRY*	pEntry;
				void*			pvData;
			}	IndexEntry;
		public:
			~CGameResourceFileEditor();
			static CGameResourceFileEditor* CreateEditor(const char* pszFilePath, const char* pszFileName);

			bool SaveChanges();

			bool CreateEntry(const wchar_t* pwszEntryName);
			bool RemoveEntry(const wchar_t* pwszEntryName);

			bool Seek(const wchar_t* pwszEntryName);

			bool GetResourceType(uint8& type);
			bool GetCompressAlgorithm(uint8& algo);
			bool GetNameLength(uint32& uiLength);
			bool GetEntryName(wchar_t* pwszEntryName, uint32 uiLength);
			bool GetDataLength(uint32& uiLength);
			bool GetData(uint32 uiOffset, void* pvData, uint32 uiLength);

			bool SetResourceType(uint8 type);
			bool SetCompressAlgorithm(uint8 algo);
			bool SetEntryName(const wchar_t* pwszEntryName);
			bool SetDataLength(uint32 uiLength);
			bool SetData(uint32 uiOffset, const void* pvData, uint32 uiLength);
			bool SetData(const wchar_t* pwszEntryName);

		private:
			CGameResourceFileEditor();

			static uint32 CalculateHashCode(const wchar_t* pwszString);
			static uint32 CalculateHashCode(const wchar_t* pwszString, uint32 uiLength);

			uint32	m_uiMaxFileIndex;

			std::list<IndexEntry>						m_EmptyList;
			std::list<IndexEntry>::iterator				m_iCurrentEntry;
			std::vector<std::list<IndexEntry> >			m_IndexVector;
			std::map<uint32, std::list<IndexEntry> >	m_StoreMap;
			std::set<uint32>							m_RemoveSet;
		};

		bool DecodeYJ1(const void* Source, void*& Destination, uint32& Length);
		bool EncodeYJ1(const void* Source, uint32 SourceLength, void*& Destination, uint32& Length);
		bool DecodeYJ2(const void* Source, void*& Destination, uint32& Length);
		bool EncodeYJ2(const void* Source, uint32 SourceLength, void*& Destination, uint32& Length, bool bCompatible = true);
		bool DecodeRNG(const void* Source, void* PrevFrame);
		bool EncodeRNG(const void* PrevFrame, const void* CurFrame, void*& Destination, uint32& Length);
		bool DecodeRLE(const void* Rle, void* Destination, sint32 Width, sint32 Height, sint32 x, sint32 y);
		bool EncodeRLE(const void* Source, const void *Base, sint32 Width, sint32 Height, void*& Destination, uint32& Length);
		bool EncodeRLE(const void* Source, uint8 TransparentColor, sint32 Width, sint32 Height, void*& Destination, uint32& Length);

		bool DecodeYJ1StreamInitialize(void*& pvState, uint32 uiGrowBy = 0x10000);
		bool DecodeYJ1StreamInput(void* pvState, const void* Source, uint32 SourceLength);
		bool DecodeYJ1StreamOutput(void* pvState, void* Destination, uint32& Length);
		bool DecodeYJ1StreamFinished(void* pvState, uint32& AvailableLength);
		bool DecodeYJ1StreamFinalize(void* pvState);

		bool DecodeYJ2StreamInitialize(void*& pvState, uint32 uiGrowBy = 0x10000);
		bool DecodeYJ2StreamInput(void* pvState, const void* Source, uint32 SourceLength);
		bool DecodeYJ2StreamOutput(void* pvState, void* Destination, uint32& Length);
		bool DecodeYJ2StreamFinished(void* pvState, uint32& AvailableLength);
		bool DecodeYJ2StreamFinalize(void* pvState);

		bool EncodeYJ2StreamInitialize(void*& pvState, uint32 uiSourceLength, uint32 uiGrowBy = 0x10000, bool bCompatible = true);
		bool EncodeYJ2StreamInput(void* pvState, const void* Source, uint32 SourceLength, bool bFinished = false);
		bool EncodeYJ2StreamOutput(void* pvState, void* Destination, uint32& Length, uint32& Bits);
		bool EncodeYJ2StreamFinished(void* pvState);
		bool EncodeYJ2StreamFinalize(void* pvState);
	}
}
