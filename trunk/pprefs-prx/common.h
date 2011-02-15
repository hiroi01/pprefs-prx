//ごちゃごちゃしてるからいつか直そう



#ifndef PPREFSCOMMON_H
#define PPREFSCOMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pspkernel.h>
#include <pspmscm.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psploadexec_kernel.h>
#include <pspsysmem_kernel.h>

#include "conf.h"
#include "memory.h"
#include "button.h"
#include "thread.h"
#include "file.h"
#include "language.h"
#include "nidresolve.h"


// Thanks to Davee
#define PSP_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)


#define compareScePspDateTime(a,b) ( \
a.second == b.second && \
a.minute == b.minute && \
a.hour   == b.hour   && \
a.day    == b.day    && \
a.month  == b.month  && \
a.year   == b.year      \
)




#define PPREFS_CONF_NUM 14

/*------------------------------------------
for ini
------------------------------------------*/


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
	if( config.basePathOri[0] == '\0' ){ \
		strcpy(config.basePath,config.basePathDefault); \
	}else{ \
		if( config.basePathOri[i] != '/' ){ \
			config.basePathOri[i+1] = '/'; \
			config.basePathOri[i+2] = '\0'; \
		} \
		strcpy(config.basePath,config.basePathOri); \
	} \
}


#define INI_NAME "/pprefs.ini"

/*-----------------------------------------------------------*/


#define COMMON_BUF_LEN 256
struct pprefsButtonDatas{
	unsigned int flag;
	char *name;
};

extern dir_t dirTmp;
extern char commonBuf[COMMON_BUF_LEN];
extern const char *lineFeedCode[];
extern Conf_Key config;
extern SceCtrlData padData;
extern char ownPath[256];
extern char rootPath[16];
extern int deviceModel;
extern char *modelName[];
extern struct pprefsButtonDatas buttonData[];
extern int buttonNum[];
extern INI_Key conf[PPREFS_CONF_NUM];
extern int hitobashiraFlag;

#endif
