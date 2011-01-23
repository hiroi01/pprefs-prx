
#ifndef __PPREFS__
#define __PPREFS__



#define PRINT_SCREEN() \
libmClearBuffers(); \
libmPrint(10,10,FG_COLOR,BG_COLOR,"pprefs Ver. 1.08   by hiroi01"); \
libmPrint(440,10,FG_COLOR,BG_COLOR,modelName[deviceModel]);

#define libmPrintf(x,y,fg,bg,format, ... ) libmPrintf(x,y,fg,bg,commonBuf,COMMON_BUF_LEN,format, ##__VA_ARGS__)
#define fillLine(sy,color) libmFillRect( 0 , sy , 480 , sy + LIBM_CHAR_HEIGHT ,color);





#endif
