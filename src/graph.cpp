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
	int w=(dest->w-dest_x<bmp->w?dest->w-dest_x:bmp->w);
	int h=(dest->h-dest_y<bmp->h?dest->h-dest_y:bmp->h);
	blit(bmp,dest,source_x,source_y,dest_x,dest_y,w,h);
	return true;
}
sprite::sprite(uint8_t *src):x(0),y(0),l(0),width(0),height(0),buf(src)
{}
sprite::sprite(uint8_t *src,int layer,int y_off,int layer_off):x(0),y(0),l(0),width(0),height(0),buf(src)
{
	width=((uint16_t*)src)[0];
	height=((uint16_t*)src)[1];
	x=x_scrn_offset-width/2;
	y=y_scrn_offset+layer+y_off;
	l=layer+layer_off;
}
sprite::~sprite()
{}
bool sprite::blit_to(BITMAP *dest)
{
	Pal::Tools::DecodeRLE(buf,dest->dat,dest->w,dest->w,dest->h,x,y-l-height);
	return true;
}
bool sprite::blit_to(BITMAP *dest,int dest_x,int dest_y)
{
	Pal::Tools::DecodeRLE(buf,dest->dat,dest->w,dest->w,dest->h,dest_x,dest_y);
	return true;
}

sprite_prim::sprite_prim():id(-1)
{}
sprite_prim::sprite_prim(int _id):id(_id)
{}
sprite_prim::sprite_prim(int _id,uint8_t *src,int layer,int y_off,int layer_off):id(_id)
{
	getsource(src,layer,y_off,layer_off);
}
sprite_prim::sprite_prim(int _id,uint8_t *src):id(_id)
{
	getsource(src);
}
sprite_prim::sprite_prim(const sprite_prim &rhs):id(rhs.id),sprites(rhs.sprites)
{}
void sprite_prim::getsource(uint8_t *src,int layer,int y_off,int layer_off)
{
	for(int i=0,subfiles=determain_smkfs(src);i<subfiles;i++)
		sprites.push_back(boost::shared_ptr<sprite>(new sprite(src+2*((uint16_t *)src)[i],layer,y_off,layer_off)));
}
void sprite_prim::getsource(uint8_t *src)
{
	for(int i=0,subfiles=determain_smkfs(src);i<subfiles;i++)
		sprites.push_back(boost::shared_ptr<sprite>(new sprite(src+2*((uint16_t *)src)[i],0,0,0)));
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