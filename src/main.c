/*
 *--------------------------------------
 * Program Name: LOONAR LANDER
 *       Author: Rodger "Iambian" Weisman
 *      License: BSD (See LICENSE file)
 *  Description: Land the loon-filled lander on the moon
 *--------------------------------------
*/

#define VERSION_INFO "v0.1"

#define TRANSPARENT_COLOR 0xF8
#define GREETINGS_DIALOG_TEXT_COLOR 0xDF
#define BULLET_TABLE_SIZE 512
#define ENEMY_TABLE_SIZE 32

#define GM_TITLE 0
#define GM_OPENANIM 1
#define GM_GAMEMODE 2
#define GM_CLOSINGANIM 3
#define GM_DYING 4
#define GM_GAMEOVER 5
#define GM_OUTPOSTSHOP 6
#define GM_RETURNTOBASE 7

#define GMBOX_X (LCD_WIDTH/4)
#define GMBOX_Y (LCD_HEIGHT/2-LCD_HEIGHT/8)
#define GMBOX_W (LCD_WIDTH/2)
#define GMBOX_H (LCD_HEIGHT/4)

#define ACCELFACTOR ((int)(0.07*256))

/* Standard headers (recommended) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External library headers */
#include <debug.h>
#include <intce.h>
#include <keypadc.h>
#include <graphx.h>
#include <decompress.h>
#include <fileioc.h>

#include "gfx/sprites_gfx.h"
#include "gfx/explosion_gfx.h"



union fp16_8 {
	int fp;
	struct {
		uint8_t fpart;
		int16_t ipart;
	} p;
} curx,cury,dx,dy,cura,da,tempfp1,tempfp2,gravity,thrust;

const char *gameoverdesc[] = {"Operator malfulction","The loon got spaced","Fuel seal failure","You had only one job"};
const char *gameoverquit = "You quit the mission";
const char *gameovertext = "Game Over";
const char *successlanding = "Successful landing!";
const char *getready = "Get ready!";
const char *blankstr = "";
const char *title1 = "LOONAR";
const char *title2 = "LANDERS";
const char *title3 = "[2nd] = Start game";
const char *title4 = "[Mode] = Quit game";
const char *title5 = "High score: ";

/* Put your function prototypes here */

void drawbg();
void drawplayer();
void moveplayer();
//---
void putaway();
void waitanykey();
void keywait();
void drawdebug();
void centerxtext(char* strobj,int y);
void* decompress(void* cdata_in, int out_size);
//---
void genlevel();

/* Put all your globals here */
int debugvalue;
uint8_t gamemode;
uint8_t maintimer;
int8_t menuoption;

uint8_t surfaceheight[320];
int stars[2*32];

int landingpadx;
int landingpadw;
int curlevel;
int fuel;
int score;
int highscore;

gfx_sprite_t* looner_spr;
gfx_sprite_t* lander_spr;
gfx_sprite_t* flame1left_spr;
gfx_sprite_t* flame2left_spr;
gfx_sprite_t* flame1right_spr;
gfx_sprite_t* flame2right_spr;
gfx_sprite_t* explosion_spr;
gfx_sprite_t* tmp_ptr;



