/*
	
	ありがとう、
	maxemさん、plumさん、takkaさん(アルファベット順)

*/



#include "common.h"
#include "libmenu.h"
#include "main.h"
#include "memory.h"
#include "file.h"
#include "button.h"
#include "thread.h"
#include "sepluginstxt.h"
#include "configmenu.h"
#include "pprefs.h"
#include "fileselecter.h"
#include "editpergame.h"
#include "language.h"

// モジュールの定義
PSP_MODULE_INFO( "PLUPREFS", PSP_MODULE_KERNEL, 0, 0 );






/*------------------------------------------------------
 COMMON
------------------------------------------------------*/

char commonBuf[COMMON_BUF_LEN];
char *sepluginsTextPath[] = {
	"ms0:/seplugins/vsh.txt",
	"ms0:/seplugins/game.txt",
	"ms0:/seplugins/pops.txt"
};

Conf_Key config;

//改行コード
const char *lineFeedCode[] = { "\r\n", "\n", "\r"};

SceCtrlData padData;


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
char ownPath[256];

int deviceModel = 5;
char *modelName[6] = {
	"[01g]",
	"[02g]",
	"[03g]",
	"[04g]",
	"[g o]",
	"[???]"
};

int now_type = 0;

bool now_state = false; // = true suspending   = false no suspending




/*------------------------------------------------------*/


#define MAX_DISPLAY_NUM 21

extern char *pergameBuf;







struct pdataLine tmp_pdataLine;


int stop_flag;








/*----------------------------------------------------------------------------*/
int module_start( SceSize arglen, void *argp );
int module_stop( void );

int main_thread( SceSize arglen, void *argp );
void main_menu(void);




int copyMeProcess(void);

