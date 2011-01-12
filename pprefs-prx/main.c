/*
	
	ありがとう、
	maxemさん、plumさん、takkaさん(アルファベット順)

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psploadexec_kernel.h>
#include <pspsysmem_kernel.h>

#include "main.h"
#include "libmenu.h"
#include "memory.h"
#include "file.h"
#include "conf.h"
#include "button.h"
#include "thread.h"

// モジュールの定義
PSP_MODULE_INFO( "PLUPREFS", PSP_MODULE_KERNEL, 0, 0 );


#define compareScePspDateTime(a,b) ( \
a.second == b.second && \
a.minute == b.minute && \
a.hour   == b.hour   && \
a.day    == b.day    && \
a.month  == b.month  && \
a.year   == b.year      \
)


#define INI_PATH "/pprefs.ini"


#define LEN_PER_LINE 256
#define MAX_LINE 21



#define MAX_DISPLAY_NUM 21

#define PRINT_SCREEN() \
libmClearBuffers(); \
libmPrint(10,10,FG_COLOR,BG_COLOR,"pprefs Ver. 1.07   by hiroi01"); \
libmPrint(440,10,FG_COLOR,BG_COLOR,modelName[deviceModel]);

#define SUSPEND_THREADS() \
Suspend_Resume_Threads(SUSPEND_MODE); \
now_state = true;

#define RESUME_THREADS() \
Suspend_Resume_Threads(RESUME_MODE); \
now_state = false;

char *sepluginsTextPath[] = {
	"ms0:/seplugins/vsh.txt",
	"ms0:/seplugins/game.txt",
	"ms0:/seplugins/pops.txt"
};

struct {
	int num;//number of line
	bool exist;//ファイルが存在するかフラグ
	bool edit;//編集フラグ
	SceIoStat stat;
	struct pdataLine{
		char path[LEN_PER_LINE];
		bool toggle;//= ture ON / = false OFF
	}line[MAX_LINE];
} pdata[3];


struct pdataLine tmp_pdataLine;


//ファイルリスト(ファイルブラウザ)のためのbuffer
dir_t dirBuf[128];

int stop_flag;


SceCtrlData padData;

Conf_Key config;

//改行コード
char *lineFeedCode[3] = { "\r\n", "\n", "\r"};

//buttonNum buttonDataは,ボタン入れ替えに使う
//buttonNumの数字を入れ替えれば役割も入れ替わる
//
//e.g. 1)
// if( pad.Buttons & buttonData.flag[butttonNum[0]] )  {  }
//e.g. 2)
//printf( buttonData.name[butttonNum[0]] );
struct {
	unsigned int flag;
	char *name;
}buttonData[] = {
	{PSP_CTRL_CROSS,"×"},
	{PSP_CTRL_CIRCLE,"○"}
};
int buttonNum[2] = {0,1};
/*----------------------------------------------------------------------------*/


#define COMMON_BUF_LEN 256
char commonBuf[COMMON_BUF_LEN];

#define libmPrintf(x,y,fg,bg,format, ... ) libmPrintf(x,y,fg,bg,commonBuf,COMMON_BUF_LEN,format, ##__VA_ARGS__)
#define fillLine(sy,color) libmFillRect( 0 , sy , 480 , sy + LIBM_CHAR_HEIGHT ,color);

/*----------------------------------------------------------------------------*/



int now_type = 0;
char ownPath[256];
bool now_state = false; // = true suspending   = false no suspending

int deviceModel = 5;
char *modelName[] = {
	"[01g]",
	"[02g]",
	"[03g]",
	"[04g]",
	"[g o]",
	"[???]"
};

/*----------------------------------------------------------------------------*/
int module_start( SceSize arglen, void *argp );
int module_stop( void );

int main_thread( SceSize arglen, void *argp );
void main_menu(void);

int removeAnItem(int type,int num);
int addNewItem(int type,struct pdataLine *lineData);


int readSepluginsText( int ptype ,bool checkFlag );
int writeSepluginsText(int ptype);

void makeWindowWithButtonGetting(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);

int copyMeProcess(void);

void saveEdit(void);

void pprefsSleep(double sleepTime);
double gettimeofday_sec();

/*----------------------------------------------------------------------------*/






