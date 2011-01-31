#include "pprefsmenu.h"
#include "button.h"
#include "common.h"
#include "language.h"
#include <string.h>

char pprefsPrintBuf[128];

#define SPACE_BETWEEN_THE_LINES (LIBM_CHAR_HEIGHT + 2)



#define jms1(c) \
(((0x81 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0x9F)) || ((0xE0 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC) ))

#define jms2(c) \
((0x7F != (unsigned char)(c)) && (0x40 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC))


int str2bytelen(char *str)
{
	int i,ret;
	
	ret = 0;
	for( i = 0; str[i] != '\0'; i ++ ){
		if( jms1(str[i]) && jms2(str[i+1]) ){
			i++;
		}
		ret++;
	}
	
	return ret;
}

/*
int pprefsMakeSelectBox( int start_x, int start_y, const char *itemName , u32 selectKey)
{
	int numOfItem,i,tmp,end_x,end_y;
	SceCtrlData pad;

	i = 0;
	end_x = 0;
	numOfItem = 0;
	while(1){
		numOfItem++;
		if( itemName[i] == '\0' ) break;
		tmp = str2bytelen(itemName);
		if( end_x < tmp ) end_x = tmp;
		i += strlen(itemName) + 1;
	}
	end_x = LIBM_CHAR_WIDTH * end_x + LIBM_CHAR_WIDTH + 6 + start_x;
	end_y = LIBM_CHAR_HEIGHT * numOfItem + 4 + start_y;

	makeWindow( start_x, start_y, end_x, end_y, FG_COLOR, BG_COLOR );
	
	i = 0;
	while( 1 ){
		if( itemName[i] == '\0' ) break;
		libmPrint(start_x + LIBM_CHAR_WIDTH + 4, start_y + i*LIBM_CHAR_HEIGHT,FG_COLOR,BG_COLOR,&itemName[i]);
		i = strlen(itemName) + 1;
	}

	i = 0;
	libmPrint(start_x + 2 ,start_y + i*LIBM_CHAR_HEIGHT,FG_COLOR,BG_COLOR,">");
	while(1){
		wait_button_up(&pad);
		if( pad.Buttons & PSP_CTRL_UP )
		{
			libmPrint(start_x + 2 ,start_y + i*LIBM_CHAR_HEIGHT,FG_COLOR,BG_COLOR," ");
			i--;
			if( i < 0 ) i = numOfItem - 1;
			libmPrint(start_x + 2 ,start_y + i*LIBM_CHAR_HEIGHT,FG_COLOR,BG_COLOR,">");
		}
		else if( pad.Buttons & PSP_CTRL_DOWN )
		{
			libmPrint(start_x + 2 ,start_y + i*LIBM_CHAR_HEIGHT,FG_COLOR,BG_COLOR," ");
			i++;
			if( i >= numOfItem ) i = 0;
			libmPrint(start_x + 2 ,start_y + i*LIBM_CHAR_HEIGHT,FG_COLOR,BG_COLOR,">");
		}
		else if( pad.Buttons & selectKey )
		{
			return i;
		}
	}
}
*/


