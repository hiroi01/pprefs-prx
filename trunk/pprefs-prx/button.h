
#ifndef __BUTTON__
#define __BUTTON__

#include <stdbool.h>
#include <pspctrl.h>

// É}ÉNÉçÇÃíËã`
#define CHEACK_KEY (PSP_CTRL_SELECT | PSP_CTRL_START | PSP_CTRL_UP | PSP_CTRL_RIGHT | \
    PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_TRIANGLE | \
    PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_SQUARE | PSP_CTRL_NOTE | PSP_CTRL_HOME)

#define CHEACK_KEY_2 (PSP_CTRL_SELECT | PSP_CTRL_START | PSP_CTRL_UP | PSP_CTRL_RIGHT | \
    PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_TRIANGLE | \
    PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_SQUARE | PSP_CTRL_NOTE | PSP_CTRL_HOME | \
    PSP_CTRL_HOLD | PSP_CTRL_SCREEN | PSP_CTRL_VOLUP | PSP_CTRL_VOLDOWN )

#define XOR_KEY (PSP_CTRL_DISC)


void waitButtonUp(void);
void waitButtonDown(unsigned int key);
void waitAnyButtonDown(unsigned int key);
void waitButtonUpEx(unsigned int exception_key);
bool isButtonDown(unsigned int key);

void get_button(SceCtrlData *data);

void wait_button_up(SceCtrlData *data);
void wait_button_down(SceCtrlData *data,unsigned int key);
void wait_any_button_down(SceCtrlData *data,unsigned int key);
void wait_button_up_ex(SceCtrlData *data,unsigned int exception_key);
bool is_button_down(SceCtrlData *data,unsigned int key);

#endif