void main(void) {
    int x,y,i,j,temp,subtimer;
	kb_key_t k;
	char* tmp_str;
	/* Initialize system */
	malloc(0);  //for linking purposes
	gfx_Begin(gfx_8bpp);
	gfx_SetDrawBuffer();
	gfx_SetTransparentColor(TRANSPARENT_COLOR);
	gfx_SetClipRegion(0,0,320,240);
	/* Initialize variables */
	looner_spr = decompress(looner_compressed,looner_size);
	lander_spr = decompress(lander_compressed,lander_size);
	flame1left_spr = decompress(flame1left_compressed,flame1left_size);
	flame2left_spr = decompress(flame2left_compressed,flame2left_size);
	flame1right_spr = decompress(flame1right_compressed,flame1right_size);
	flame2right_spr = decompress(flame2right_compressed,flame2right_size);
	explosion_spr = gfx_MallocSprite(48,48);
	highscore = 0;
	
	/* Initiate main game loop */
	gamemode = GM_TITLE;
	maintimer = 0;
	menuoption = 0;
	curlevel = 0;
	genlevel();  //just for the star background
	while (1) {
		kb_Scan();
		switch (gamemode) {
			case GM_TITLE:
				k = kb_Data[1];
				i = randInt(0,255);  //keep picking rands to cycle
				if (k == kb_2nd) {
					landingpadw = 50;
					gamemode = GM_OPENANIM;
					gravity.fp = (int) (0.03*256);
					thrust.fp = (int) (0.07*256);
					genlevel();
					maintimer = 0;
					fuel = 999;
					score = 0;
					break;
				}
				if (k == kb_Mode) putaway();
				
				gfx_FillScreen(0x00);
				gfx_SetColor(0xFF);
				for (i=0;i<32;i++) gfx_SetPixel(stars[(i*2)+0],stars[(i*2)+1]);
				
				gfx_SetTextFGColor(0xEF);
				gfx_SetTextScale(4,4);
				centerxtext(title1,5);
				gfx_SetTextScale(4,4);
				centerxtext(title2,40);
				gfx_SetTextScale(2,2);
				centerxtext(title3,120);
				centerxtext(title4,144);
				gfx_SetTextScale(1,1);
				gfx_SetTextXY(5,230);
				gfx_PrintString(title5);
				gfx_PrintInt(highscore,6);
				gfx_SetTextXY(290,230);
				gfx_PrintString(VERSION_INFO);
				gfx_SwapDraw();
				break;
			case GM_OPENANIM:
				//game gfx
				drawbg();
				drawplayer();
				gfx_SetColor(0x08);  //xlibc dark blue
				gfx_FillRectangle(GMBOX_X,GMBOX_Y,GMBOX_W,GMBOX_H);
				gfx_SetTextFGColor(GREETINGS_DIALOG_TEXT_COLOR);
				tmp_str = (maintimer & 0x08) ? getready : blankstr;
				gfx_GetStringWidth(tmp_str);
				centerxtext(tmp_str,GMBOX_Y+25);
				gfx_SwapDraw();
				if (maintimer>64) gamemode = GM_GAMEMODE;
				break;
			case GM_GAMEMODE:
				k = kb_Data[1];
				if (k & kb_Mode) {
					gamemode = GM_GAMEOVER;
					break;
				}
				drawbg();
				drawplayer();
				moveplayer();
				gfx_SwapDraw();
				break;
			case GM_DYING:
				drawbg();
				if (maintimer<5) drawplayer();
				if (maintimer<10) {
					dzx7_Turbo(explosion_ts_tiles_compressed[maintimer],explosion_spr);
					gfx_TransparentSprite(explosion_spr,curx.p.ipart-8,cury.p.ipart-14);
				}
				if (maintimer>15) {
					gamemode = GM_GAMEOVER;
					keywait();
				}
				gfx_SwapDraw();
				break;
			case GM_GAMEOVER:
				gfx_SetColor(0x08);  //xlibc dark blue
				gfx_FillRectangle(GMBOX_X,GMBOX_Y,GMBOX_W,GMBOX_H);
				tmp_str = gameoverdesc[randInt(0,3)];
				if (kb_Data[1] == kb_Mode) tmp_str = gameoverquit;
				gfx_GetStringWidth(tmp_str);
				gfx_SetTextFGColor(GREETINGS_DIALOG_TEXT_COLOR);
				centerxtext(tmp_str,GMBOX_Y+15);
				centerxtext(gameovertext,GMBOX_Y+35);
				gfx_SwapDraw();
				waitanykey();
				gamemode = GM_TITLE;
				if (score>highscore) {
					highscore = score;
				}
				break;
			case GM_CLOSINGANIM:
				//At given rate of outpost return, maintimer maxes at 112
				drawbg();
				drawplayer();
				gfx_SetColor(0x08);  //xlibc dark blue
				gfx_FillRectangle(GMBOX_X,GMBOX_Y,GMBOX_W,GMBOX_H);
				gfx_SetTextFGColor(GREETINGS_DIALOG_TEXT_COLOR);
				gfx_GetStringWidth(successlanding);
				centerxtext(successlanding,GMBOX_Y+25);
				gfx_SwapDraw();
				if (maintimer>64) {
					gamemode = GM_OPENANIM;
					score += fuel;
					fuel += 100;
					curlevel++;
					//landing pad scaling: shrinks by one pixel every 5 levels, min 38
					temp = 50 - (curlevel%5);
					landingpadw = (temp>38) ? temp : 38;
					genlevel();
					maintimer=0;
				}
				break;
				
			default:
				putaway();
				break;
		}
		maintimer++;
	}
	
	

//	for (i=0;i<32;i++) enemies[i] = emptyenemy; //NOT NEEDED ANYMORE BUT KEPT FOR REFERENCE
//	ti_CloseAll();
//	slot = ti_Open("LLOONDAT","r");
//	if (slot) ti_Read(&highscore,sizeof highscore, 1, slot);
//	ti_CloseAll();
//	loonsub_sprite = gfx_MallocSprite(32,32);
//	dzx7_Turbo(loonsub_compressed,loonsub_sprite);

}