int main_thread( SceSize arglen, void *argp )
{
	double timesec;
	char *temp;

	while( 1 )
	{
		if(
			sceKernelFindModuleByName("sceSystemMemoryManager") && 
			sceKernelFindModuleByName("sceIOFileManager") && 
			sceKernelFindModuleByName("sceGE_Manager") && 
			sceKernelFindModuleByName("sceDisplay_Service") && 
			sceKernelFindModuleByName("sceController_Service") && 
			sceKernelFindModuleByName("sceKernelLibrary")
		)
		{
			break;
		}
		
		sceKernelDelayThread(1000);
	}
	


	strcpy(ownPath, argp);
	Read_Conf(argp,&config);
	
	if( config.swapButton ){
		buttonNum[0] = 1;
		buttonNum[1] = 0;
	}
	
	pdata[0].num = 0;
	pdata[1].num = 0;
	pdata[2].num = 0;
	pdata[0].edit = false;
	pdata[1].edit = false;
	pdata[2].edit = false;
	pdata[0].exist = false;
	pdata[1].exist = false;
	pdata[2].exist = false;
	
	readSepluginsText(3,false);

	//deviceModel 多分
	//0 -> 1000
	//1 -> 2000
	//2 -> 3000
	//3 -> 3000
	//4 -> go
	//5 -> unknown
	deviceModel = sceKernelGetModel();
	if( !( deviceModel >= 0 &&  deviceModel <= 4 ) ) deviceModel = 5;
	
	
	padData.Buttons = 0;
	
	//BOOT MESSAGE
	if( config.bootMessage ){
		strcpy(commonBuf," pprefs 起動準備完了! / 起動ボタン:");
		GET_KEY_NAME(config.bootKey, commonBuf);
		temp = strrchr(commonBuf, '+');
		if( temp != NULL ){
			temp[-1] = ' ';
			temp[0]  = '\0';
		}
		timesec = gettimeofday_sec();
		while( stop_flag ){
			//表示
			if( libmInitBuffers(false,PSP_DISPLAY_SETBUF_NEXTFRAME) ){
				libmPrint(0,264,SetAlpha(WHITE,0xFF),SetAlpha(BLACK,0xFF),commonBuf);
				sceDisplayWaitVblankStart();
			}

			sceCtrlPeekBufferPositive( &padData, 1 );
			if( ( padData.Buttons & CHEACK_KEY_2 ) != 0 ) break;
			if( (gettimeofday_sec() - timesec) >= 8 ) break;
			
			sceKernelDelayThread( 10000 );
		}
	}
	
	
	while( stop_flag ){
		if((padData.Buttons & config.bootKey) == config.bootKey){
			main_menu();
		}
		//    padData.Buttons ^= XOR_KEY;
		sceCtrlPeekBufferPositive( &padData, 1 );
		sceKernelDelayThread( 50000 );
	}

  return 0;
}

u32 detect_key(void){
	
	double timesec;
	char *temp;
	u32 beforeKey = 0;
	
	wait_button_up(&padData);
	makeWindow(
		24 , 28 ,
		480 - LIBM_CHAR_WIDTH*3 , 28 + LIBM_CHAR_HEIGHT*5,
		FG_COLOR,BG_COLOR
	);
	libmPrint( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT    , FG_COLOR, BG_COLOR, "キー検出 残り 秒" );
	while(1){
		get_button(&padData);
		if( padData.Buttons != 0 ) break;
	}

	timesec = gettimeofday_sec();
	while(1){
		get_button(&padData);
		if( beforeKey != padData.Buttons ){
			libmFillRect( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT*2+2, 480 - LIBM_CHAR_WIDTH*3, 28 + LIBM_CHAR_HEIGHT*3+2, BG_COLOR );
			commonBuf[0] = '\0';
			GET_KEY_NAME(padData.Buttons, commonBuf);
			temp = strrchr(commonBuf, '+');
			if( temp != NULL ) temp[-1] = '\0';
			libmPrint   ( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT*2+2, FG_COLOR, BG_COLOR, commonBuf);
			beforeKey = padData.Buttons;
		}
		libmPrintf  ( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT    , FG_COLOR, BG_COLOR, "キー検出 残り%d秒", (int)(3 - (gettimeofday_sec() - timesec)) );
		if( (gettimeofday_sec() - timesec) >= 3 ) break;
	}
	
	return padData.Buttons;
}

