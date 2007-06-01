#include "UI.h"

dialog::dialog(int style,int x,int y,int rows,int columns)
{
	rows--;columns--;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			border[i][j]=game->UIpics.getsprite(i*3+j+style);
	for(int i=0;i<2+rows;i++)
	{
		static int len=0;
		int ti,j=0;
		if(i==0)
			ti=0;
		else if(i==1+rows)
				ti=2;
		else
			ti=1; 
		border[ti][0]->blit_to(screen,x,y+len);
		for(;j<columns;j++)
			border[ti][1]->blit_to(screen,x+border[ti][0]->width+j*border[ti][1]->width,y+len);
		border[ti][2]->blit_to(screen,x+border[ti][0]->width+j*border[ti][1]->width,y+len);
		len+=border[ti][1]->height;
	}
	PALETTE pal;get_palette(pal);save_bitmap("test.bmp",screen,pal);
}