void drawbg() {
	uint8_t y,i;
	int x;
	gfx_FillScreen(0x00);
	for (x=0;x<320;x++) {
		//gfx_SetPixel(x,surfaceheight[x]);
		y = surfaceheight[x];
		gfx_SetColor(0xEF);
		gfx_VertLine(x,y,240-y);
		if ((x>landingpadx) && (x<(landingpadx+landingpadw))) {
			gfx_SetColor(0xE4);
			gfx_VertLine(x,y+1,3);
		}
	}
	gfx_SetColor(0xFF);
	for (i=0;i<32;i++) {
		gfx_SetPixel(stars[(i*2)+0],stars[(i*2)+1]);
	}
	
	gfx_SetTextFGColor(0xFE);
	gfx_SetTextXY(3,3);
	gfx_PrintString("FUEL: ");
	gfx_PrintInt(fuel,3);
}

void drawplayer() {
	int x,y;
	gfx_sprite_t* temp;
	x = curx.p.ipart;
	y = cury.p.ipart;
	gfx_Sprite(looner_spr,x+6,y+7);
	gfx_TransparentSprite(lander_spr,x,y);
	if (kb_Data[7] & kb_Left) {
		temp = (maintimer&1) ? flame1left_spr : flame2left_spr;
		gfx_TransparentSprite(temp,x-5,y+21);
	}
	if (kb_Data[7] & kb_Right) {
		temp = (maintimer&1) ? flame1right_spr : flame2right_spr;
		gfx_TransparentSprite(temp,x+29,y+21);
	}
	
}

void moveplayer() {
	int i,coly;
	dy.fp -= gravity.fp;  //always that falling feeling
	if ((kb_Data[7] & kb_Right) && fuel>0) {
		dx.fp += thrust.fp;
		dy.fp += thrust.fp/2;
		fuel--;
	}
	if ((kb_Data[7] & kb_Left) && fuel>0) {
		dx.fp -= thrust.fp;
		dy.fp += thrust.fp/2;
		fuel--;
	}
	curx.fp -= dx.fp;
	cury.fp -= dy.fp;
	coly = cury.p.ipart+31;
	for (i=curx.p.ipart;i<(curx.p.ipart+32);i++) {
		if ((i>=0) && (i<320)) {
			if (coly>surfaceheight[i]) {
				if ((curx.p.ipart>landingpadx) && (curx.p.ipart<(landingpadx+landingpadw-32))) {
					maintimer = 0;
					if (dx.fp>64 || dx.fp<-64 || dy.fp < -192) {
						gamemode = GM_DYING;
					} else {
						gamemode = GM_CLOSINGANIM;
					}
				} else {
					gamemode = GM_DYING;
					maintimer = 0;
				}
			}
		}
	}
}



