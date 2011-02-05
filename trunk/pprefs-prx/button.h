

/*
    umd dumpのコードがベース
*/


#ifndef __BUTTON__
#define __BUTTON__

#include <stdbool.h>
#include <pspctrl.h>
#include <pspkernel.h>

// マクロの定義
#define CHEACK_KEY (PSP_CTRL_SELECT | PSP_CTRL_START | PSP_CTRL_UP | PSP_CTRL_RIGHT | \
    PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_TRIANGLE | \
    PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_SQUARE | PSP_CTRL_NOTE | PSP_CTRL_HOME)

#define CHEACK_KEY_2 (PSP_CTRL_SELECT | PSP_CTRL_START | PSP_CTRL_UP | PSP_CTRL_RIGHT | \
    PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_TRIANGLE | \
    PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_SQUARE | PSP_CTRL_NOTE | PSP_CTRL_HOME | \
	PSP_CTRL_SCREEN | PSP_CTRL_VOLUP | PSP_CTRL_VOLDOWN )

#define XOR_KEY (PSP_CTRL_DISC)



#define GET_KEY_NAME(key,str) \
if( key& PSP_CTRL_SELECT ) strcat(str,"SELECT + "); \
if( key& PSP_CTRL_START ) strcat(str,"START + "); \
if( key& PSP_CTRL_UP ) strcat(str,PB_SYM_PSP_UP" + "); \
if( key& PSP_CTRL_RIGHT ) strcat(str,PB_SYM_PSP_RIGHT" + "); \
if( key& PSP_CTRL_DOWN ) strcat(str,PB_SYM_PSP_DOWN" + "); \
if( key& PSP_CTRL_LEFT ) strcat(str,PB_SYM_PSP_LEFT" + "); \
if( key& PSP_CTRL_LTRIGGER ) strcat(str,"[L] + "); \
if( key& PSP_CTRL_RTRIGGER ) strcat(str,"[R] + "); \
if( key& PSP_CTRL_TRIANGLE ) strcat(str,PB_SYM_PSP_TRIANGLE" + "); \
if( key& PSP_CTRL_CIRCLE ) strcat(str,PB_SYM_PSP_CIRCLE" + "); \
if( key& PSP_CTRL_CROSS ) strcat(str,PB_SYM_PSP_CROSS" + "); \
if( key& PSP_CTRL_SQUARE ) strcat(str,PB_SYM_PSP_SQUARE" + "); \
if( key& PSP_CTRL_NOTE ) strcat(str,PB_SYM_PSP_NOTE" + "); \
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


void waitButtonUp(void);
void waitButtonDown(unsigned int key);
void waitAnyButtonDown(unsigned int key);
void waitButtonUpEx(unsigned int exception_key);
bool isButtonDown(unsigned int key);


//CHEACK_KEY_2で定義されたキーだけget
void get_button(SceCtrlData *data);


//第一引数にbuffer(SceCtrlData)をわたす必要あり
void wait_button_up_multithread(SceCtrlData *data);
void wait_button_up(SceCtrlData *data);
void wait_button_down(SceCtrlData *data,unsigned int key);
void wait_any_button_down(SceCtrlData *data,unsigned int key);
void wait_button_up_ex(SceCtrlData *data,unsigned int exception_key);
bool is_button_down(SceCtrlData *data,unsigned int key);

//made by plum
//thank you plum
int Count_Buttons(u32 buttons, int count);

#endif
