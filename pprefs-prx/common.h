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


#define MAKE_CALL(a, f) _sw(0x0C000000 | (((u32)(f) >> 2)  & 0x03ffffff), a);
#define MAKE_JUMP(a, f) _sw(0x08000000 | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define REDIRECT_FUNCTION(a, f) { u32 address = a; _sw(0x08000000 | (((u32)(f) >> 2)  & 0x03ffffff), address);  _sw(0, address+4); }


#define compareScePspDateTime(a,b) ( \
a.second == b.second && \
a.minute == b.minute && \
a.hour   == b.hour   && \
a.day    == b.day    && \
a.month  == b.month  && \
a.year   == b.year      \
)





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



/*-----------------------------------------------------------*/


#define COMMON_BUF_LEN 256

struct pprefsButtonDatas{
	unsigned int flag;
	char *name;
};


#ifdef PPREFS_LITE

#define INI_NAME "/pprefs_lite.ini"
#define PPREFS_CONF_NUM 11


#else

#define INI_NAME "/pprefs.ini"
#define PPREFS_CONF_NUM 16


#endif


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
