/*
 * PAL RLE format library
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
 *《仙剑奇侠传》RLE格式处理库
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

#include <stdlib.h>
#include <memory.h>

#include "pallib.h"
using namespace Pal::Tools;

bool Pal::Tools::DecodeRLE(const void *Rle, void *Destination, sint32 Width, sint32 Height, sint32 x, sint32 y)
{
	sint32 sx, sy, dx, dy, i, j, width, height;
	uint8 count;
	uint8* dst;
	uint8* dest;
	uint16	rle_width = *(uint16*)Rle,
			rle_height = *((uint16*)Rle + 1);
	uint8* ptr = (uint8*)Rle + 4;

	if (Rle == NULL || Destination == NULL)
		return false;
	if (x + rle_width < 0 || x >= Width || y + rle_height < 0 || y >= Height)
		return true;

	if (x < 0)
		sx = 0, dx = -x;
	else
		sx = x, dx = 0;
	if (y < 0)
		sy = 0, dy = -y;
	else
		sy = y, dy = 0;
	width = (Width - sx < rle_width - dx) ? (Width - sx) : (rle_width - dx);
	height = (Height - sy < rle_height - dy) ? (Height - sy) : (rle_height - dy);

	for(i = 0; i < dy; i++)
		for(j =0; j< rle_width;)
		{
			count=*ptr++;
			if(count < 0x80)
				ptr += count;
			j += count & 0x7f;
		}
	for(dst = dest = (uint8 *)Destination + (sy * Width) + sx, i = 0; i < height; i++, dest = dst + i * Width)
		for(j = 0; j < rle_width;)
		{
			count = *ptr++;
			if (count < 0x80)
			{
				if(j < dx &&  j + count > dx){
					memcpy(dest, ptr + dx - j, count - dx + j);
					dest += (count - dx +j);
				}else if(j >= dx && j < dx + width){
					int count2 = (j + count < dx + width ) ? count : dx + width - j;
					memcpy(dest, ptr, count2);
					dest += count2;
				}else
					dest += count;
				ptr += count;
			}else
				if(j >= dx)
					dest += count & 0x7f;
			j += count & 0x7f;
		}

	return true;
}

/*
bool Pal::Tools::DecodeRLE(const void *Rle, void *Destination, sint32 Width, sint32 Height, sint32 x, sint32 y)
{
	sint32 sx, sy, dx, dy, temp;
	uint16 rle_width, rle_height;
	uint8  count, cnt;
	uint8* dest;
	uint8* ptr;

	//检查输入参数
	if (Rle == NULL || Destination == NULL)
		return false;
	//取 RLE 图像的宽度和高度
	rle_width = *(uint16*)Rle;
	rle_height = *((uint16*)Rle + 1);
	ptr = (uint8*)Rle + 4;
	//检查 RLE 图像能否显示到指定的图像中
	if (Width <= 0 || Height <= 0 || x + rle_width < 0 || x >= Width || y + rle_height < 0 || y >= Height)
		return true;

	//跳过由于 y < 0 导致的不能显示的部分
	for(sy = 0, dy = y; dy < 0; dy++, sy++)
		for(sx = 0; sx < rle_width;)
		{
			count = *ptr++;
			if (count < 0x80)
			{
				ptr += count;
				sx += count;
			}
			else
				sx += count & 0x7f;
		}
	//设置目标区域指针
	dest = (uint8*)Destination + dy * Width;
	//填充目标区域
	for(; dy < Height && sy < rle_height; dy++, sy++)
	{
		//跳过由于 x < 0 导致的不能显示的部分
		for(count = 0, dx = x; dx < 0;)
		{
			count = *ptr++;
			if (count < 0x80)
			{
				ptr += count;
				if ((dx += count) >= 0)
				{
					//不透明像素需要部分显示，将ptr和count置成正确的值
					ptr -= dx;
					count = dx;
					dx = 0;
				}
			}
			else
				dx += count & 0x7f;
		}
		//检查是否已经越过目标图像边界
		if ((temp = Width - dx) <= 0)
			dest += Width;
		else
		{
			//填充目标图像
			if (count < 0x80)
			{
				//不透明部分
				if (count > temp)
					cnt = temp;
				else
					cnt = count;
				//填充
				memcpy(dest, ptr, cnt);
				dest += cnt;
				dx += cnt;
				//调整源指针
				ptr += count;
			}
			else
				dest += dx;
			//填充该行剩下的部分
			for(sx = dx - x; sx < rle_width && dx < Width;)
			{
				//取次数
				count = *ptr++;
				if (count < 0x80)
				{
					//不透明部分，处理同上
					if (count > Width - dx)
						cnt = Width - dx;
					else
						cnt = count;
					//填充
					memcpy(dest, ptr, cnt);
					dest += cnt;
					dx += cnt;
					//调整源指针和源 x 值
					ptr += count;
					sx += count;
				}
				else
				{
					//透明部分
					count &= 0x7f;
					//调整源指针和源、目标 x 值
					sx += count;
					dx += count;
				}
			}
			//填充未到行尾，调整目标指针
			if (dx < Width)
				dest += Width - dx;
		}
		//调整源指针，使指针指向下一行
		while(sx < rle_width)
		{
			count = *ptr++;
			if (count < 0x80)
			{
				ptr += count;
				sx += count;
			}
			else
				sx += count & 0x7f;
		}
	}
	return true;
}
*/

