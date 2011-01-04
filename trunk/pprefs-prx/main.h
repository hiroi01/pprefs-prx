
#ifndef __PPREFS__
#define __PPREFS__

#include "color.h"
#define FG_COLOR WHITE
#define BG_COLOR BLACK


#define GET_KEY_NAME(key,str) \
if( key& PSP_CTRL_SELECT ) strcat(str,"SELECT + "); \
if( key& PSP_CTRL_START ) strcat(str,"START + "); \
if( key& PSP_CTRL_UP ) strcat(str,"Å™ + "); \
if( key& PSP_CTRL_RIGHT ) strcat(str,"Å® + "); \
if( key& PSP_CTRL_DOWN ) strcat(str,"Å´ + "); \
if( key& PSP_CTRL_LEFT ) strcat(str,"Å© + "); \
if( key& PSP_CTRL_LTRIGGER ) strcat(str,"[L] + "); \
if( key& PSP_CTRL_RTRIGGER ) strcat(str,"[R] + "); \
if( key& PSP_CTRL_TRIANGLE ) strcat(str,"Å¢ + "); \
if( key& PSP_CTRL_CIRCLE ) strcat(str,"Åõ + "); \
if( key& PSP_CTRL_CROSS ) strcat(str,"Å~ + "); \
if( key& PSP_CTRL_SQUARE ) strcat(str,"Å† + "); \
if( key& PSP_CTRL_NOTE ) strcat(str,"ÅÙ + "); \
if( key& PSP_CTRL_HOME ) strcat(str,"HOME(PS) + "); \
if( key& PSP_CTRL_HOLD ) strcat(str,"HOLD + "); \
if( key& PSP_CTRL_SCREEN ) strcat(str,"SCREEN + "); \
if( key& PSP_CTRL_VOLUP ) strcat(str,"[+] + "); \
if( key& PSP_CTRL_VOLDOWN ) strcat(str,"[-] + ");

#define GET_KEY_NAME_FOR_CONF(key,str) \
if( key & PSP_CTRL_SELECT ) strcat(str,"SELECT + "); \
if( key & PSP_CTRL_START ) strcat(str,"START + "); \
if( key & PSP_CTRL_UP ) strcat(str,"UP + "); \
if( key & PSP_CTRL_RIGHT ) strcat(str,"RIGHT + "); \
if( key & PSP_CTRL_DOWN ) strcat(str,"DOWN + "); \
if( key & PSP_CTRL_LEFT ) strcat(str,"LEFT + "); \
if( key & PSP_CTRL_LTRIGGER ) strcat(str,"L_TRI + "); \
if( key & PSP_CTRL_RTRIGGER ) strcat(str,"R_TRI + "); \
if( key & PSP_CTRL_TRIANGLE ) strcat(str,"TRIANGLE + "); \
if( key & PSP_CTRL_CIRCLE ) strcat(str,"CIRCLE + "); \
if( key & PSP_CTRL_CROSS ) strcat(str,"CROSS + "); \
if( key & PSP_CTRL_SQUARE ) strcat(str,"SQUARE + "); \
if( key & PSP_CTRL_NOTE ) strcat(str,"NOTE + "); \
if( key & PSP_CTRL_HOME ) strcat(str,"HOME + "); \
if( key & PSP_CTRL_HOLD ) strcat(str,"HOLD + "); \
if( key & PSP_CTRL_SCREEN ) strcat(str,"SCREEN + "); \
if( key & PSP_CTRL_VOLUP ) strcat(str,"VOLUP + "); \
if( key & PSP_CTRL_VOLDOWN ) strcat(str,"VOLDOWN + ");

void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);
int read_line_file(SceUID fp, char* line, int num);//from umd dumper
int removeSpace(char *str);

int checkMs(void);

int read_line_file_keepn(SceUID fp, char* line, int num);



#endif
