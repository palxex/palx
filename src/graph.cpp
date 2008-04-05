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
#include "UI.h"

bitmap::bitmap(const uint8_t *src,int w,int h):
	bmp(create_bitmap(w,h)),width(w),height(h)
{
	if(src)
		memcpy(bmp->dat,src,width*height);
}
bitmap::bitmap(BITMAP *b):bmp(create_bitmap(b->w,b->h)),width(b->w),height(b->h)
{
    blit(b,bmp,0,0,0,0,b->w,b->h);
}
bitmap::bitmap(const bitmap &rhs):bmp(create_bitmap(rhs.width,rhs.height)),width(rhs.width),height(rhs.height)
{
	blit(rhs,bmp,0,0,0,0,width,height);
}
bitmap::~bitmap()
{
	destroy_bitmap(bmp);
}
uint8_t *bitmap::getdata()
{
	return (uint8_t*)bmp->dat;
}
bool bitmap::blit_to(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y)
{
	blit(bmp,dest,source_x,source_y,dest_x,dest_y,std::max(bmp->w,dest->w),std::max(bmp->h,dest->h));
	if(dest==screen)
		flush_screen();
	return true;
}
bool do_nothing(int srcVal, uint8* pOutVal, void* pUserData){return false;}
sprite::sprite(uint8_t *src):buf(src),filter(do_nothing),filt_data(0),x(0),y(0),l(0),width(0),height(0)
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
sprite *sprite::setXYL(int x,int y,int l)
{
	this->x=x-width/2;
	this->y=y;
	this->l=l;
	return this;
}

void sprite::setfilter(filter_func r,int data)
{
	filter=r;filt_data=data;
}
void sprite::setfilter()
{
	filter=do_nothing;
	filt_data=0;
}
void sprite::blit_middle(BITMAP *dest,int x,int y)
{
	this->x=x-((uint16_t*)buf)[0]/2;
	this->y=y+((uint16_t*)buf)[1]/2;
	this->l=0;
	blit_to(dest);
}
void sprite::blit_middlebottom(BITMAP *dest,int x,int y)
{
	this->x=x-((uint16_t*)buf)[0]/2;
	this->y=y;
	this->l=0;
	blit_to(dest);
}
bool sprite::blit_to(BITMAP *dest)
{
	if(!dest->dat)
	{
		BITMAP *buf2=create_bitmap(dest->w,dest->h);
		blit(dest,buf2,0,0,0,0,dest->w,dest->h);
		Pal::Tools::DecodeRle(buf,buf2->dat,dest->w,dest->h,x,y-l-height,filt_data?filter:do_nothing,(void*)&filt_data);
		blit(buf2,dest,0,0,0,0,dest->w,dest->h);
		destroy_bitmap(buf2);
	}else
		Pal::Tools::DecodeRle(buf,dest->dat,dest->w,dest->h,x,y-l-height,filt_data?filter:do_nothing,(void*)&filt_data);
	return true;
}

bool sprite::blit_to(BITMAP *dest,int x,int y,bool shadow,int sx,int sy)
{
	if(shadow){
		this->x=x+sx;
		this->y=y+height+sy;
		this->l=0;
		setfilter(shadow_filter,1);
		blit_to(dest);
		setfilter();
	}

	this->x=x;
	this->y=y+height;
	this->l=0;
	return blit_to(dest);
}
void sprite::blit_filter(BITMAP *dest,int x,int y,filter_func r,int data,bool is,bool middlebottom)
{
	if(is)
		setfilter(r,data);
	if(middlebottom)
		blit_middlebottom(dest,x,y);
	else
		blit_to(dest,x,y);
	if(is)
		setfilter();
}

