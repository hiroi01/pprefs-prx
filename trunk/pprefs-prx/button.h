
#ifndef __BUTTON__
#define __BUTTON__

// É}ÉNÉçÇÃíËã`
#define CHEACK_KEY (PSP_CTRL_SELECT | PSP_CTRL_START | PSP_CTRL_UP | PSP_CTRL_RIGHT | \
    PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_TRIANGLE | \
    PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_SQUARE | PSP_CTRL_NOTE | PSP_CTRL_HOME)

#define XOR_KEY (PSP_CTRL_DISC)

void get_button(SceCtrlData *data);
void wait_button_up(SceCtrlData *data);
void wait_button_down(SceCtrlData *data,unsigned int key);

#endif
