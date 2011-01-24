
#ifndef __PPREFSMENU_H_
#define __PPREFSMENU_H_
#include <pspkernel.h>
#include "libmenu.h"

#include "color.h"

#define FG_COLOR BLACK
#define BG_COLOR WHITE


#define PRINT_SCREEN() \
libmFillRect( 0 , 0 , 480 , 272 , BG_COLOR); \
libmPrint(10,10,FG_COLOR,BG_COLOR,"ptextviewer Ver. 1.00   by hiroi01"); \


extern char pprefsPrintBuf[128];
#define libmPrintf(x,y,fg,bg,format, ... ) libmPrintfXY(x,y,fg,bg,pprefsPrintBuf,COMMON_BUF_LEN,format, ##__VA_ARGS__)
#define libmPrint libmPrintXY
#define fillLine(sy,color) libmFillRect( 0 , sy , 480 , sy + LIBM_CHAR_HEIGHT ,color);


void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);
void makeWindowWithGettingButton(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor, SceCtrlData *pad);


#endif
