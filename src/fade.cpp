#include "allegdef.h"
#include "internal.h"
#include "game.h"

bool mutex_can_change_palette=false;
void pal_fade_out(int t)
{
	if(!mutex_can_change_palette)
	{
		mutex_can_change_palette=true;
		PALETTE pal;
		if(t)
			for(int i=0x3f;i>=3;i--)
			{
				for(int j=0;j<0x100;j++)
				{
					pal[j].r=game->pat.get()[j].r*i/0x40;
					pal[j].g=game->pat.get()[j].g*i/0x40;
					pal[j].b=game->pat.get()[j].b*i/0x40;
				}
				set_palette(pal);
				rest(t*10);
			}
		memset(&pal,0,sizeof(PALETTE));
		set_palette(pal);
	}
}
void pal_fade_in(int t)
{
	if(mutex_can_change_palette)
	{
		mutex_can_change_palette=false;
		PALETTE pal;
		if(t)
			for(int i=4;i<=0x3f;i++)
			{
				for(int j=0;j<0x100;j++)
				{
					pal[j].r=game->pat.get()[j].r*i/0x40;
					pal[j].g=game->pat.get()[j].g*i/0x40;
					pal[j].b=game->pat.get()[j].b*i/0x40;
				}
				set_palette(pal);
				rest(t*10);
			}
			set_palette(game->pat.get());
	}
}
uint8_t normalize(uint8_t i)
{
	int a=i%0x10,b=i/0x10;
	if(b>a)
		b--;
	else if(b<a)
		b++;
	return b*0x10+a;
}
void normalize_fade()
{
	PALETTE pal;
	for(int i=0;i<0x100;i++)
	{
		pal[i].r=normalize(game->pat.get()[i].r);
		pal[i].g=normalize(game->pat.get()[i].g);
		pal[i].b=normalize(game->pat.get()[i].b);
	}
	set_palette(pal);
}