int config_menu(void){
	Conf_Key newConfig = config;
	char *temp;
	int now_arrow = 0,menuNum = 8;
	char *lineFeedCodeName[] = {
		"CR+LF",
		"LF",
//		"LF"
	};
	
	while(1){
		PRINT_SCREEN();
		
		libmPrint(15,28,BG_COLOR,FG_COLOR," 設定 ");
		commonBuf[0] = '\0';
		GET_KEY_NAME(newConfig.bootKey, commonBuf);
		temp = strrchr(commonBuf, '+');
		if( temp != NULL ) temp[-1] = '\0';
		libmPrint (15 + LIBM_CHAR_WIDTH* 5, 46    , FG_COLOR, BG_COLOR,commonBuf);
		libmPrint (15, 46                         , FG_COLOR, BG_COLOR, "起動キー:");
		libmPrint (15, 46 + 1*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "pprefsを起動させるボタンの指定(デフォルトはHOME)");

		libmPrintf(15, 46 + 2*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "本体起動時メッセージ:%s",newConfig.bootMessage?"O N":"OFF");
		libmPrint (15, 46 + 3*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "本体を起動したときに左下に表\示される「pprefs起動準備完了!～」を表\示するか?(デフォルトはON)");
	
		libmPrintf(15, 46 + 4*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "×/○の役割入れ替え:%s", newConfig.swapButton?"O N":"OFF");
		libmPrint (15, 46 + 5*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "×/○ボタンの役割を入れ替える ON→○決定/×キャンセル OFF→×決定/○キャンセル(デフォルトはOFF)");
		
		libmPrintf(15, 46 + 6*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "一度押しでVSH再起動:%s", newConfig.onePushRestart?"O N":"OFF");
		libmPrint (15, 46 + 7*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "VSH再起動するのにSTARTを一度押すか二度押すか ON→一度押し OFF→二度押し(デフォルトはOFF)");

		libmPrintf(15, 46 + 8*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "改行コード:%s", lineFeedCodeName[newConfig.lineFeedCode]);
		libmPrint (15, 46 + 9*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "このプラグインがテキストを書き出すときに使う改行コード(デフォルトはCR+LF)");

		libmPrint (15, 46 +10*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "上記の設定で保存する");

		libmPrint (15, 46 +12*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "デフォルト値にする");

		libmPrint (15, 46 +14*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "やめる");

		libmPrintf(5, 46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, ">");

		libmPrintf(5,264,FG_COLOR,BG_COLOR," %s:選択 HOME:やめる",buttonData[buttonNum[0]].name);

		wait_button_up(&padData);
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_DOWN ){
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				if( now_arrow + 1 < menuNum ) now_arrow++;
				else now_arrow = 0;
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				wait_button_up(&padData);
			}else if( padData.Buttons & PSP_CTRL_UP ){
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				if( now_arrow - 1 >= 0 ) now_arrow--;
				else now_arrow = menuNum - 1;
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				wait_button_up(&padData);
			}else if( padData.Buttons & buttonData[buttonNum[0]].flag ){
				if( now_arrow == 0 ){
					newConfig.bootKey = detect_key();
					if( newConfig.bootKey == 0 ) newConfig.bootKey = PSP_CTRL_HOME;
					break;
				}else if( now_arrow == 1 ){
					newConfig.bootMessage = ! newConfig.bootMessage;
					break;
				}else if( now_arrow == 2 ){
					newConfig.swapButton = ! newConfig.swapButton;
					break;
				}else if( now_arrow == 3 ){
					newConfig.onePushRestart = ! newConfig.onePushRestart;
					break;
				}else if( now_arrow == 4 ){
					newConfig.lineFeedCode++;
					if( newConfig.lineFeedCode > 1 ) newConfig.lineFeedCode = 0;
					break;
				}else if( now_arrow == (menuNum -3) ){
					if( ! now_state ) SUSPEND_THREADS();
					config = newConfig;
					if( config.swapButton ){
						buttonNum[0] = 1;
						buttonNum[1] = 0;
					}else{
						buttonNum[0] = 0;
						buttonNum[1] = 1;
					}
					Write_Conf(ownPath,&newConfig);
					wait_button_up(&padData);
					return 0;
				}else if( now_arrow == (menuNum -2) ){
					Set_Default_Conf(&newConfig);
					break;
				}else if( now_arrow == (menuNum -1) ){
					wait_button_up(&padData);
					return 1;
				}
			}else if( padData.Buttons & PSP_CTRL_HOME ){
					wait_button_up(&padData);
					return 1;
			}
		}

	}
	
}

int file_selecter(void){
	int dir_num,offset,i,now_arrow;
	char currentPath[256] = "ms0:/seplugins/";

	
	while(1){

		
		dir_num = read_dir(dirBuf,currentPath, 0);
		offset = 0;
		now_arrow = 0;

		PRINT_SCREEN();
		libmPrintf(15,28,BG_COLOR,FG_COLOR," <<追記>> : 追記したいプラグインを選択してください ");
		libmPrintf(15,36,BG_COLOR,FG_COLOR," [%s] [%d] ",currentPath,dir_num);
		libmPrintf(5,264,FG_COLOR,BG_COLOR," %s:選択 HOME:やめる L:フォルダ階層上へ R:フォルダ開く ",buttonData[buttonNum[0]].name);

PRINT_LIST:
		libmFillRect( 0 , 46 , 480 , 46 + MAX_DISPLAY_NUM*(LIBM_CHAR_HEIGHT+2),BG_COLOR );
		if( dir_num != 0 ){
			for( i = 0; i < dir_num && i < MAX_DISPLAY_NUM; i++ ){
				if( dirBuf[i+offset].type == TYPE_DIR )
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s/",dirBuf[i+offset].name);
				else
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dirBuf[i+offset].name);
			}
			libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		}
		
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_DOWN ){
				if( now_arrow + 1 < MAX_DISPLAY_NUM && now_arrow + 1 < dir_num ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow++;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
					wait_button_up(&padData);
				}else{
					if( offset+MAX_DISPLAY_NUM < dir_num ) offset++;
					wait_button_up(&padData);
					goto PRINT_LIST;
				}
			}else if( padData.Buttons & PSP_CTRL_UP ){
				
				if( now_arrow - 1 >= 0 ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow--;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
					wait_button_up(&padData);
				}else{
					if( offset > 0 ) offset--;
					wait_button_up(&padData);
					goto PRINT_LIST;
				}
			}else if( padData.Buttons & (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER) ){
				//からのフォルダーではない
				if(  dir_num != 0 ){
					//選択されたものがフォルダー
					if( dirBuf[offset+now_arrow].type == TYPE_DIR ){
						strcat(currentPath,dirBuf[offset+now_arrow].name);
						strcat(currentPath,"/");
						wait_button_up(&padData);
						break;
					//選択されたものがフォルダーではない && buttonData[buttonNum[0]].flagボタンが押されている
					}else if( (padData.Buttons & buttonData[buttonNum[0]].flag) && dir_num != 0 ){
						strcpy( tmp_pdataLine.path , currentPath );
						strcat( tmp_pdataLine.path , dirBuf[offset+now_arrow].name );
						tmp_pdataLine.toggle = false;
						addNewItem(now_type,&tmp_pdataLine);
						pdata[now_type].edit = true;
						wait_button_up(&padData);
						return 1;
					}
				}
				wait_button_up(&padData);
			}else if( padData.Buttons & PSP_CTRL_LTRIGGER ){
				wait_button_up(&padData);
				if( up_dir(currentPath) >= 0 ){
					break;
				}
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				wait_button_up(&padData);
				return 0;
			}
			wait_button_up(&padData);
		}
	}
}


