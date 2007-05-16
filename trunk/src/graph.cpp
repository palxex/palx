#include "allegdef.h"
#include "internal.h"

bitmap::bitmap(const uint8_t *src,int width,int height):
	bmp(create_bitmap(width,height))
{
	if(src)
		memcpy(bmp->dat,src,width*height);
}
bitmap::~bitmap()
{
	destroy_bitmap(bmp);
}
bool bitmap::blit_to(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y)
{
	blit(bmp,dest,source_x,source_y,dest_x,dest_y,screen->w-abs(source_x-dest_x),screen->h-abs(source_y-dest_y));
	return true;
}
sprite::sprite(uint8_t *src):x(0),y(0),l(0),width(0),height(0),buf(src)
{}
sprite::sprite(uint8_t *src,int runtime_x,int runtime_y,int layer,int y_off,int layer_off):x(0),y(0),l(0),width(0),height(0),buf(src)
{
	width=((uint16_t*)src)[0];
	height=((uint16_t*)src)[1];
	x=runtime_x-width/2;
	y=runtime_y+layer+y_off;
	l=layer+layer_off;
}
sprite::~sprite()
{}
void sprite::blit_middle(BITMAP *dest,int x,int y)
{
	if(!dest->dat)
	{
		BITMAP *buf2=create_bitmap(dest->w,dest->h);
		blit(dest,buf2,0,0,0,0,dest->w,dest->h);
		Pal::Tools::DecodeRLE(buf,buf2->dat,dest->w,dest->w,dest->h,x-((uint16_t*)buf)[0]/2,y-((uint16_t*)buf)[1]/2);
		blit(buf2,dest,0,0,0,0,dest->w,dest->h);
	}else
		Pal::Tools::DecodeRLE(buf,dest->dat,dest->w,dest->w,dest->h,x-((uint16_t*)buf)[0]/2,y-((uint16_t*)buf)[1]/2);
}
bool sprite::blit_to(BITMAP *dest)
{
	if(!dest->dat)
	{
		BITMAP *buf2=create_bitmap(dest->w,dest->h);
		blit(dest,buf2,0,0,0,0,dest->w,dest->h);
		Pal::Tools::DecodeRLE(buf,buf2->dat,dest->w,dest->w,dest->h,x,y-l-height);
		blit(buf2,dest,0,0,0,0,dest->w,dest->h);
	}else
		Pal::Tools::DecodeRLE(buf,dest->dat,dest->w,dest->w,dest->h,x,y-l-height);
	return true;
}
bool sprite::blit_to(BITMAP *dest,int x,int y,int l)
{
	this->x=x;
	this->y=y;
	this->l=l;
	blit_to(dest);
	return true;
}

sprite_prim::sprite_prim():id(-1)
{}
sprite_prim::sprite_prim(int _id):id(_id)
{}
sprite_prim::sprite_prim(int _id,uint8_t *src,int x,int y,int layer,int y_off,int layer_off):id(_id)
{
	getsource(src,layer,x,y,y_off,layer_off);
}
sprite_prim::sprite_prim(int _id,uint8_t *src):id(_id)
{
	getsource(src);
}
sprite_prim::sprite_prim(const sprite_prim &rhs):id(rhs.id),sprites(rhs.sprites)
{}
void sprite_prim::getsource(uint8_t *src,int layer,int x,int y,int y_off,int layer_off)
{
	for(int i=0,subfiles=determain_smkfs(src);i<subfiles;i++)
		sprites.push_back(boost::shared_ptr<sprite>(new sprite(src+2*((uint16_t *)src)[i],x,y,layer,y_off,layer_off)));
}
sprite_prim &sprite_prim::getsource(uint8_t *src)
{
	for(int i=0,subfiles=determain_smkfs(src);i<subfiles;i++)
		sprites.push_back(boost::shared_ptr<sprite>(new sprite(src+2*((uint16_t *)src)[i],0,0,0,0,0)));
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
bool operator<(const sprite& lhs,const sprite& rhs)
{
	return lhs.y<rhs.y;
}
bool operator==(const sprite_prim& lhs,const sprite_prim& rhs)
{
	return lhs.id==rhs.id;
}

ALFONT_FONT *ttfont::glb_font;