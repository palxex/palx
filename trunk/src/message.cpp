#include "allegdef.h"
#include "internal.h"
#include "game.h"

int current_dialog_lines = 0;
int glbvar_fontcolor  = 0x4F;
int font_color_yellow = 0x2D;
int font_color_red    = 0x1A;
int font_color_cyan   = 0x8D;
int font_color_cyan_1 = 0x8C;
int frame_pos_flag = 1;
int dialog_x = 12;
int dialog_y = 8;
int frame_text_x = 0x2C;
int frame_text_y = 0x1A;

void show_wait_icon()
{
	game->message_handles.getsprite(0)->blit_to(screen,frame_text_x+80,frame_text_y);
	while(!keypressed()) rest(100);
	clear_keybuf();current_dialog_lines=0;
}

void dialog_firstline(char *str)
{
	ttfont(str).blit_to(screen,dialog_x+1,dialog_y+1,0);
	ttfont(str).blit_to(screen,dialog_x,dialog_y,font_color_cyan_1);
}

void dialog_string(char *str,int lines)
{
	ttfont(str).blit_to(screen,frame_text_x+1,frame_text_y+lines*16+1,0);
	ttfont(str).blit_to(screen,frame_text_x,frame_text_y+lines*16,glbvar_fontcolor);
}