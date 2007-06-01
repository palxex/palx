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

int icon_x=0;
int icon_y=0;
int icon=0;

void show_wait_icon()
{
	game->message_handles.getsprite(icon)->blit_to(screen,icon_x,icon_y);
	while(!keypressed()) rest(10);
	clear_keybuf();
	current_dialog_lines=0;
	icon=0;
}

void dialog_firstline(char *str)
{
	ttfont(str).shadow_blit(screen,dialog_x,dialog_y,font_color_cyan_1);
}

void dialog_string(char *str,int lines)
{
	static char word[3];
	int text_x=frame_text_x;
	for(int i=0,len=(int)strlen(str);i<len;i++)
		switch(str[i]){
			case '-':
				std::swap(glbvar_fontcolor, font_color_cyan);
				break;
			case '\'':
				std::swap(glbvar_fontcolor, font_color_red);
				break;
			case '\"':
				std::swap(glbvar_fontcolor, font_color_yellow);
				break;
			case '$':
				break;
			case '~':
				break;
			case ')':
				icon=1;
				break;
			case '(':
				icon=2;
				break;
			default:
				strncpy(word,str+i,2);
				ttfont(word).shadow_blit(screen,text_x,frame_text_y+lines*16,glbvar_fontcolor);
				icon_x=text_x+=16;
				i++;
	}
	icon_y=frame_text_y+lines*16;
}