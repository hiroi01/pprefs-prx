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

#define SET_CONFIG() \
{ \
	int i; \
	if( config.swapButton ){ \
		buttonNum[0] = 1; \
		buttonNum[1] = 0; \
	}else{ \
		buttonNum[0] = 0; \
		buttonNum[1] = 1; \
	} \
	i = strlen(config.basePathOri) - 1; \
	if( config.basePathOri[i] != '/' ){ \
		config.basePathOri[i+1] = '/'; \
		config.basePathOri[i+2] = '\0'; \
	} \
	strcpy(config.basePath,config.basePathOri); \
}


#define INI_NAME "/pprefs.ini"

//extern char sepluginsTextPath[3][64];
extern const char *sepluginsBasePath[];
extern char commonBuf[COMMON_BUF_LEN];
extern const char *lineFeedCode[];
extern Conf_Key config;
extern SceCtrlData padData;
extern char ownPath[256];
extern int deviceModel;
extern char *modelName[];
extern struct pprefsButtonDatas buttonData[];
extern int buttonNum[];
extern bool now_state;
extern INI_Key conf[10];


extern bool hitobashiraFlag;

#endif