/*


return: どのメニューが実行されたか
(0 < : エラー)
 0 = : 何も実行されなかった
 1 = : メニューNo.1が実行された
 2 = : メニューNo.2が実行された
・
・
・
*/


int sub_menu(int currentSelected,int position){
	int i,now_arrow;
	int menunum = (deviceModel == 4)?3:4;
	char *menu[] ={
		"追記",
		"削除",
		"設定",
		"COPY ME"
	};

	now_arrow = 0;

	makeWindow( 8 , position , 8 + LIBM_CHAR_WIDTH*12 , position + LIBM_CHAR_HEIGHT+2 + (LIBM_CHAR_HEIGHT+2) * menunum + 2 ,FG_COLOR,BG_COLOR);

	//最下の行の表示を消して書き直す
	libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);
	libmPrintf(5,264,FG_COLOR,BG_COLOR," %s:選択 %s:戻る ",buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);


	libmPrint( 8 + LIBM_CHAR_WIDTH  , position - 2 ,FG_COLOR,BG_COLOR,"メニュー");
	libmPrintf( 8 + LIBM_CHAR_WIDTH , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) , FG_COLOR , BG_COLOR , ">");
	for( i = 0; i < menunum; i++ ){
		libmPrint( 8 + LIBM_CHAR_WIDTH*2 , position + LIBM_CHAR_HEIGHT+2 + i*(LIBM_CHAR_HEIGHT+2) ,FG_COLOR,BG_COLOR,menu[i]);
	}
	while(1){

		get_button(&padData);
		if( padData.Buttons & PSP_CTRL_DOWN ){
			libmFillRect( 8 + LIBM_CHAR_WIDTH , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) , 8 + LIBM_CHAR_WIDTH*2 , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) + LIBM_CHAR_HEIGHT , BG_COLOR );
			if( now_arrow + 1 < menunum ){
				now_arrow++;
			}else{
				now_arrow = 0;
			}
			libmPrint( 8 + LIBM_CHAR_WIDTH , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) , FG_COLOR , BG_COLOR , ">");
			wait_button_up(&padData);
		}else if( padData.Buttons & PSP_CTRL_UP ){
			libmFillRect( 8 + LIBM_CHAR_WIDTH , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) , 8 + LIBM_CHAR_WIDTH*2 , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) + LIBM_CHAR_HEIGHT , BG_COLOR );
			if( now_arrow - 1 >= 0 ){
				now_arrow--;
			}else{
				now_arrow = menunum - 1;
			}
			libmPrint( 8 + LIBM_CHAR_WIDTH , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) , FG_COLOR , BG_COLOR , ">");
			wait_button_up(&padData);
		}else if( padData.Buttons & buttonData[buttonNum[0]].flag ){
			wait_button_up(&padData);
			break;
		}else if( padData.Buttons & (PSP_CTRL_HOME|buttonData[buttonNum[1]].flag) ){
			wait_button_up(&padData);
			return 0;
		}
		wait_button_up(&padData);
	}

	//追記
	if( now_arrow == 0 ){
		file_selecter();
	//削除
	}else if( now_arrow == 1 ){
		removeAnItem(now_type,currentSelected);
		pdata[now_type].edit = true;
	//設定
	}else if( now_arrow == 2 ){
		config_menu();
	//COPY ME
	}else if( now_arrow == 3 ){
		int tmp = copyMeProcess();
		if( tmp < 0 ){
			makeWindow(
				24 , 28 ,
				 24 + LIBM_CHAR_WIDTH*23 , 28 + LIBM_CHAR_HEIGHT*5,
				 FG_COLOR,BG_COLOR
			);
			libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"エラーが発生しました");
			if( tmp == -1 )
				libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"pprefsが入っているMSを入れてください");
			else
				libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"ErrorNo.:%d",tmp);
			libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:OK ",buttonData[buttonNum[0]].name);
			wait_button_down(&padData,buttonData[buttonNum[0]].flag);
		}
		if( tmp !=  1 ) readSepluginsText(3,true);
	}
	
	return now_arrow + 1;
	
}