int pprefsMakeSelectBox( int start_x, int start_y,char *titleLabel, char *itemName[] , u32 selectKey, int type)
{

	int numOfItem,i,tmp,end_x,end_y;
	SceCtrlData pad;

	end_x = str2bytelen(titleLabel);
	for( i = 0; itemName[i] != NULL; i++ ){
		tmp = str2bytelen(itemName[i]);
		if( end_x < tmp ) end_x = tmp;
	}
	numOfItem = i;
	
	end_x = LIBM_CHAR_WIDTH * end_x + LIBM_CHAR_WIDTH + 6 + start_x;
	end_y = start_y + SPACE_BETWEEN_THE_LINES * numOfItem + SPACE_BETWEEN_THE_LINES + 4;
	
	start_y += 2;



	makeWindow( start_x , start_y , end_x, end_y, FG_COLOR, BG_COLOR );

	
	libmPrint(start_x + 4 , start_y - 2 ,FG_COLOR,BG_COLOR,titleLabel);

	libmFillRect(0 , 254 , 480 , 272 ,BG_COLOR);
	libmPrintf(5,264, EX_COLOR ,BG_COLOR,( type == 0 )?PPREFSMSG_SELECTBOX_HOTOUSE:PPREFSMSG_SELECTBOX_HOTOUSE_2,buttonData[buttonNum[0]].name);

	for( i = 0; i < numOfItem; i++ ){
		libmPrint(start_x + LIBM_CHAR_WIDTH + 4, start_y + (i+1)*SPACE_BETWEEN_THE_LINES + 2,FG_COLOR,BG_COLOR,itemName[i]);
	}

	i = 0;
	libmPrint(start_x + 2 ,start_y + (i+1)*SPACE_BETWEEN_THE_LINES + 2,FG_COLOR,BG_COLOR,">");
	while(1){
		wait_button_up(&pad);
		get_button(&pad);
		if( pad.Buttons & PSP_CTRL_UP )
		{
			libmPrint(start_x + 2 ,start_y + (i+1)*SPACE_BETWEEN_THE_LINES+ 2,FG_COLOR,BG_COLOR," ");
			i--;
			if( i < 0 ) i = numOfItem - 1;
			libmPrint(start_x + 2 ,start_y + (i+1)*SPACE_BETWEEN_THE_LINES+ 2,FG_COLOR,BG_COLOR,">");
		}
		else if( pad.Buttons & PSP_CTRL_DOWN )
		{
			libmPrint(start_x + 2 ,start_y + (i+1)*SPACE_BETWEEN_THE_LINES+ 2,FG_COLOR,BG_COLOR," ");
			i++;
			if( i >= numOfItem ) i = 0;
			libmPrint(start_x + 2 ,start_y + (i+1)*SPACE_BETWEEN_THE_LINES+ 2,FG_COLOR,BG_COLOR,">");
		}
		else if( pad.Buttons & selectKey )
		{
			wait_button_up(&pad);
			return i;
		}
		else if( type != 0 && pad.Buttons & PSP_CTRL_HOME )
		{
			wait_button_up(&pad);
			return -1;
		}
	}
}


void pprefsSleep(clock_t sleepTime)
{
	clock_t time = sceKernelLibcClock();
	
	while( (sceKernelLibcClock()- time) < sleepTime );
}

//一瞬で窓を作る
//この関数マクロで作れる気がする
void makeWindowQuick(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor)
{
	libmFillRect(sx , sy , ex , ey , bgcolor );
	libmFrame(sx , sy , ex ,ey , fgcolor );
}

//にゅーってでる窓を作る
void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor)
{
	int nowx = sx,nowy = sy;
	while(1){
		nowx += 8;
		nowy += 8;
		if( nowx > ex ) nowx = ex;
		if( nowy > ey ) nowy = ey;
		libmFillRect(sx , sy , nowx , nowy , bgcolor );
		libmFrame(sx , sy , nowx ,nowy , fgcolor );
		if( nowx == ex && nowy == ey ) break;
		pprefsSleep(8 * 1000);
	}
	
}

//にゅーってしている最中にもボタンをgetする
void makeWindowWithGettingButton(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor, SceCtrlData *pad)
{
	int nowx = sx,nowy = sy;
	while(1){
		get_button(pad);
		nowx += 8;
		nowy += 8;
		if( nowx > ex ) nowx = ex;
		if( nowy > ey ) nowy = ey;
		libmFillRect(sx , sy , nowx , nowy , bgcolor );
		libmFrame(sx , sy , nowx ,nowy , fgcolor );
		if( nowx == ex && nowy == ey ) break;
		pprefsSleep(8 * 1000);
	}
	
}