bool Pal::Tools::EncodeRLE(const void *Source, const void *Base, sint32 Width, sint32 Height, void*& Destination, uint32& Length)
{
	sint32 i, j, length = 0;
	bool flag;
	uint8 count;
	uint8* src = (uint8*)Source;
	uint8* base = (uint8*)Base;
	uint8* temp;
	uint8* ptr;

	if (Source == NULL || Base == NULL)
		return false;
	if ((ptr = temp = new uint8 [Width * Height * 2]) == NULL)
		return false;

	for(i = 0; i < Height; i++)
	{
		flag = (*src == *base); count = 0;
		for(j = 0; j < Width; j++)
		{
			if (flag)
				if (*src++ == *base++)
				{
					if (count < 0x7f)
						count++;
					else
					{
						*ptr++ = 0xff;
						count = 1;
					}
				}
				else
				{
					*ptr++ = count | 0x80;
					flag = false;
					count = 1;
				}
			else
				if (*src++ != *base++)
				{
					if (count < 0x7f)
						count++;
					else
					{
						*ptr++ = 0x7f;
						memcpy(ptr, src - 0x7f, 0x7f);
						ptr += 0x7f;
						count = 1;
					}
				}
				else
				{
					*ptr++ = count;
					memcpy(ptr, src - count, count);
					ptr += count;
					flag = true;
					count = 1;
				}
		}
	}

	if ((Destination = malloc(length + 4)) == NULL)
	{
		delete [] temp;
		return false;
	}
	*((uint16*)Destination) = (uint16)Width;
	*((uint16*)Destination + 1) = (uint16)Height;
	memcpy((uint8*)Destination + 4, temp, length);
	Length = length + 4;
	delete [] temp;
	return true;
}

bool Pal::Tools::EncodeRLE(const void *Source, const uint8 TransparentColor, sint32 Width, sint32 Height, void*& Destination, uint32& Length)
{
	sint32 i, j, length = 0;
	bool flag;
	uint8 count;
	uint8* src = (uint8*)Source;
	uint8* temp;
	uint8* ptr;

	if (Source == NULL)
		return false;
	if ((ptr = temp = new uint8 [Width * Height * 2]) == NULL)
		return false;

	for(i = 0; i < Height; i++)
	{
		flag = (*src == TransparentColor); count = 0;
		for(j = 0; j < Width; j++)
		{
			if (flag)
				if (*src++ == TransparentColor)
				{
					if (count < 0x7f)
						count++;
					else
					{
						*ptr++ = 0xff;
						count = 1;
					}
				}
				else
				{
					*ptr++ = count | 0x80;
					flag = false;
					count = 1;
				}
			else
				if (*src++ != TransparentColor)
				{
					if (count < 0x7f)
						count++;
					else
					{
						*ptr++ = 0x7f;
						memcpy(ptr, src - 0x7f, 0x7f);
						ptr += 0x7f;
						count = 1;
					}
				}
				else
				{
					*ptr++ = count;
					memcpy(ptr, src - count, count);
					ptr += count;
					flag = true;
					count = 1;
				}
		}
	}

	if ((Destination = malloc(length + 4)) == NULL)
	{
		delete [] temp;
		return false;
	}
	*((uint16*)Destination) = (uint16)Width;
	*((uint16*)Destination + 1) = (uint16)Height;
	memcpy((uint8*)Destination + 4, temp, length);
	Length = length + 4;
	delete [] temp;
	return true;
}