#define swap_pdataLine(first,second) \
        strcpy( tmp_pdataLine.path , first.path ); \
        tmp_pdataLine.toggle = first.toggle; \
        strcpy( first.path  , second.path ); \
        first.toggle = second.toggle; \
        strcpy( second.path  , tmp_pdataLine.path ); \
        second.toggle = tmp_pdataLine.toggle;



#define printEditedMark() libmPrint(63 , 28 , BG_COLOR , FG_COLOR,"*")



void main_menu(void)
{
	// wait till releasing buttons
	wait_button_up(&padData);
	double timesec = gettimeofday_sec();
	// suspend XMB
//	Suspend_Resume_Threads(SUSPEND_MODE);
	
	//prepare for displaying and display
	libmInitBuffers(false,PSP_DISPLAY_SETBUF_NEXTFRAME);
	PRINT_SCREEN();

	int i,tmp;
	int now_arrow = 0;//current position of arrow
	



	
	readSepluginsText(3,true);
	
	wait_button_up(&padData);

	while(1){
		PRINT_SCREEN();
		if( ! now_state ) libmPrint(424,10,FG_COLOR,BG_COLOR,"NS");
		else libmPrint(424,10,FG_COLOR,BG_COLOR,"  ");
		
		libmPrintf(0,264,FG_COLOR,BG_COLOR,"%s選択 △メニュー □+↑/↓並び替え SELECT編集破棄&リロード HOME保存&終了 START保存&VSH再起動",buttonData[buttonNum[0]].name);

		libmPrintf(15,28,BG_COLOR,FG_COLOR,"<<[L]  %s [R]>>",sepluginsTextPath[now_type]);
		if( pdata[now_type].edit ) libmPrint(63 , 28 , BG_COLOR , FG_COLOR,"*");
		libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		for( i = 0; i < pdata[now_type].num; i++ ){
			libmPrintf(15,38 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,
			"[%s] %s",pdata[now_type].line[i].toggle?"O N":"OFF",pdata[now_type].line[i].path);
		}

		wait_button_up_ex(&padData,PSP_CTRL_SQUARE);
		while(1){
			//フリーズしないようにするため、0.5秒のwaitをもってからsuspend
			if( ! now_state ){
				if( (gettimeofday_sec() - timesec) >= 0.5 ){
					SUSPEND_THREADS();
					libmPrint(424,10,FG_COLOR,BG_COLOR,"  ");
				}
			}
			
			get_button(&padData);

			if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP) && pdata[now_type].num > 0 ){
				tmp = now_arrow; //現在の矢印の位置を覚えておく
				
				//矢印の位置を変更 / change position of arrow
				if( padData.Buttons & PSP_CTRL_DOWN ){
					if(  now_arrow + 1 < pdata[now_type].num )
						now_arrow++;
					else
						now_arrow = 0;
				}else if( padData.Buttons & PSP_CTRL_UP ){
					if( now_arrow - 1 >= 0 )
						now_arrow--;
					else
						now_arrow = pdata[now_type].num - 1;
				}

				//□が押されてるなら、並び替え(そして編集フラグ立てる) / if □ is pushed , sort ( and flag edit )
				if( padData.Buttons & PSP_CTRL_SQUARE ){
					printEditedMark();
					pdata[now_type].edit = true;
					//tmp - now_arrow ==  1 one up
					//tmp - now_arrow == -1 one down
					//tmp - now_arrow >   1 up top      ( tmp > now_arrow )
					//tmp - now_arrow <  -1 down bottom ( tmp < now_arrow )
					if( tmp - now_arrow > 1 ){
						for( i = tmp; i > now_arrow; i-- ){
							swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i-1]);
						}
						break;
					}else if( tmp - now_arrow < -1 ){
						for( i = tmp; i < now_arrow; i++ ){
							swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i+1]);
						}
						break;
					}else{
						swap_pdataLine(pdata[now_type].line[now_arrow],pdata[now_type].line[tmp]);
					}
				}
				
				//画面に表示 / display on screen
				fillLine(38 + tmp*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
				libmPrintf(15,38 + tmp*(LIBM_CHAR_HEIGHT+2) , FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[tmp].toggle?"O N":"OFF",pdata[now_type].line[tmp].path);
				fillLine(38 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
				libmPrintf(15,38 + now_arrow*(LIBM_CHAR_HEIGHT+2) , FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].path);
				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");

				//□以外のボタンが離されるまでwait / wait till releasing buttons except □
				wait_button_up_ex(&padData,PSP_CTRL_SQUARE);
				
			}else if( padData.Buttons & buttonData[buttonNum[0]].flag && pdata[now_type].num > 0 ){
				pdata[now_type].edit = true;
				 printEditedMark();
				pdata[now_type].line[now_arrow].toggle = !pdata[now_type].line[now_arrow].toggle;
				libmPrintf(
				            15,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,
				            "[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].path
				);
				wait_button_up(&padData);
			}else if( padData.Buttons & PSP_CTRL_RTRIGGER ){
				if( now_type == 0 ) now_type = 1;
				else if( now_type == 1 ) now_type = 2;
				else if( now_type == 2 ) now_type = 0;
				wait_button_up(&padData);
				now_arrow = 0;
				break;
			}else if( padData.Buttons & PSP_CTRL_LTRIGGER ){
				if( now_type == 0 ) now_type = 2;
				else if( now_type == 1 ) now_type = 0;
				else if( now_type == 2 ) now_type = 1;
				wait_button_up(&padData);
				now_arrow = 0;
				break;
			}else if( padData.Buttons &  PSP_CTRL_TRIANGLE  ){
				if( sub_menu(now_arrow,( now_arrow < 10 )?148:46) != 0 ){
					
					now_arrow = 0;
				}
				wait_button_up(&padData);
				break;
			}else if( padData.Buttons &  PSP_CTRL_START ){
				wait_button_up(&padData);
				
				if( ! config.onePushRestart ){
					makeWindowWithButtonGetting(
						100 , 36 ,
						 100 + LIBM_CHAR_WIDTH*16 , 44 + LIBM_CHAR_HEIGHT*5,
						 FG_COLOR,BG_COLOR
					);
					libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,"もう一度STARTを押すと");
					libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"RESTART VSH");
					while(1){
						if( padData.Buttons & PSP_CTRL_START ){
							libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,"RESTARTING...");
							libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"           ");
							saveEdit();
							if( now_state ) RESUME_THREADS();
							sceKernelExitVSHVSH(NULL);
							return;
						}else if( padData.Buttons & (CHEACK_KEY & ~PSP_CTRL_START) ){
							break;
						}
						get_button(&padData);
					}
				}else{
					saveEdit();
					if( now_state ) RESUME_THREADS();
					sceKernelExitVSHVSH(NULL);
					return;
				}
				
				wait_button_up(&padData);
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				saveEdit();
				wait_button_up(&padData);
				if( now_state ) RESUME_THREADS();
				return;
			}else if( padData.Buttons & PSP_CTRL_SELECT && now_state ){

				if( pdata[0].edit || pdata[1].edit || pdata[2].edit ){
					libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);
					makeWindow(8 , 28 , 8 + LIBM_CHAR_WIDTH*18 , 28 + LIBM_CHAR_HEIGHT*5,FG_COLOR,BG_COLOR);
					libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"編集を破棄して、");
					libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"リロードしてもよろしいですか?");
					libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:はい %s:いいえ",buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);
					while(1){
						get_button(&padData);
						if( padData.Buttons & buttonData[buttonNum[0]].flag ){
							wait_button_up(&padData);
							readSepluginsText(3,false);
							break;
						}else if( padData.Buttons & (buttonData[buttonNum[1]].flag | PSP_CTRL_HOME) ){
							wait_button_up(&padData);
							break;
						}
					}
				}else{
					readSepluginsText(3,false);
				}
				
				break;
			}
		}
	}

}


