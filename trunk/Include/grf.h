/*
 * PAL library GRF format include file
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
 *《仙剑奇侠传》库GRF格式头文件
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

/*	GRF(Game Resource File) 文件格式
　　GRF 文件由两部分组成：索引（Index）及数据（Data）。
　　索引是有格式的部分，存放各个子文件的信息。数据是无格式的部分，以数据流的形式存放
数据。在索引中有对数据的引用。
　　索引的文件名任意，扩展名为“.GRF”；数据有两种存储形式：集成存储，即数据存放于索
引文件中，位于索引之后；单独存储，每个索引项均使用独立文件，文件名为“NNNNNNNN”（其
中“NNNNNNNN”为十六进制数）。
　　单独存储在对数据进行修改时效率较高；而集成存储则在读取时显得较简单。因此可以编辑
修改的GRF文件一律为单独存储。
 */

#pragma once

#ifndef	PAL_LIBRARY_H
#	error	Please include pallib.h instead of this file!
#endif

/* 压缩算法常量定义 */
#define	GRF_COMPRESS_ALGORITHM_NONE		0	/* 无压缩 */
#define	GRF_COMPRESS_ALGORITHM_YJ1		1	/* YJ_1 */
#define	GRF_COMPRESS_ALGORITHM_YJ2		2	/* YJ_2 (WIN版) */

/* 资源类型常量定义 */
#define	GRF_RESOURCE_TYPE_NONE			0	/* 无类型 */

/* 错误代码常量定义 */
#define	GRF_ERROR_CODE_SUCCESSFUL		0							/* 无错误（成功） */
#define	GRF_ERROR_CODE_BASE				0x80000000L					/* 错误码基础值 */
#define	GRF_ERROR_CODE_INTERNALERROR	(GRF_ERROR_CODE_BASE + 0)	/* 内部错误（内存分配错误） */
#define	GRF_ERROR_CODE_INVAILDPARAMETER	(GRF_ERROR_CODE_BASE + 1)	/* 非法参数 */
#define	GRF_ERROR_CODE_PATHNOTEXIST		(GRF_ERROR_CODE_BASE + 2)	/* 指定的路径不存在或无法创建 */
#define	GRF_ERROR_CODE_FILEERROR		(GRF_ERROR_CODE_BASE + 3)	/* 文件操作错误 */
#define	GRF_ERROR_CODE_INVAILDFILE		(GRF_ERROR_CODE_BASE + 4)	/* 非法文件格式 */
#define	GRF_ERROR_CODE_ENTRYEXISTED		(GRF_ERROR_CODE_BASE + 5)	/* 指定的项已经存在 */
#define	GRF_ERROR_CODE_ENTRYNOTEXIST	(GRF_ERROR_CODE_BASE + 6)	/* 指定的项不存在 */
#define	GRF_ERROR_CODE_ENTRYNOTSPECIFY	(GRF_ERROR_CODE_BASE + 7)	/* 还未选定当前项 */

#pragma pack(1)
typedef struct _GRF_HEADER
{
	char	Signature[4];		/* 文件标志，应为'GRF\0' */
	uint32	FileLength;			/* 文件总长度 */
	uint32	EntryCount;			/* 包含索引项数 */
	uint32	DataOffset;			/* 数据起始偏移，为 0 表示数据另行存放 */
}	GRF_HEADER;

typedef struct _INDEX_ENTRY
{
	union
	{
	uint32	FileIndex;			/* （单独存储）存储文件号 */
	uint32	Offset;				/* （集成存储）存储偏移量 */
	};
	uint32	Length;				/* 文件长度 */
	uint8	ResourceType;		/* 资源类型 */
	uint8	CompressAlgorithm;	/* 压缩算法 */
	uint16	PathLength;			/* 路径全名长度 */
	wchar_t	EntryPath[0];		/* 路径全名 */
}	INDEX_ENTRY;
#pragma pack()
