
#ifndef __PPREFSMENU_H_
#define __PPREFSMENU_H_
#include <pspkernel.h>
#include <cmlibmenu.h>
#include "common.h"
#include "color.h"

#define FG_COLOR config.color0
#define BG_COLOR config.color1
#define SL_COLOR config.color2
#define EX_COLOR config.color3
#define ON_COLOR config.color4
#define OF_COLOR config.color5

#define FG_COLOR_DEFAULT WHITE
#define BG_COLOR_DEFAULT BLACK
#define SL_COLOR_DEFAULT RED
#define EX_COLOR_DEFAULT SILVER
#define ON_COLOR_DEFAULT WHITE
#define OF_COLOR_DEFAULT GRAY


extern char pprefsPrintBuf[128];

/*
#define libmPrintf(x,y,fg,bg,format, ... ) libmPrintfXY(x,y,fg,bg,pprefsPrintBuf,COMMON_BUF_LEN, format, ##__VA_ARGS__)
#define libmPrint(x, y, fg, bg, str) libmPrintXY(x, y, fg, bg, str)
#define fillLine(sy,color) libmFillRect(0, sy, 480, sy + LIBM_CHAR_HEIGHT ,color);
*/

#define fillLine(sy,color) libmFillRect( 0 , sy , 480 , sy + LIBM_CHAR_HEIGHT ,color);

#define libmInitBuffers(opt, sync) libmInitBuffers(opt, sync, &dinfo)
#define libmClearBuffers() libmClearBuffers(&dinfo)
#define libmPrintf(x,y,fg,bg,format, ... ) pprefsLibmenuPrintf(x,y,fg,bg, format, ##__VA_ARGS__)
#define libmPrint(x, y, fg, bg, str) libmPrintXY(x, y, fg, bg, str, &dinfo)
#define libmPrint16(x, y, fg, bg, str) libmPrintXY16(x, y, fg, bg, str, &dinfo)
#define libmFillRect(sx, sy, ex, ey, color) libmFillRect(sx, sy, ex, ey, color, &dinfo)
#define libmFrame(sx, sy, ex, ey, color) libmFrame(sx, sy, ex, ey, color, &dinfo)
#define libmPoint(adr, src) libmPoint(adr, src, &dinfo)
#define libmMakeDrawAddr(x, y) libmMakeDrawAddr(x, y, &dinfo)
#define libmGetColor(addr) libmGetColor(addr, &dinfo)

#define pprefsMakeSelectBoxSpeedy(start_x,start_y,titleLabel, itemName , selectKey, type) pprefsMakeSelectBox_(start_x,start_y,titleLabel, itemName , selectKey, type, true)
#define pprefsMakeSelectBox(start_x,start_y,titleLabel, itemName , selectKey, type) pprefsMakeSelectBox_(start_x,start_y,titleLabel, itemName , selectKey, type, false)


//#define libmPrintf(x,y,fg,bg,format, ... ) libmPrintfXY(x,y,fg,bg,pprefsPrintBuf,COMMON_BUF_LEN, &g_libm_draw_info, format, ##__VA_ARGS__)
//#define libmPrint(x, y, fg, bg, str) libmPrintXY(x, y, fg, bg, str, &g_libm_draw_info)
//#define libmFillRect(sx, sy, ex, ey, color) libmFillRect(sx, sy, ex, ey, color, &g_libm_draw_info)
//#define libmFrame(sx, sy, ex, ey, color) libmFrame(sx, sy, ex, ey, color, &g_libm_draw_info)
//#define libmInitBuffers(opt, sync) libmInitBuffers(opt, sync, &g_libm_draw_info)
//#define libmPoint(adr, src) libmPoint(adr, src, &g_libm_draw_info)
//#define libmMakeDrawAddr(x, y) libmMakeDrawAddr(x, y, &g_libm_draw_info)


#define pprefsMakeSelectBoxSpeedy(start_x,start_y,titleLabel, itemName , selectKey, type) pprefsMakeSelectBox_(start_x,start_y,titleLabel, itemName , selectKey, type, true)
#define pprefsMakeSelectBox(start_x,start_y,titleLabel, itemName , selectKey, type) pprefsMakeSelectBox_(start_x,start_y,titleLabel, itemName , selectKey, type, false)



/*-----------------------------------------------
セレクトボックスを作る
幅と高さは文字数によって自動調整
@prams : start_x
         描画開始位置のx座標
@prams : start_y
         描画開始位置のy座標
@prams : titleLabel
         セレクトボックスの上に表示されるタイトル
@prams : itemName
         選ばせる項目
         char *itemName[] = { "1st", "2nd", "3rd" , NULL };
         のような変数(最後はNULL)のポインタをわたす
@prams : selectKey
         決定ボタンのボタンフラグ
@prams : type
         == 0 ノーマル
         == 1 HOMEボタンで選択を「止める」事が出来るセレクトボックスになる
@prams : isSpeedy
         セレクトボックスの描画を高速化するか?
         
@returns :
            >= 0  選ばれた項目が1番目なら0,2番目なら1,...という値
            <  0  @params : (type が1のときだけ)「止める」が選択されたら

-----------------------------------------------*/
int pprefsMakeSelectBox_( int start_x, int start_y,char *titleLabel, char *itemName[] , u32 selectKey, int type, bool isSpeedy);
void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);
void makeWindowSpeedy(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);
void makeWindowWithGettingButton(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor, SceCtrlData *pad);
void makeWindowQuick(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);
void printScreen(void);
void pprefsLibmenuPrintf(int x, int y, u32 fg, u32 bg, const char *fmt, ...);

#endif
