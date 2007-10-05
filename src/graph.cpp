/***************************************************************************
 *   PALx: A platform independent port of classic RPG PAL                  *
 *   Copyleft (C) 2006 by Pal Lockheart                                    *
 *   palxex@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, If not, see                          *
 *   <http://www.gnu.org/licenses/>.                                       *
 ***************************************************************************/
#include "allegdef.h"
#include "internal.h"
#include "game.h"

bitmap::bitmap(const uint8_t *src,int width,int height):
	bmp(create_bitmap(width,height))
{
	if(src)
		memcpy(bmp->dat,src,width*height);
}
bitmap::bitmap(BITMAP *b):bmp(create_bitmap(b->w,b->h))
{
    blit(b,bmp,0,0,0,0,b->w,b->h);
}
bitmap::~bitmap()
{
	destroy_bitmap(bmp);
}
bool bitmap::blit_to(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y)
{
	blit(bmp,dest,source_x,source_y,dest_x,dest_y,std::max(bmp->w,dest->w),std::max(bmp->h,dest->h));
	return true;
}
sprite::sprite(uint8_t *src):x(0),y(0),l(0),width(0),height(0),buf(src)
{
	width=((uint16_t*)src)[0];
	height=((uint16_t*)src)[1];
}
sprite::~sprite()
{}
sprite *sprite::clone()
{
	return new sprite(buf);
}
void sprite::setXYL(int x,int y,int l)
{
	this->x=x-width/2;
	this->y=y;
	this->l=l;
}
void sprite::blit_middle(BITMAP *dest,int x,int y)
{
	this->x=x-((uint16_t*)buf)[0]/2;
	this->y=y+((uint16_t*)buf)[1]/2;
	this->l=0;
	blit_to(dest);
}
bool do_nothing(int srcVal, uint8* pOutVal, void* pUserData){return false;}
bool sprite::blit_to(BITMAP *dest)
{
	if(!dest->dat)
	{
		BITMAP *buf2=create_bitmap(dest->w,dest->h);
		blit(dest,buf2,0,0,0,0,dest->w,dest->h);
		Pal::Tools::DecodeRle(buf,buf2->dat,dest->w,dest->h,x,y-l-height,do_nothing,NULL);
		blit(buf2,dest,0,0,0,0,dest->w,dest->h);
		destroy_bitmap(buf2);
	}else
		Pal::Tools::DecodeRle(buf,dest->dat,dest->w,dest->h,x,y-l-height,do_nothing,NULL);
	return true;
}
bool sprite::blit_to(BITMAP *dest,int x,int y,bool shadow,int sx,int sy)
{
	if(shadow)
	{
		uint8_t *rle = buf + 4, *dst;
		int l = dest->w;

		BITMAP *bmp;
		if(dest!=screen)
			dst=(uint8_t *)dest->dat;
		else{
			bmp=create_bitmap(SCREEN_W,SCREEN_H);
			blit(screen,bmp,0,0,0,0,SCREEN_W,SCREEN_H);
			dst=(uint8_t *)bmp->dat;
		}

		for(int i=(y+sy)*l+x+sx,prei=i;i<(y+sy+height)*l+x+sx && i<=dest->w*dest->h;i=prei+l,prei=i)
			for(int j=0;j<width;)
			{
				uint8_t flag=*rle++;
				if(flag>=0x80)
					i += (flag-0x80);
				else{
					for(int t=j;t<j+flag;t++)
						dst[i+t]-=(dst[i+t]%0x10/2);
					rle+=flag;
				}
				j+=(flag%0x80);
			}

		if(dest==screen){
			blit(bmp,screen,0,0,0,0,SCREEN_W,SCREEN_H);
			destroy_bitmap(bmp);
		}
	}

	this->x=x;
	this->y=y+height;
	this->l=0;
	return blit_to(dest);
}
bool operator<(const sprite &lhs, const sprite &rhs)
{
	return lhs.y<rhs.y;
}

sprite_prim::sprite_prim():id(-1)
{}
sprite_prim::sprite_prim(int _id):id(_id)
{}
sprite_prim::sprite_prim(cached_res &archive,int _id):id(_id)
{
	getsource(archive.decode(id));
}
int sprite_prim::determain_smkfs(uint8_t *src)
{
	uint16_t *usrc=(uint16_t*)src;
	return usrc[0]-(usrc[usrc[0]-1]==0||usrc[usrc[0]-1]>0x8000?1:0);
}
sprite_prim::sprite_prim(int _id,uint8_t *src):id(_id)
{
	getsource(src);
}
sprite_prim::sprite_prim(const sprite_prim &rhs):id(rhs.id),sprites(rhs.sprites)
{}
sprite_prim &sprite_prim::getsource(uint8_t *src)
{
	for(int i=0,subfiles=determain_smkfs(src);i<subfiles;i++)
		sprites.push_back(boost::shared_ptr<sprite>(new sprite(src+2*((uint16_t *)src)[i])));
	return *this;
}
sprite * sprite_prim::getsprite(int i)
{
	return sprites[i].get();
}
void sprite_prim::blit(int curr,BITMAP *bmp)
{
	sprites[curr]->blit_to(bmp);
}
bool operator==(const sprite_prim& lhs,const sprite_prim& rhs)
{
	return lhs.id==rhs.id;
}

ALFONT_FONT *ttfont::glb_font;
void ttfont::blit_to(BITMAP *dest,int x,int y,uint8_t color,bool shadow){
	if(shadow)
		alfont_textout(dest, glb_font, msg, x+1, y+1, 0);
	alfont_textout(dest, glb_font, msg, x, y, color);
}

palette::palette()
	:pal(-1),day(-1)
{}
void palette::read(uint32_t i)
{
	pal=i;
	bool fx;long len;
	myRGB *buf=(myRGB *)PAT.decode(i,0,fx,len);
	RGB   *p=(RGB*)pat;
	for(int t=0;t<(i==0 || i==5?512:256);t++)
		p[t].r=buf[t].r,
		p[t].g=buf[t].g,
		p[t].b=buf[t].b;
}
PALETTE &palette::get(int palette_offset)
{
	return pat[palette_offset!=0];
}
void palette::set(int t)
{
	day=t;
	set_palette(pat[day]);
}
void palette::switch_daytime()
{
	day=!day;
	set(day);
}