int module_start( SceSize arglen, void *argp )
{
	
	Get_FirstThreads();
	
	SceUID thid;
	
	
	//umd dumpとは逆で flag == 0 の時にストップする仕様
	stop_flag = 1;
	thid = sceKernelCreateThread( "PPREFS", main_thread, 30, 0x6000, PSP_THREAD_ATTR_NO_FILLSTACK, 0 );
	if( thid ) sceKernelStartThread( thid, arglen, argp );

  return 0;
}

int module_stop( void )
{
	stop_flag = 0;
	  return 0;
}



/*
	remove an item from pdata

	@param : type
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	@param : num
	number of removing data

	@return : 
	= 0 no problem
	< 0 on error
*/

int removeAnItem(int type,int num){
	if( !(0 <= type &&  type <= 2) ) return -1;

	if( num == pdata[type].num -1  ){//last item
		if( pdata[type].num > 0) pdata[type].num--;
	}else{
		for( ; num + 1 < pdata[type].num; num++ ){
			strcpy(pdata[type].line[num].path,pdata[type].line[num+1].path);
			pdata[type].line[num].toggle = pdata[type].line[num+1].toggle;
		}
		if( pdata[type].num > 0) pdata[type].num--;
	}
	
	return 0;
}

/*
	add a new item to pdata

	@param : type
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	@param : str
	adding text(plugin's path)

	@return : 
	= 0 no problem
	< 0 on error
*/
int addNewItem(int type,struct pdataLine *lineData)
{
	if( !(0 <= type &&  type <= 2) ) return -1;
	if( !(pdata[type].num < MAX_LINE) ) return -2;

	strcpy(pdata[type].line[pdata[type].num].path,lineData->path);
	pdata[type].line[pdata[type].num].toggle = lineData->toggle;
	pdata[type].num++;
	
	return 0;
	

}


