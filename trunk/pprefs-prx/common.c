#include "common.h"

dir_t dirTmp;
char commonBuf[COMMON_BUF_LEN];
const char *lineFeedCode[] = { "\r\n", "\n", "\r"}; //改行コード
Conf_Key config;
SceCtrlData padData;
char ownPath[256];
char rootPath[16];
int deviceModel = 9;
libm_draw_info dinfo;
libm_vram_info vinfo;
char modelName[6];

//buttonNum buttonDataは,ボタン入れ替えに使う
//buttonNumの数字を入れ替えれば役割も入れ替わる
//
//e.g. 1)
// if( pad.Buttons & buttonData.flag[butttonNum[0]] )  {  }
//e.g. 2)
//printf( "select:%s", buttonData.name[butttonNum[0]] );

struct pprefsButtonDatas buttonData[] = {
	{PSP_CTRL_CROSS,PB_SYM_PSP_CROSS},
	{PSP_CTRL_CIRCLE,PB_SYM_PSP_CIRCLE}
};
int buttonNum[] = {0,1};
ILP_Key conf[PPREFS_CONF_NUM];
int hitobashiraFlag = 0;
char basePathDefault[64];
char basePathCurrent[64];

void pprefsApplyConfig(void)
{
	int i;
	if( config.swapButton ){
		buttonNum[0] = 1;
		buttonNum[1] = 0;
	}else{
		buttonNum[0] = 0;
		buttonNum[1] = 1;
	}

	int len = strlen(config.basePath);
	if( len == 0 ){
		strcpy(basePathCurrent, basePathDefault);
	}else{
		strcpy(basePathCurrent, config.basePath);
		if( basePathCurrent[len-1] != '/' ){
			basePathCurrent[len+0] = '/';
			basePathCurrent[len+1] = '\0';
		}
		
	}

}