void saveEdit(void);



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
	//2 -> 3000 03g?
	//3 -> 3000 04g?
	//4 -> go
	//5 -> unknown
	deviceModel = sceKernelGetModel();
	if( deviceModel != 0 && deviceModel != 1 && deviceModel != 2 && deviceModel != 3 && deviceModel != 4 ) deviceModel = 5;
	
	
	padData.Buttons = 0;
	
	//BOOT MESSAGE
	if( config.bootMessage ){
		strcpy(commonBuf,PPREFSMSG_BOOTMESSAGE);
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
//	int menunum = (deviceModel == 4)?4:5;
	int menunum = 5;
	char *menu_fat[] = { PPREFSMSG_SUBMENU_LIST };
	char *menu_go[] = { PPREFSMSG_SUBMENU_LIST_GO };
	char **menu = (deviceModel == 4)?menu_go:menu_fat;

	now_arrow = 0;

	makeWindow( 8 , position , 8 + LIBM_CHAR_WIDTH*12 , position + LIBM_CHAR_HEIGHT+2 + (LIBM_CHAR_HEIGHT+2) * menunum + 2 ,FG_COLOR,BG_COLOR);

	//最下の行の表示を消して書き直す
	libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);
	libmPrintf(5,264, SILVER ,BG_COLOR,PPREFSMSG_SUBMENU_HOTOUSE,buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);


	libmPrint( 8 + LIBM_CHAR_WIDTH  , position - 2 ,FG_COLOR,BG_COLOR,PPREFSMSG_SUBMENU_TITLE);
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
		if( fileSelecter("ms0:/seplugins/", &dirTmp, PPREFSMSG_ADD_TOP, 0, "ccbcccac") == 0 ){
			strcpy(tmp_pdataLine.path, dirTmp.name);
			tmp_pdataLine.toggle = false;
			addNewItem(now_type,&tmp_pdataLine);
			pdata[now_type].edit = true;
		}
	//削除
	}else if( now_arrow == 1 ){
		removeAnItem(now_type,currentSelected);
		pdata[now_type].edit = true;
	//設定
	}else if( now_arrow == 2 ){
		editPergameMenu();
	}else if( now_arrow == 3 ){
		config_menu();
	//COPY ME or eh0
	}else if( now_arrow == 4 ){
		if(deviceModel == 4){
			fileSelecter("eh0:/", &dirTmp, PPREFSMSG_SUBMENU_EH0, 0, NULL);
		}else{
			int tmp = copyMeProcess();
			if( tmp < 0 ){
				makeWindow(
						   24 , 28 ,
						   24 + LIBM_CHAR_WIDTH*23 , 28 + LIBM_CHAR_HEIGHT*5,
						   FG_COLOR,BG_COLOR
						   );
				libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_ERROR);
				if( tmp == -1 )
					libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_INSERTERROR);
				else
					libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"ErrorNo.:%d",tmp);
				libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:OK ",buttonData[buttonNum[0]].name);
				wait_button_down(&padData,buttonData[buttonNum[0]].flag);
			}
			if( tmp !=  1 ) readSepluginsText(3,true);
		}
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

	int i,tmp,headOffset = 0;
	int now_arrow = 0;//current position of arrow
	clock_t time = 0;
	u32 beforeButtons = 0;


	
	readSepluginsText(3,true);
	
	wait_button_up(&padData);

	while(1){
		PRINT_SCREEN();
		if( ! now_state ) libmPrint(424,10,FG_COLOR,BG_COLOR,"NS");
		else libmPrint(424,10,FG_COLOR,BG_COLOR,"  ");
		
		libmPrintf(0,264,SILVER ,BG_COLOR,PPREFSMSG_MAINMENU_HOTOUSE,buttonData[buttonNum[0]].name);

		libmPrintf(15,28,BG_COLOR,FG_COLOR,"<<[L]  %s [R]>>",sepluginsTextPath[now_type]);
		if( pdata[now_type].edit ) libmPrint(63 , 28 , BG_COLOR , FG_COLOR,"*");
//		libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		for( i = 0; i < MAX_DISPLAY_NUM && i < pdata[now_type].num; i++ ){
			libmPrintf(15,38 + i*(LIBM_CHAR_HEIGHT+2),(now_arrow == i+headOffset)?RED:FG_COLOR,BG_COLOR,
			"[%s] %s",pdata[now_type].line[i+headOffset].toggle?"O N":"OFF",pdata[now_type].line[i+headOffset].path);
		}

		if( beforeButtons == 0 ) wait_button_up(&padData);
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
				if( beforeButtons & (PSP_CTRL_DOWN|PSP_CTRL_UP) ){
					if( (sceKernelLibcClock() - time) >= (2 * 100 * 1000) ){
						time = sceKernelLibcClock();
					}else{
						continue;
					}
				}else{
					beforeButtons = (PSP_CTRL_DOWN|PSP_CTRL_UP);
					time = sceKernelLibcClock();
				}
				
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
				
				//one up
				if( tmp - now_arrow == 1 && headOffset == tmp ){
					headOffset--;
					break;
				//one down
				}else if( tmp - now_arrow == -1 && headOffset+MAX_DISPLAY_NUM-1 == tmp ){
					headOffset++;
					break;					
				}else if( pdata[now_type].num  > MAX_DISPLAY_NUM ){
					//up top
					if( tmp - now_arrow > 1 ){
						headOffset = 0;
					//donw bottom
					}else if( tmp - now_arrow <  -1 ){
						headOffset = pdata[now_type].num - MAX_DISPLAY_NUM;
					}
					break;
				}
				
				//画面に表示 / display on screen
				fillLine(38 + (tmp-headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
				libmPrintf(15,38 + (tmp-headOffset)*(LIBM_CHAR_HEIGHT+2) , FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[tmp].toggle?"O N":"OFF",pdata[now_type].line[tmp].path);
				fillLine(38 + (now_arrow-headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
				libmPrintf(15,38 + (now_arrow-headOffset)*(LIBM_CHAR_HEIGHT+2) , RED,BG_COLOR,"[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].path);
//				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");

				//□以外のボタンが離されるまでwait / wait till releasing buttons except □
//				wait_button_up_ex(&padData,PSP_CTRL_SQUARE);
				
			}else if( padData.Buttons & buttonData[buttonNum[0]].flag && pdata[now_type].num > 0 ){
				if( beforeButtons & buttonData[buttonNum[0]].flag ) continue;
				beforeButtons = buttonData[buttonNum[0]].flag;
				
				pdata[now_type].edit = true;
				 printEditedMark();
				pdata[now_type].line[now_arrow].toggle = !pdata[now_type].line[now_arrow].toggle;
				libmPrintf(
				            15,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,
				            "[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].path
				);
//				wait_button_up(&padData);
			}else if( padData.Buttons & PSP_CTRL_RTRIGGER ){
				if( beforeButtons & PSP_CTRL_RTRIGGER ) continue;
				beforeButtons = PSP_CTRL_RTRIGGER;

				if( now_type == 0 ) now_type = 1;
				else if( now_type == 1 ) now_type = 2;
				else if( now_type == 2 ) now_type = 0;
//				wait_button_up(&padData);
				now_arrow = 0;
				break;
			}else if( padData.Buttons & PSP_CTRL_LTRIGGER ){
				if( beforeButtons & PSP_CTRL_LTRIGGER ) continue;
				beforeButtons = PSP_CTRL_LTRIGGER;

				if( now_type == 0 ) now_type = 2;
				else if( now_type == 1 ) now_type = 0;
				else if( now_type == 2 ) now_type = 1;
//				wait_button_up(&padData);
				now_arrow = 0;
				break;
			}else if( padData.Buttons &  PSP_CTRL_TRIANGLE ){
				if( beforeButtons & PSP_CTRL_TRIANGLE ) continue;
				beforeButtons = PSP_CTRL_TRIANGLE;

				SUSPEND_THREADS();
				if( sub_menu(now_arrow,( now_arrow < 10 )?148:46) != 0 ){
					
					now_arrow = 0;
				}
//				wait_button_up(&padData);
				break;
			}else if( padData.Buttons &  PSP_CTRL_START ){
				if( beforeButtons & PSP_CTRL_START ) continue;
				beforeButtons = PSP_CTRL_START;

				wait_button_up(&padData);
				
				if( ! config.onePushRestart ){
					makeWindowWithButtonGetting(
						100 , 36 ,
						 100 + LIBM_CHAR_WIDTH*16 , 44 + LIBM_CHAR_HEIGHT*5,
						 FG_COLOR,BG_COLOR
					);
					libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,PPREFSMSG_MAINMENU_REPUSHSTART);
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
					RESUME_THREADS();
					sceKernelExitVSHVSH(NULL);
					return;
				}
				
				wait_button_up(&padData);
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				if( beforeButtons & PSP_CTRL_HOME ) continue;
				beforeButtons = PSP_CTRL_HOME;
				
				saveEdit();
				wait_button_up(&padData);
				if( now_state ) RESUME_THREADS();
				return;
			}else if( padData.Buttons & PSP_CTRL_SELECT ){
				if( beforeButtons & PSP_CTRL_SELECT ) continue;
				beforeButtons = PSP_CTRL_SELECT;

				SUSPEND_THREADS();

				if( pdata[0].edit || pdata[1].edit || pdata[2].edit ){
					
					libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);
					makeWindow(8 , 28 , 8 + LIBM_CHAR_WIDTH*18 , 28 + LIBM_CHAR_HEIGHT*5,FG_COLOR,BG_COLOR);
					libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,PPREFSMSG_MAINMENU_RELOAD);
					libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,PPREFSMSG_YESORNO,buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);
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
			}else{
				beforeButtons = 0;
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
	if( pergameBuf != NULL ) free(pergameBuf);
	return 0;
}







int copyMeProcess(void){
	libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);

	if( pdata[0].edit || pdata[1].edit || pdata[2].edit ){
		makeWindow(
			24 , 28 ,
			24 + LIBM_CHAR_WIDTH*32 , 28 + LIBM_CHAR_HEIGHT*6,
			FG_COLOR,BG_COLOR
		);
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_FIRSTCONFIRM);
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3, FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_FIRSTCONFIRMHOTOUSE);
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

	buf = malloc(stat.st_size);
	if( buf == NULL ) return -1;

	if( (fd = sceIoOpen(ownPath,PSP_O_RDONLY,0777)) < 0 ){
		free(buf);
		return -2;
	}
	
	readSize = sceIoRead(fd,buf,stat.st_size);
	sceIoClose(fd);
	if( readSize  != stat.st_size ){
		free(buf);
		return -3;
	}
	


	while(1){
		makeWindow(
			24 , 28 ,
			 24 + LIBM_CHAR_WIDTH*23 , 28 + LIBM_CHAR_HEIGHT*5,
			 FG_COLOR,BG_COLOR
		);
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_INSERT2NDMS);
		libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_INSERT2NDMSHOWTOUSE,buttonData[buttonNum[0]].name);
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
		free(buf);
		return -4;
	}
	
	writeSize = sceIoWrite(fd,buf,readSize);
	sceIoClose(fd);
/*	if( readSize  != writeSize ){
		free(buf);
		return -5;
	}
*/	

	makeWindow(
		24 , 28 ,
		 24 + LIBM_CHAR_WIDTH*18 , 28 + LIBM_CHAR_HEIGHT*5,
		 FG_COLOR,BG_COLOR
	);

	if( writeSize == readSize ){
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_DONETOCOPY);
	}else{
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,PPREFSMSG_COPYME_FAILTOCOPY);
	}

	libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:OK ",buttonData[buttonNum[0]].name);
	while(1){
		get_button(&padData);
		if( padData.Buttons & buttonData[buttonNum[0]].flag ) break;
	}
	wait_button_up(&padData);
	free(buf);
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
					makeWindow(24, 28,24 + LIBM_CHAR_WIDTH*26, 28 + LIBM_CHAR_HEIGHT*5, FG_COLOR, BG_COLOR);
					libmPrintf(24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT      , FG_COLOR,BG_COLOR,PPREFSMSG_FAILTOWRITE,sepluginsTextPath[i]);
					libmPrintf(24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT*3 + 4, FG_COLOR,BG_COLOR,PPREFSMSG_FAILTOWRITEHOWTOUSE,buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);

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



