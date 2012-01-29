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

#include <cmlibmenu.h>
#include <libinip.h>

#include "memory.h"
#include "button.h"
#include "threadctrl.h"
#include "file.h"
#include "language.h"
#include "nidresolve.h"

#include "debuglog.h"

#ifdef PPREFS_LITE

#define PPREFS_TITLE_STRING "pprefs lite Ver. 1.032beta     by hiroi01"

#else

#define PPREFS_TITLE_STRING "pprefs Ver. 1.132beta     by hiroi01"

#endif

// Thanks to Davee
#define PSP_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)

#define NOP 0x00000000

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
------------------------------------------*/


//used in sortgame.c
enum
{
	SORT_TYPE_NORMAL_LIST = 1,	//
	SORT_TYPE_NOT_USED_ = 2,	//not used
	
	SORT_TYPE_CATEGORIZES_LIGHT_620 = 4,//
	SORT_TYPE_CATEGORIZES = 8,//
	SORT_TYPE_CATEGORIZES_LIGHT_63X = 16,	//
	SORT_TYPE_CATEGORIZES_LIGHT = (SORT_TYPE_CATEGORIZES_LIGHT_620 | SORT_TYPE_CATEGORIZES_LIGHT_63X),
	
	SORT_TYPE_ISOCSO = 32,	//
	SORT_TYPE_GAME = 64,	//
	SORT_TYPE_GAME150 = 128, //
	SORT_TYPE_GAME5XX = 256, //
	
	SORT_TYPE_NOTREMOVE_ISOCACHE = 512, //if this flag is true, not remove isocache
	SORT_TYPE_NOTDISPLAY_ICON0 = 1024, //if this flag is true, not displayed ICON0
};


#ifdef PPREFS_LITE

#define INI_NAME "/pprefs_lite.ini"
#define PPREFS_CONF_NUM 11

#else

#define INI_NAME "/pprefs.ini"
#define PPREFS_CONF_NUM 12

#endif

typedef struct
{
	u32 bootKey;//0
	bool swapButton;//1
	bool bootMessage;//2
	bool onePushRestart;//3
	int lineFeedCode;//4 // = 0:CR+LF  =1:CR  =2:LF
	char basePath[64];//5
	u32 color0;//6
	u32 color1;//7
	u32 color2;//8
	u32 color3;//9
	u32 color4;//10
	u32 color5;//11
	u32 disablePluginsKey;//12
	
#ifndef PPREFS_LITE
	u32 sortType;//13
#endif

} Conf_Key;

extern char basePathDefault[64];
extern char basePathCurrent[64];

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
extern char modelName[6];//"[01g]" "[02g]" "[03g]" "[04g]" "[g o]" .....
extern struct pprefsButtonDatas buttonData[];
extern int buttonNum[];
extern ILP_Key conf[PPREFS_CONF_NUM];
extern int hitobashiraFlag;
extern libm_draw_info dinfo;
extern libm_vram_info vinfo;

/*-----------------------------------------------------------*/

void pprefsApplyConfig(void);


#endif