/*
	@param : ptype 
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	
	@return : 
	= 0 no problem
	= 1,2,3 error about vsh.txt(=1) or game.txt(=2) or pops.txt(=3)
	< 0 on error

*/

int writeSepluginsText(int ptype){
	if( !(0 <= ptype &&  ptype <= 2) ){
		return -1;
	}

	int i,type = ptype;
	SceUID fp;

	
	checkMs();

	fp = sceIoOpen(sepluginsTextPath[type], PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if( fp < 0 ) return (type+1);

	for( i = 0; i < pdata[type].num; i++ ){
		sprintf(commonBuf,"%s %c%s",
				pdata[type].line[i].path,
				pdata[type].line[i].toggle?'1':'0',
				lineFeedCode[config.lineFeedCode]
		);
		sceIoWrite(fp,commonBuf,strlen(commonBuf));
	}
	sceIoClose(fp);
	
	//各種情報更新
	sceIoGetstat(sepluginsTextPath[type], &pdata[type].stat);
	pdata[type].edit = false;
	pdata[type].exist = true;

	return 0;
}




/*
	@param : ptype 
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	= 3 all
	@param : checkFlag
	= true ファイルを読む必要があるかチェックして、必要なら読み込む
	= true 必要かどうか関係なく、ファイルを読み込む
	@return : 
	 = 0 no problem
	!= 0 on error
*/

int readSepluginsText( int ptype ,bool checkFlag )
{

	SceUID fp;
	int type,i,readSize,loopend,ret = 0;
	char line[LEN_PER_LINE],*ptr;
	SceIoStat stat;


	if( 0 <= ptype &&  ptype <= 2 ){
		type = ptype;
		loopend = ptype + 1;
	}else if( ptype == 3 ){
		type = 0;
		loopend = 3;
	}else{
		 return -1;
	}
	
	checkMs();
	
	for( ; type < loopend; type++){
		pdata[type].edit = false;

		if( sceIoGetstat(sepluginsTextPath[type], &stat) < 0 ){//ファイルが存在しない
			pdata[type].exist = false;
			pdata[type].num = 0;
			continue;
		}
		
		//checkFlagが立っていて、これ以前にファイルが存在していて、sizeとmtimeが一緒なら -> ファイルを読む必要なし
		if( checkFlag &&
		    pdata[type].exist == true &&
		    stat.st_size == pdata[type].stat.st_size &&
		    compareScePspDateTime(stat.st_mtime, pdata[type].stat.st_mtime)
		){
			continue;
		}

		
		fp = sceIoOpen(sepluginsTextPath[type], PSP_O_RDONLY, 0777);
		if( fp < 0 ){//ファイルオープンエラー
			pdata[type].exist = false;
			pdata[type].num = 0;
			ret |= (1<<type);
			continue;
		}


		i = 0;
		while( i < MAX_LINE ){
			readSize = read_line_file(fp,line,LEN_PER_LINE - 1);
			if( readSize < 0 ){
				break;
			}
			if( line[0] == '\0' || line[0] == ' ' ){
				continue;
			}else if( (ptr = strchr(line,' ')) == NULL){// ' 'が見つからないなら /  if ' ' is not found
				line[readSize] = '\0';
				strcpy(pdata[type].line[i].path,line);
				pdata[type].line[i].toggle = false;
			}else{
				*ptr = '\0';
				strcpy(pdata[type].line[i].path,line);
				if( (ptr - line + 1) < LEN_PER_LINE ){
					if( *(ptr+1) == '1' )
						pdata[type].line[i].toggle = true;
					else
						pdata[type].line[i].toggle = false;
				}
			}
			i++;
		}
		sceIoClose(fp);
		
		//各種情報を更新
		pdata[type].num = i;
		pdata[type].exist = true;
		pdata[type].stat = stat;
	}
	return ret;
}


void pprefsSleep(double sleepTime)
{
	double timesec= gettimeofday_sec();
	
	while( (gettimeofday_sec() - timesec) < sleepTime );
}
//にゅーってでる窓を作る
void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor)
{
	int nowx = sx,nowy = sy;
	while(1){
		nowx += 8;
		nowy += 8;
		if( nowx > ex ) nowx = ex;
		if( nowy > ey ) nowy = ey;
		libmFillRect(sx , sy , nowx , nowy , bgcolor );
		libmFrame(sx , sy , nowx ,nowy , fgcolor );
		if( nowx == ex && nowy == ey ) break;
		pprefsSleep(0.008);
	}

}