//---------------------------------------------------------------------------

void putaway() {
//	int_Reset();
	gfx_End();
//	slot = ti_Open("LLOONDAT","w");
//	ti_Write(&highscore,sizeof highscore, 1, slot);
	ti_CloseAll();
	exit(0);
}

void waitanykey() {
	keywait();            //wait until all keys are released
	while (!kb_AnyKey()); //wait until a key has been pressed.
	while (kb_AnyKey());  //make sure key is released before advancing
}	

void keywait() {
	while (kb_AnyKey());  //wait until all keys are released
}

void drawdebug() {
	static int i=0;
	i++;
	gfx_SetTextFGColor(0xFE);
	/*
	gfx_SetTextXY(10,5);
	gfx_PrintInt(i,4);
	gfx_SetTextXY(50,5);
	gfx_PrintChar('X');
	gfx_PrintChar(':');
	gfx_PrintInt(curx.p.ipart,5);
	gfx_SetTextXY(120,5);
	gfx_PrintChar('Y');
	gfx_PrintChar(':');
	gfx_PrintInt(cury.p.ipart,4);
	*/
	gfx_SetTextXY(10,15);
	gfx_PrintInt(debugvalue,4); /*
	gfx_SetTextXY(50,15);
	gfx_PrintChar('D');
	gfx_PrintChar('X');
	gfx_PrintChar(':');
	gfx_PrintInt(dx.p.ipart,5);
	gfx_SetTextXY(120,15);
	gfx_PrintChar('D');
	gfx_PrintChar('Y');
	gfx_PrintChar(':');
	gfx_PrintInt(dy.p.ipart,4);
	
	gfx_SetTextXY(50,25);
	gfx_PrintChar('M');
	gfx_PrintChar('F');
	gfx_PrintChar(':');
	gfx_PrintInt(movefactor,5);
	gfx_SetTextXY(120,25);
	gfx_PrintChar('M');
	gfx_PrintChar('S');
	gfx_PrintChar(':');
	gfx_PrintInt(maxspeed.fp,4);
	*/
}

void centerxtext(char* strobj,int y) {
	gfx_PrintStringXY(strobj,(LCD_WIDTH-gfx_GetStringWidth(strobj))/2,y);
}

void* decompress(void *cdata_in,int out_size) {
	void *ptr = malloc(out_size);
	dzx7_Turbo(cdata_in,ptr);
	return ptr;
}

//----------------------------------------------------------------------------------

void genlevel() {
	int i,cpos,tmp,j,rlen,disp,startpoint,endpoint;
	cury.p.ipart = 8;
	startpoint = curx.p.ipart = (320-32)/2;
	dx.fp = dy.fp = cura.fp = da.fp = 0;
	
	for (i=0;i<32;i++) {
		stars[(i*2)+0] = randInt(0,319); //xpos
		stars[(i*2)+1] = randInt(0,239); //ypos
	}
	
	cpos = randInt(88,215);
	landingpadx = randInt(5,310-5-landingpadw);
	
	for (i=0;i<320;) {
		rlen = randInt(4,8);
		disp = randInt(0,16)-8;
		tmp = cpos+disp;
		if ((tmp<80) || (tmp>220)) continue;  //Rechoose if out of bounds
		for (j=0;((j<rlen)&&(i<320));j++,i++) {
			surfaceheight[i] = cpos;
			if ((i<landingpadx) || (i>(landingpadx+landingpadw))) {
				if (disp>0) {
					cpos++;
					disp--;
				}
				if (disp<0) {
					cpos--;
					disp++;
				}
			}
		}
	}
}