sprite_prim::sprite_prim():id(-1)
{}
sprite_prim::sprite_prim(int _id):id(_id)
{}
sprite_prim::sprite_prim(cached_res &archive,int _id):id(_id)
{
	getsource(archive);
}
sprite_prim::sprite_prim(const sprite_prim &rhs):id(rhs.id),sprites(rhs.sprites)
{}
sprite_prim &sprite_prim::getsource(cached_res &archive,int _id)
{
	if(_id!=-1)
		id=_id;
	frame=archive.slices(id);
	for(int i=0;i<frame;i++)
		sprites.push_back(boost::shared_ptr<sprite>(new sprite(archive.decode(id,i))));
	return *this;
}
int sprite_prim::frames() const
{
	return frame;
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

ALFONT_FONT *ttfont::glb_font=NULL;
ttfont::ttfont()
{
	using namespace std;
	if(!glb_font){
        alfont_init();
        glb_font=alfont_load_font(global->get<string>("font","path").c_str());
		alfont_set_language(glb_font, global->get<string>("config","encode").c_str());
        alfont_set_convert(glb_font, TYPE_WIDECHAR);
        alfont_text_mode(-1);
        alfont_set_font_background(glb_font, FALSE);
        alfont_set_char_extra_spacing(glb_font,1);
        alfont_set_font_size(glb_font,16);
	}
}
void ttfont::blit_to(const char *msg,BITMAP *dest,int x,int y,uint8_t color,bool shadow){
	if(shadow)
		alfont_textout(dest, glb_font, msg, x+1, y+1, 0);
	alfont_textout(dest, glb_font, msg, x, y, color);
}

uint16_t pixfont::worbuf[10000];
uint8_t pixfont::fonbuf[300000];
int nChar;
pixfont::pixfont()
{
   FILE *fp;

   //
   // Load the wor16.asc file.
   //
   fp = fopen((global->get<std::string>("config","path")+"/WOR16.ASC").c_str(), "rb");

   //
   // Get the size of wor16.asc file.
   //
   fseek(fp, 0, SEEK_END);
   nChar = ftell(fp);
   nChar /= 2;

   fseek(fp, 0, SEEK_SET);
   fread(worbuf, 2, nChar, fp);

   //
   // Close wor16.asc file.
   //
   fclose(fp);

   //
   // Read all bitmaps from wor16.fon file.
   //
   fp = fopen((global->get<std::string>("config","path")+"/WOR16.FON").c_str(), "rb");

   //
   // The font glyph data begins at offset 0x682 in wor16.fon.
   //
   fseek(fp, 0x682, SEEK_SET);
   fread(fonbuf, 30, nChar, fp);
   fclose(fp);
}
void pixfont::blit_to(const char *msg, BITMAP *dest, int x, int y, unsigned char color, bool shadow)
{
	for(int word=0,len=strlen(msg)/2;word<len;word++)
	{
		uint16_t wChar=((uint16_t*)msg)[word];
   int i=0, j=0, dx=0;

   //
   // Locate for this character in the font lib.
   //
   for (i = 0; i < nChar; i++)
   {
      if (worbuf[i] == wChar)
      {
         break;
      }
   }

   if (i >= nChar)
   {
      //
      // This character does not exist in the font lib.
      //
      return;
   }

   uint8_t *pChar = fonbuf + i * 30;

   //
   // Draw the character to the surface.
   //
   int dy = y * SCREEN_W;
   for (i = 0; i < 30; i++)
   {
      dx = x + word*16 + ((i & 1) << 3);
      for (j = 0; j < 8; j++)
      {
         if (pChar[i] & (1 << (7 - j)))
         {
			 if(shadow)
				 ((uint8_t*)dest->dat)[dy + SCREEN_W + dx + 1] = 0;
            ((uint8_t*)dest->dat)[dy + dx] = color;
         }
         dx++;
      }
      dy += (i & 1) * SCREEN_W;
   }
	}
}

palette::palette()
	:pal(-1),day(-1)
{}
void palette::read(uint32_t i)
{
	pal=i;
	bool fx;long len;
	myRGB *buf=(myRGB *)Pal::PAT.decode(i,0,fx,len);
	RGB   *p=(RGB*)pat;
	for(int t=0;t<((i==0 || i==5)?512:256);t++)
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

BITMAP *fakescreen,*backbuf,*bakscreen;
#undef screen
BITMAP *realscreen=screen;
