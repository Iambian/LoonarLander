/*
 *--------------------------------------
 * Program Name: LOONAR LANDER
 *       Author: Rodger "Iambian" Weisman
 *      License: BSD (See LICENSE file)
 *  Description: Land the loon-filled lander on the moon
 *--------------------------------------
*/

#define VERSION_INFO "v0.2"

#define TRANSPARENT_COLOR 0xF8
#define GREETINGS_DIALOG_TEXT_COLOR 0xDF

#define GM_TITLE 0
#define GM_OPENANIM 1
#define GM_GAMEMODE 2
#define GM_CLOSINGANIM 3
#define GM_DYING 4
#define GM_GAMEOVER 5

#define TITLE_TEXT_COLOR 0xEF
#define TITLE_SELECTED_COLOR 0xEC

#define GMBOX_X (LCD_WIDTH/4)
#define GMBOX_Y (LCD_HEIGHT/2-LCD_HEIGHT/8)
#define GMBOX_W (LCD_WIDTH/2)
#define GMBOX_H (LCD_HEIGHT/4)

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
} curx,cury,dx,dy,gravity,thrust;

struct { unsigned int score[4]; uint8_t difficulty; uint8_t flags;} file;

const char *gameoverdesc[] = {"Operator malfulction","The loon got spaced","Fuel seal failure","You had only one job"};
const char *gameoverquit = "You quit the mission";
const char *menuopts[] = {"Start Game","Change Difficulty","About","Quit Game"};
const char *title3 = "[2nd] = Start game";
const char *title4 = "[Mode] = Quit game";
const char *filename = "LOONLDAT";
const char *credits[] = {
	"Push LEFT/RIGHT to move the lander",
	"Push the MODE key to quit",
	"Land softly on the landing pad",
	"or else the lander will explode.",
	"",
	"Program by Rodger 'Iambian' Weisman",
	"Licensed under 2-Clause BSD License",
	"",
	"Report bugs here:",
	"http://cemete.ch/p261077",
};
const char *difftext[] = {"Easy","Medium","Hard","Lowest Bidder"};

/* Put your function prototypes here */

void drawstars();
int gamemode();
void drawdialogbox();
void genstars();
void drawbg();
void drawplayer();
//---
extern void landgen();
void waitanykey();
void keywait();
void centerxtext(char* strobj,int y);
gfx_rletsprite_t* decompress(void* cdata_in);
void drawtitle();
//---
void gameoverdialog(char* s);

/* Put all your globals here */
uint8_t surfaceheight[320];
int stars[2*32];
int landingpadx,landingpadw,fuel;

gfx_rletsprite_t* looner_spr;
gfx_rletsprite_t* lander_spr;
gfx_rletsprite_t* flame1left_spr;
gfx_rletsprite_t* flame2left_spr;
gfx_rletsprite_t* flame1right_spr;
gfx_rletsprite_t* flame2right_spr;
gfx_sprite_t* explosion_spr;



