#include "allegdef.h"

int get_key()
{
	int vk=0;
	if(keypressed())
		switch(int k=(readkey()>>8)){
		case KEY_ESC:
		case KEY_INSERT:
		case KEY_ALT:
			vk = VK_MENU;
			break;
		case KEY_ENTER:
		case KEY_SPACE:
			vk = VK_EXPLORE;
			break;
		case KEY_LEFT:
			vk = VK_LEFT;
			break;
		case KEY_UP:
			vk = VK_UP;
			break;
		case KEY_RIGHT:
			vk = VK_RIGHT;
			break;
		case KEY_DOWN:
			vk = VK_DOWN;
			break;
		case KEY_R:
			vk = VK_REPEAT;
			break;
		case KEY_A:
			vk = VK_AUTO;
			break;
		case KEY_D:
			vk = VK_DEFEND;
			break;
		case KEY_E:
			vk = VK_USE;
			break;
		case KEY_W:
			vk = VK_THROW;
			break;
		case KEY_Q:
			vk = VK_QUIT;
			break;
		case KEY_S:
			vk = VK_STATUS;
			break;
		case KEY_F:
			vk = VK_FORCE;
			break;
		default:
			vk = k;
		}
	if(key_shifts & KB_CTRL_FLAG)
		vk = VK_EXPLORE;
	clear_keybuf();
	return vk;
}