//にゅーってしている最中にもボタンをgetする
void makeWindowWithButtonGetting(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor)
{
	int nowx = sx,nowy = sy;
	while(1){
		get_button(&padData);
		nowx += 8;
		nowy += 8;
		if( nowx > ex ) nowx = ex;
		if( nowy > ey ) nowy = ey;
		libmFillRect(sx , sy , nowx , nowy , bgcolor );
		libmFrame(sx , sy , nowx ,nowy , fgcolor );
		if( nowx == ex && nowy == ey ) break;
		pprefsSleep(0.008);
	}

}





int copyMeProcess(void){
	libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);

	if( pdata[0].edit || pdata[1].edit || pdata[2].edit ){
		makeWindow(
			24 , 28 ,
			24 + LIBM_CHAR_WIDTH*32 , 28 + LIBM_CHAR_HEIGHT*6,
			FG_COLOR,BG_COLOR
		);
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,"\"COPY ME\"を実行する前に現在の編集を保存してください");
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3, FG_COLOR,BG_COLOR,"START:保存して続ける SELECT:編集破棄し続ける");
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*4+ 2 ,FG_COLOR,BG_COLOR,"HOME:\"COPY ME\"をやめる");
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_START ){
				saveEdit();
				break;
			}else if( padData.Buttons & PSP_CTRL_SELECT ){
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				return 1;
			}
		}
	}

	
	
	
	SceIoStat stat;
	char *buf;
	int readSize,writeSize,fd;

	checkMs();

	sceIoGetstat(ownPath, &stat);

	buf = memoryAlloc(stat.st_size);
	if( buf == NULL ) return -1;

	if( (fd = sceIoOpen(ownPath,PSP_O_RDONLY,0777)) < 0 ){
		memoryFree(buf);
		return -2;
	}
	
	readSize = sceIoRead(fd,buf,stat.st_size);
	sceIoClose(fd);
	if( readSize  != stat.st_size ){
		memoryFree(buf);
		return -3;
	}
	


	while(1){
		makeWindow(
			24 , 28 ,
			 24 + LIBM_CHAR_WIDTH*23 , 28 + LIBM_CHAR_HEIGHT*5,
			 FG_COLOR,BG_COLOR
		);
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"2枚目のメモリースティックを");
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"入れてください");
		libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:入れた HOME:コピーやめる",buttonData[buttonNum[0]].name);
		while(1){
			get_button(&padData);
			if( padData.Buttons & buttonData[buttonNum[0]].flag ){
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				return 2;
			}
		}
		wait_button_up(&padData);
		if( !(checkMs() < 0) ) break;
	}
	
	sceIoMkdir("ms0:/seplugins",0777);
	

	if( (fd = sceIoOpen("ms0:/seplugins/pprefs.prx",PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC,0777)) < 0 ){
		memoryFree(buf);
		return -4;
	}
	
	writeSize = sceIoWrite(fd,buf,readSize);
	sceIoClose(fd);
/*	if( readSize  != writeSize ){
		memoryFree(buf);
		return -5;
	}
*/	

	makeWindow(
		24 , 28 ,
		 24 + LIBM_CHAR_WIDTH*18 , 28 + LIBM_CHAR_HEIGHT*5,
		 FG_COLOR,BG_COLOR
	);

	if( writeSize == readSize ){
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"コピーが完了しました");
	}else{
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"コピーに失敗した可能\性があります");
	}

	libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:OK ",buttonData[buttonNum[0]].name);
	while(1){
		get_button(&padData);
		if( padData.Buttons & buttonData[buttonNum[0]].flag ) break;
	}
	wait_button_up(&padData);
	memoryFree(buf);
	return 0;
}



void saveEdit(void)
{
	int i;

//	checkMs();

	for( i = 0; i < 3; i++ ){
		if ( pdata[i].edit ){
			if( ! now_state ) SUSPEND_THREADS();
			while(1){
				if( writeSepluginsText(i) < 0 ){

					makeWindow(
						24 , 28 ,
						24 + LIBM_CHAR_WIDTH*26 , 28 + LIBM_CHAR_HEIGHT*5,
						FG_COLOR,BG_COLOR
					);
					libmPrintf( 24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"%s",sepluginsTextPath[i]);
					libmPrint(  24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"の書き込みに失敗しました");
					libmPrintf(  24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:リトライ %s:スキップ ",buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);

					while(1){
						get_button(&padData);
						if( padData.Buttons & buttonData[buttonNum[0]].flag ){
							wait_button_up(&padData);
							continue;
						}else if( padData.Buttons & buttonData[buttonNum[1]].flag ){
							wait_button_up(&padData);
							break;
						}
					}
				}else{
					break;
				}
			}
		}
	}
}





double gettimeofday_sec()
{
	struct timeval tv;
	//gettimeofday(&tv, NULL);
	sceKernelLibcGettimeofday(&tv, NULL);
	return tv.tv_sec + (double)tv.tv_usec*1e-6;
}