void main(void) {
    int i,score;
	uint8_t temp8,mopt,y;
	kb_key_t k;
	/* Initialize system */
	malloc(0);  //for linking purposes
	gfx_Begin(gfx_8bpp);
	gfx_SetDrawBuffer();
	gfx_SetTransparentColor(TRANSPARENT_COLOR);
	/* Initialize variables */
	looner_spr = decompress(looner_compressed);
	lander_spr = decompress(lander_compressed);
	flame1left_spr = decompress(flame1left_compressed);
	flame2left_spr = decompress(flame2left_compressed);
	flame1right_spr = decompress(flame1right_compressed);
	flame2right_spr = decompress(flame2right_compressed);
	explosion_spr = gfx_MallocSprite(48,48);
	memset(&file,0,sizeof(file));
	
	/* Initiate main game loop */
	genstars();
	mopt = 0;
	while (1) {
		kb_Scan();
		k = kb_Data[1];
		temp8 = randInt(0,1); //keep picking rand 
		if (k&kb_2nd) {
			if (!mopt) {
				score = gamemode();
				if (score > file.score[file.difficulty]) file.score[file.difficulty] = score;
			} else if (mopt == 1) {
				file.difficulty = (file.difficulty+1)&3;
			} else if (mopt == 2) {
				drawstars();
				drawtitle();
				for (i=0,y=100;i<10;i++,y+=12) centerxtext(credits[i],y);
				gfx_SwapDraw();
				waitanykey();
			} else break;
			keywait();
		} else if (k&kb_Mode) break;
		k = kb_Data[7];
		if (k&kb_Down) mopt++;
		if (k&kb_Up) mopt--;
		if (k) { 
			mopt &= 3;
			keywait();
		}
		drawstars();
		drawtitle();
		gfx_SetTextScale(2,2);
		for (i=0,y=100;i<4;i++,y+=24) {
			if (i==mopt) gfx_SetTextFGColor(TITLE_SELECTED_COLOR);
			centerxtext(menuopts[i],y);
			gfx_SetTextFGColor(TITLE_TEXT_COLOR);
		}
		gfx_SwapDraw();
	}
	gfx_End();
//	slot = ti_Open(filename,"w");
//	ti_Write(&highscore,sizeof highscore, 1, slot);
	ti_CloseAll();
	return;
}
#define DMODES_WIDTH 4
//                  grav, trst, xtol, ytol
int16_t dmodes[] = {   7,   18,  256, -888,   //easy
					   7,   18,  192, -444,   //medium
					   7,   18,   96, -192,   //hard
					   7,   18,   64,  -96,   //lowest bidder
};
//Returns score when the player quits or dies
int gamemode() {
	uint8_t timer,lc8;
	kb_key_t k;
	int curlevel,score;
	int i,startpoint;
	uint8_t ytemp,cpos,j,rlen;
	int8_t disp;
	int coly,colx;
	uint8_t gamestate;
	int ytol,xtol,xvariance;

	curlevel = score = 0;
	landingpadw = 60;
	fuel = 999;
	
	gfx_SetTextScale(1,1);
	lc8 = file.difficulty*DMODES_WIDTH;
	gravity.fp = (int) dmodes[lc8+0];
	thrust.fp = (int) dmodes[lc8+1];
	xtol = (int) dmodes[lc8+2];
	ytol = (int) dmodes[lc8+3];
	
	//Endless loop that never breaks. The main gameplay subloop contains
	//a return to escape.
	while (1){
		//------------------------------------------------------------------
		//GENERATE LEVEL
		genstars();
		xvariance = (file.difficulty==3)?2-randInt(0,4):0;
		cury.p.ipart = 8;
		startpoint = curx.p.ipart = (320-32)/2;
		dx.fp = dy.fp = 0;
		
		cpos = randInt(88,215);
		landingpadx = randInt(5,310-5-landingpadw);
		
		for (i=0;i<320;) {
			rlen = randInt(4,8);
			disp = randInt(0,16)-8;
			ytemp = cpos+disp;
			if ((ytemp<80) || (ytemp>220)) continue;  //Rechoose if out of bounds
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
		//------------------------------------------------------------------
		timer=64;
		//OPEN ANIMATION
		while (timer--) {
			drawbg();
			drawplayer();
			drawdialogbox();
			centerxtext((timer&8)?"Get ready!":"",GMBOX_Y+25);
			gfx_SwapDraw();
		}
		//-----------------------------------------------------------------------------
		//GAME MODE (break when progress to next stage, animate, return on game over)
		while (1) {
			drawbg();  //this calls kb_Scan();
			k = kb_Data[1];
			if (k&kb_Mode) return score;
			drawplayer();
			//-- Moving the player and collision detection
			k = kb_Data[7];       //Get player actions
			dy.fp -= gravity.fp;  //always falling
			dx.fp += xvariance;
			if (k&kb_Right && fuel) {
				dx.fp += thrust.fp;
				dy.fp += thrust.fp>>1;
				fuel--;
			}
			if (k&kb_Left && fuel) {
				dx.fp -= thrust.fp;
				dy.fp += thrust.fp>>1;
				fuel--;
			}
			curx.fp -= dx.fp;
			cury.fp -= dy.fp;
			coly = cury.p.ipart+31;
			//main collision detection loop
			gamestate = GM_GAMEMODE;
			for (i=curx.p.ipart,lc8=32;lc8>0;i++,lc8--) {
				if ((i>=0) && (i<320) && (coly>surfaceheight[i])) {
					if ((curx.p.ipart>landingpadx) && (curx.p.ipart<(landingpadx+landingpadw-32))) {
						if (dx.fp>xtol || dx.fp<(-xtol) || dy.fp < ytol) {
							gamestate = GM_DYING;
						} else {
							gamestate = GM_CLOSINGANIM;
						}
					} else {
						gamestate = GM_DYING;
					}
				}
			}
			if (curx.p.ipart<-50 || curx.p.ipart > 370 || cury.p.ipart<-200) {
				gameoverdialog("The loon flew the coop");
				return score;
			}
			if (cury.p.ipart>240) gamestate = GM_DYING;
			
			if (gamestate == GM_CLOSINGANIM) break;
			if (gamestate == GM_DYING) {
				timer = 0;
				while (++timer<15) {
					drawbg();
					if (timer<5) drawplayer();
					if (timer<10) {
						dzx7_Turbo(explosion_ts_tiles_compressed[timer],explosion_spr);
						gfx_TransparentSprite(explosion_spr,curx.p.ipart-8,cury.p.ipart-14);
					}
					gfx_SwapDraw();
				}
				gameoverdialog(gameoverdesc[randInt(0,3)]);
				return score;
			}
			gfx_SwapDraw();
		}
		//-----------------------------------------------------------------------------
		timer=64;
		//STAGE COMPLETED
		while (timer--) {
			drawbg();
			drawplayer();
			drawdialogbox();
			centerxtext("Successful landing!",GMBOX_Y+25);
			gfx_SwapDraw();
		}
		score += fuel;
		fuel += 50*(5-file.difficulty);
		curlevel++;
		if (landingpadw>38) landingpadw--;
	}
}

void drawdialogbox() {
	gfx_SetColor(0x08);  //xlibc dark blue
	gfx_SetTextBGColor(0xFF);
	gfx_FillRectangle(GMBOX_X,GMBOX_Y,GMBOX_W,GMBOX_H);
	gfx_SetTextFGColor(GREETINGS_DIALOG_TEXT_COLOR);
}

void drawstars() {
	uint8_t i;
	gfx_FillScreen(0x00);
	gfx_SetColor(0xFF);
	for (i=0;i<32;i++) gfx_SetPixel(stars[i],stars[(i<<1)+1]);
}

void genstars() {
	uint8_t i;
	for (i=0;i<32;i++) {
		stars[(i<<1)+0] = randInt(0,319); 	//xpos
		stars[(i<<1)+1] = randInt(0,239);   //ypos
	}
}


void drawbg() {
	uint8_t y,i;
	int x;
	
	drawstars();
	landgen();
		
//	gfx_SetColor(0xEF);
//	for (x=0;x<320;x++) {
//		y = surfaceheight[x];
//		gfx_VertLine(x,y,240-y);
//	}
//	gfx_SetColor(0xE4);
//	for (i=0;i<3;i++) {
//		gfx_HorizLine(landingpadx,surfaceheight[landingpadx]+1+i,landingpadw);
//	}
	gfx_SetTextFGColor(0xFE);
	gfx_SetTextBGColor(0x00);
	gfx_SetTextXY(3,3);
	gfx_PrintString("FUEL: ");
	gfx_PrintUInt(fuel,4);
}

void drawplayer() {
	int x,y;
	kb_key_t k;	
	gfx_rletsprite_t* temp;
	static uint8_t timer = 0;
	
	kb_Scan();
	x = curx.p.ipart;
	y = cury.p.ipart;
	timer++;
	k = kb_Data[7];
	
	gfx_RLETSprite(looner_spr,x+6,y+7);
	gfx_RLETSprite(lander_spr,x,y); 
	
	if (k & kb_Left) {
		temp = (timer&1) ? flame1left_spr : flame2left_spr;
		gfx_RLETSprite(temp,x-5,y+21);
	}
	if (k & kb_Right) {
		temp = (timer&1) ? flame1right_spr : flame2right_spr;
		gfx_RLETSprite(temp,x+29,y+21);
	}
	
}

//---------------------------------------------------------------------------

void waitanykey() {
	keywait();            //wait until all keys are released
	while (!kb_AnyKey()); //wait until a key has been pressed.
	keywait();
}	

void keywait() {
	while (kb_AnyKey());  //wait until all keys are released
}

void centerxtext(char* strobj,int y) {
	gfx_PrintStringXY(strobj,(LCD_WIDTH-gfx_GetStringWidth(strobj))/2,y);
}

void* decompress(void *cdata_in) {
	gfx_sprite_t* baseimg;
	baseimg = (void*) gfx_vbuffer;
	dzx7_Turbo(cdata_in,baseimg);
	return gfx_ConvertMallocRLETSprite(baseimg);
}

void drawtitle() {
	gfx_SetTextFGColor(TITLE_TEXT_COLOR);
	gfx_SetTextBGColor(0x00);
	gfx_SetTextScale(4,4);
	centerxtext("LOONAR",5);
	centerxtext("LANDERS",40);
	gfx_SetTextScale(1,1);
	gfx_SetTextXY(5,230);
	gfx_PrintString("High score (");
	gfx_PrintString(difftext[file.difficulty]);
	gfx_PrintString(") : ");
	gfx_PrintInt(file.score[file.difficulty],6);
	gfx_PrintStringXY(VERSION_INFO,290,230);
}

void gameoverdialog(char* s) {
	drawdialogbox();
	centerxtext(s,GMBOX_Y+15);
	centerxtext("Game Over",GMBOX_Y+35);
	gfx_SwapDraw();
	waitanykey();
}