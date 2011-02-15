#include "common.h"

dir_t dirTmp;
char commonBuf[COMMON_BUF_LEN];
const char *lineFeedCode[] = { "\r\n", "\n", "\r"}; //改行コード
Conf_Key config;
SceCtrlData padData;
char ownPath[256];
char rootPath[16];
int deviceModel = 9;
char *modelName[] = {
	"[01g]", //0
	"[02g]", //1
	"[03g]", //2
	"[04g]", //3
	"[g o]", //4
	"[06g]", //5
	"[07g]", //6
	"[08g]", //7
	"[09g]", //8
	"[???]", //9
};

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
INI_Key conf[PPREFS_CONF_NUM];
int hitobashiraFlag = 0;





