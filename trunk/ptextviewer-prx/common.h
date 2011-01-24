//ごちゃごちゃしてるからいつか直そう



#ifndef PPREFSCOMMON_H
#define PPREFSCOMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psploadexec_kernel.h>
#include <pspsysmem_kernel.h>

#include "conf.h"
#include "thread.h"

#define compareScePspDateTime(a,b) ( \
a.second == b.second && \
a.minute == b.minute && \
a.hour   == b.hour   && \
a.day    == b.day    && \
a.month  == b.month  && \
a.year   == b.year      \
)

#define COMMON_BUF_LEN 256


struct pprefsButtonDatas{
	unsigned int flag;
	char *name;
};



#define SUSPEND_THREADS() \
if( ! now_state ){ \
	Suspend_Resume_Threads(SUSPEND_MODE); \
	now_state = true; \
}

#define RESUME_THREADS() \
if( now_state ){ \
	Suspend_Resume_Threads(RESUME_MODE); \
	now_state = false; \
}


extern char commonBuf[COMMON_BUF_LEN];
extern const char *lineFeedCode[];
extern Conf_Key config;
extern SceCtrlData padData;
extern char ownPath[256];
extern struct pprefsButtonDatas buttonData[];
extern int buttonNum[];
extern bool now_state;

#endif
