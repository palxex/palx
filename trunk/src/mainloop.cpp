#include "game.h"
int game::run(){
	bool running=true;
	bitmap fbp1(FBP.decode(1,0).get(),320,200);
	sprite abc1(F.decode(0,0).get());
	ttfont fon1("m.msg");
	BITMAP *bmp=create_bitmap(320,200);
	fbp1.blit_to(bmp,0,0,0,0);
	abc1.blit_to(bmp,150,100);
	fon1.blit_to(bmp,msg_idxes[0],msg_idxes[1],50,50);
	blit(bmp,screen,0,0,0,0,320,200);
	while(running) rest(10000);
	return 1;
}