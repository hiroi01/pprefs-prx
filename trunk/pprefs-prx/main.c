/*
	
	ありがとう、
	masciiさん、maxemさん、neur0nさん、plumさん、STEARさん、takkaさん、yreeenさん(アルファベット順)、他全ての開発者の方々

*/

#include <pspsysevent.h>

#include "sepluginstxt.h"
#include "configmenu.h"
#include "pprefsmenu.h"
#include "fileselecter.h"
#include "editpergame.h"
#include "usb.h"
#include "sortgame.h"
#include "backupmenu.h"
#include "common.h"


// モジュールの定義
PSP_MODULE_INFO( "PPREFS", PSP_MODULE_KERNEL, 0, 0 );


/*------------------------------------------------------*/

static int now_type = 0;//This means ... 0 : vsh.txt / 1 : game.txt / 2:pops.txt
static struct pdataLine tmp_pdataLine;
static int stop_flag;
static int disable_suspend = 0;

/*------------------------------------------------------*/


#define MAX_DISPLAY_NUM 21




#define is5xx(x) ( ( (x) >= PSP_FIRMWARE(500) ) &&  ( (x) <  PSP_FIRMWARE(600) ) )



/*----------------------------------------------------------------------------*/
int module_start( SceSize arglen, void *argp );
int module_stop( void );

int main_thread( SceSize arglen, void *argp );
void main_menu(void);
int confirm_save(void);




int copyMeProcess(void);

void saveEdit(void);



/*----------------------------------------------------------------------------*/



#ifndef PPREFS_LITE

SceUID (* pspIoOpen)(char *file, int flags, SceMode mode) = NULL;
int (* pspIoRead)(SceUID fd, void *data, u32 len) = NULL;
int (* pspIoWrite)(SceUID fd, void *data, u32 len) = NULL;
int (* pspIoClose)(SceUID fd) = NULL;



int pauseGameTest()
{
	if( pspIoOpen == NULL ) pspIoOpen = (void *)FindProc("sceIOFileManager", "IoFileMgrForKernel", 0x109F50BC);	
	if( pspIoRead == NULL ) pspIoRead = (void *)FindProc("sceIOFileManager", "IoFileMgrForKernel", 0x6A638D83);
	if( pspIoWrite == NULL ) pspIoWrite = (void *)FindProc("sceIOFileManager", "IoFileMgrForKernel", 0x42EC03AC);
	if( pspIoClose == NULL ) pspIoClose = (void *)FindProc("sceIOFileManager", "IoFileMgrForKernel", 0x810C4BC3);

	/*
	char *selectName[] = {
		"READ",
		"WRITE",
		NULL
	};
	 */
	
	char *list[] = {
		"READ",
		"WRITE",
		"REMOVE",
		NULL
	};
	

	int selectNum = pprefsMakeSelectBox(10, 10,PPREFSMSG_PAUSEGAMETEST ,list, buttonData[buttonNum[0]].flag, 1 );

	
	
	
	int readSize;


	u8 _header[512+64];
	
	/* use a pointer for math ease */
	u8 *header = _header; 
	
	/* align to 64 */
	header = (u8 *)((u32)header & ~0x3F); header = (u8 *)((u32)header + 0x40);
	
	/* now clear it */
	memset(header, 0, 512);

	SceUID fd;
	

	if( selectNum == 0 ){
		fd = pspIoOpen("eflash0a:__hibernation", PSP_O_RDONLY, 0);
		/* check for error */
		if (fd < 0)
		{
			/* return error */
			return fd;
		}
		
		readSize = pspIoRead(fd,header,512);
		pspIoClose(fd);
		
		fd = sceIoOpen("ms0:/__hibernation",PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC,0777);
		if( fd < 0 )
		{
			return fd;
		}
		sceIoWrite(fd,header,readSize);
		sceIoClose(fd);
	}else if( selectNum == 1 ){
		
		fd = sceIoOpen("ms0:/__hibernation",PSP_O_RDONLY,0777);
		if( fd < 0 )
		{
			return fd;
		}
		readSize = sceIoRead(fd,header,512);
		sceIoClose(fd);


		fd = pspIoOpen("eflash0a:__hibernation",0x04000003/* PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC*/, 0);
		/* check for error */
		if (fd < 0)
		{
			/* return error */
			return fd;
		}
		
		pspIoWrite(fd,header,readSize);
		pspIoClose(fd);
		
	}else if( selectNum == 2){
		/* open hibernation fs */
		fd = pspIoOpen("eflash0a:__hibernation", 0x04000003, 0);
		
		/* check for error */
		if (fd < 0)
		{
			/* return error */
			return fd;
		}
		
		/* write the blank header */
		int written = pspIoWrite(fd, header, 512);
		
		/* check for error */
		if (written < 0)
		{
			/* close file */
			pspIoClose(fd);
			
			/* return the error code */
			return written;
		}
		
		/* return result */
		return pspIoClose(fd);
	}

	
	
	return 0;

}


#endif


#define PSP_SYSEVENT_SUSPEND_QUERY 0x100 

int Callback_Suspend(int ev_id, char *ev_name, void *param, int *result)
{
	// 返り値に 0 ではなく -1 を返すと、スリープを無効化できる
	int ret = 0;
	
	if (ev_id == PSP_SYSEVENT_SUSPEND_QUERY && disable_suspend ) ret = -1;

	return ret;
}



const char *INI_Key_lineFeedCode_list[] = {
	"CR+LF",
	"LF",
	NULL
};






int main_thread( SceSize arglen, void *argp )
{

	int usbState = 0;
	char *temp;
	char iniPath[256];
	int i;
	//wait
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
	

	
	
	//起動時にLが押されていたら
	sceCtrlPeekBufferPositive( &padData, 1 );
	if( padData.Buttons & PSP_CTRL_LTRIGGER ) usbState = 2;

	
	//read INI and set config
	strcpy(iniPath, argp);
	temp = strrchr(iniPath, '/');
	if( temp != NULL ) *temp = '\0';
	strcat(iniPath,INI_NAME);
	
	strcpy(config.basePathDefault,rootPath);
	
	//6.35PRO or PRO-A以降かの判別(?)
	if( sceKernelDevkitVersion() == PSP_FIRMWARE(0x635) && sctrlHENGetVersion() == 0x1001 && sceKernelFindModuleByName("VshCtrl") == NULL ){
		//it may be 6.35PRO この判定は正しいのか分からない
		strcat( config.basePathDefault,"plugins/" );
	}else{
		strcat( config.basePathDefault, "seplugins/" );
	}


	
	INI_Init_Key(conf);
	
	
#ifdef PPREFS_LITE
	INI_Add_Button(conf, "BootKey", &config.bootKey, PSP_CTRL_HOME );//0
	INI_Add_Bool(conf, "SwapButton", &config.swapButton, false );//1
	INI_Add_Bool(conf, "OnePushRestart", &config.onePushRestart, false );//2
	INI_Add_List(conf, "LineFeedCode", &config.lineFeedCode, 0, INI_Key_lineFeedCode_list);//3
	INI_Add_String(conf, "BasePath", config.basePathOri, config.basePathDefault);//4
	INI_Add_Hex(conf, "Color0", &config.color0, FG_COLOR_DEFAULT, NULL);//5
	INI_Add_Hex(conf, "Color1", &config.color1, BG_COLOR_DEFAULT, NULL);//6
	INI_Add_Hex(conf, "Color2", &config.color2, SL_COLOR_DEFAULT, NULL);//7
	INI_Add_Hex(conf, "Color3", &config.color3, EX_COLOR_DEFAULT, NULL);//8
	INI_Add_Hex(conf, "Color4", &config.color4, ON_COLOR_DEFAULT, NULL);//9
	INI_Add_Hex(conf, "Color5", &config.color5, OF_COLOR_DEFAULT, NULL);//10
#else
	INI_Add_Button(conf, "BootKey", &config.bootKey, PSP_CTRL_HOME );//0
	INI_Add_Bool(conf, "BootMessage", &config.bootMessage, true );//1
	INI_Add_Bool(conf, "SwapButton", &config.swapButton, false );//2
	INI_Add_Bool(conf, "OnePushRestart", &config.onePushRestart, false );//3
	INI_Add_List(conf, "LineFeedCode", &config.lineFeedCode, 0, INI_Key_lineFeedCode_list);//4
	INI_Add_String(conf, "BasePath", config.basePathOri, config.basePathDefault);//5
	INI_Add_Hex(conf, "Color0", &config.color0, FG_COLOR_DEFAULT, NULL);//6
	INI_Add_Hex(conf, "Color1", &config.color1, BG_COLOR_DEFAULT, NULL);//7
	INI_Add_Hex(conf, "Color2", &config.color2, SL_COLOR_DEFAULT, NULL);//8
	INI_Add_Hex(conf, "Color3", &config.color3, EX_COLOR_DEFAULT, NULL);//9
	INI_Add_Hex(conf, "Color4", &config.color4, ON_COLOR_DEFAULT, NULL);//10
	INI_Add_Hex(conf, "Color5", &config.color5, OF_COLOR_DEFAULT, NULL);//11
	INI_Add_Bool(conf, "UsbConnect", &config.usbConnect, false );//12
	INI_Add_Button(conf, "UsbConnectKey", &config.usbConnectKey, (PSP_CTRL_RTRIGGER|PSP_CTRL_LTRIGGER|PSP_CTRL_UP) );//13
	INI_Add_Button(conf, "UsbDisconnectKey", &config.usbDisconnectKey, (PSP_CTRL_RTRIGGER|PSP_CTRL_LTRIGGER|PSP_CTRL_DOWN) );//14
	INI_Add_Hex(conf, "SortType", &config.sortType, 0, NULL);//15
#endif

	INI_Read_Conf(iniPath, conf);
	SET_CONFIG();
	
	//init
	for( i = 0; i < 3; i++ ){
		pdata[i].num = 0;
		pdata[i].edit = false;
		pdata[i].exist = false;
	}
	
	readSepluginsText(3,false,config.basePath);
	
#ifndef PPREFS_LITE
	if( config.usbConnect && usbState == 0 ) USBinit();
#endif

	

	PspSysEventHandler events;
	
	events.size			= 0x40;
	events.name			= "MSE_Suspend";
	events.type_mask	= 0x0000FF00;
	events.handler		= Callback_Suspend;
	
	sceKernelRegisterSysEventHandler(&events);
	
	padData.Buttons = 0;

#ifndef PPREFS_LITE

	//BOOT MESSAGE
	if( config.bootMessage ){
		clock_t timesec;

		strcpy(commonBuf,PPREFSMSG_BOOTMESSAGE);
		
		GET_KEY_NAME(config.bootKey, commonBuf);
		temp = strrchr(commonBuf, '+');
		if( temp != NULL ){
			temp[-1] = ' ';
			temp[0]  = '\0';
		}
		

		timesec = sceKernelLibcClock();
		while( stop_flag ){
			//表示
			if( libmInitBuffers(LIBM_DRAW_BLEND,PSP_DISPLAY_SETBUF_NEXTFRAME) ){
				libmPrint(0,264,SetAlpha(WHITE,0xFF),SetAlpha(BLACK,0xFF),commonBuf);
				sceDisplayWaitVblankStart();
			}

			sceCtrlPeekBufferPositive( &padData, 1 );
			if( ( padData.Buttons & CHEACK_KEY_2 ) != 0 ) break;
			if( (sceKernelLibcClock() - timesec) >= (8 * 1000 * 1000) ) break;

			sceKernelDelayThread( 10000 );
		}
	}
#endif

#ifndef PPREFS_LITE
	
	if( config.usbConnect && usbState == 0 ){
		while( stop_flag ){
			if((padData.Buttons & config.bootKey) == config.bootKey){
				disable_suspend = 1;
				main_menu();
				resumeThreads();
				disable_suspend = 0;
			}else if( usbState == 0 &&  (padData.Buttons & config.usbConnectKey) == config.usbConnectKey ){
				USBActivate();
				usbState = 1;
				wait_button_up_multithread(&padData);
			}else if( usbState != 0  && (padData.Buttons & config.usbDisconnectKey) == config.usbDisconnectKey ){
				USBDeactivate();
				usbState = 0;
				wait_button_up_multithread(&padData);
			}
			//    padData.Buttons ^= XOR_KEY;
			sceCtrlPeekBufferPositive( &padData, 1 );
			sceKernelDelayThread( 50000 );
		}
	}else{
#endif
		while( stop_flag ){
			if((padData.Buttons & config.bootKey) == config.bootKey){
				disable_suspend = 1;
				do{
					if( libmInitBuffers(LIBM_DRAW_BLEND,PSP_DISPLAY_SETBUF_NEXTFRAME) ){
						libmPrint(0,264,SetAlpha(WHITE,0xFF),SetAlpha(BLACK,0xFF),"pprefs: starting... please wait");
						sceDisplayWaitVblankStart();
					}
					sceKernelDelayThread( 50 );
				}while( safelySuspendForVSH() != 0 );
				main_menu();
				resumeThreads();
				disable_suspend = 0;
			}
			
			sceCtrlPeekBufferPositive( &padData, 1 );
			sceKernelDelayThread( 50000 );
		}

#ifndef PPREFS_LITE
	}
#endif
  return 0;
}


#define printEditedMark() libmPrint(63 , 24 , BG_COLOR , FG_COLOR,"*")

/*

@return:
どのメニューが実行されたか
 0 < : 何も実行されなかった
 0 = : メニューNo.1が実行された
 1 = : メニューNo.2が実行された
 2 = : メニューNo.3が実行された
・
・
・
*/

#ifdef PPREFS_LITE

int sub_menu(int currentSelected,int position){
	int now_arrow;

	if( deviceModel == 4 ){//if device is 'go'
		char *menu[] = {PPREFSMSG_SUBMENU_LIST_GO};
		now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
	}else{
		char *menu[] = {PPREFSMSG_SUBMENU_LIST};
		now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
	}

	wait_button_up(&padData);

	//追記
	if( now_arrow == 0 ){
		if( fileSelecter(config.basePath, &dirTmp, PPREFSMSG_ADD_TOP, 0, "ccbcccac") == 0 ){
			strcpy(tmp_pdataLine.path, dirTmp.name);
			tmp_pdataLine.toggle = false;
			addNewItem(now_type,&tmp_pdataLine);
			pdata[now_type].edit = true;
		}
	//削除
	}else if( now_arrow == 1 ){
		removeAnItem(now_type,currentSelected);
		pdata[now_type].edit = true;
	//backup
	}else if( now_arrow == 2 ){
		if( confirm_save() == 0 ){
			if( backupmenu(config.basePath, &now_type) != 0 ){
				pdata[now_type].edit = true;
				printEditedMark();
			}
		}
	//設定
	}else if( now_arrow == 3 ){
		if( confirm_save() == 0 ){
			config_menu();
			readSepluginsText(3,false,config.basePath);//re-read vsh.txt game.txt pops.txt
		}
	//COPY ME
	}else if( now_arrow == 4 ){
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
		if( tmp !=  1 ) readSepluginsText(3,true,config.basePath);
	}

	wait_button_up(&padData);
	
	return now_arrow;
	
}


#else

int sub_menu(int currentSelected,int position){
	int now_arrow;
/*
	char *menu_fat[] = { PPREFSMSG_SUBMENU_LIST };
	char *menu_go[] = { PPREFSMSG_SUBMENU_LIST_GO };
	char *menu_go_hitobashira[] = { PPREFSMSG_SUBMENU_LIST_GO_HITOBASHIRA };
	char **menu = (deviceModel == 4)?(hitobashiraFlag)?menu_go_hitobashira:menu_go:menu_fat;
*/	


	if( deviceModel == 4 ){//if device is 'go'
		if( hitobashiraFlag ){
			char *menu[] = {PPREFSMSG_SUBMENU_LIST_GO_HITOBASHIRA};
			now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
		}else{
			char *menu[] = {PPREFSMSG_SUBMENU_LIST_GO};
			now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
		}
	}else{
		if( hitobashiraFlag ){
			char *menu[] = {PPREFSMSG_SUBMENU_LIST_HITOBASHIRA};
			now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
		}else{
			char *menu[] = {PPREFSMSG_SUBMENU_LIST};
			now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
		}
	}


	wait_button_up(&padData);

	//追記
	if( now_arrow == 0 ){
		if( fileSelecter(config.basePath, &dirTmp, PPREFSMSG_ADD_TOP, 0, "ccbcccac") == 0 ){
			strcpy(tmp_pdataLine.path, dirTmp.name);
			tmp_pdataLine.toggle = false;
			addNewItem(now_type,&tmp_pdataLine);
			pdata[now_type].edit = true;
		}
	//削除
	}else if( now_arrow == 1 ){
		removeAnItem(now_type,currentSelected);
		pdata[now_type].edit = true;
	//backup menu
	}else if( now_arrow == 2 ){
		if( confirm_save() == 0 ){
			if( backupmenu(config.basePath, &now_type) != 0 ){
				pdata[now_type].edit = true;
				printEditedMark();
			}
		}
	//SORT GAME
	}else if( now_arrow == 3 ){
		sortgame_menu();
	//設定
	}else if( now_arrow == 4 ){
		if( confirm_save() == 0 ){
			config_menu();
			readSepluginsText(3,false,config.basePath);//re-read vsh.txt game.txt pops.txt
		}
	//pergame編集
	}else if( now_arrow == 5 ){
		editPergameMenu();
	//not GO -> COPY ME  / is GO -> omake
	}else if( now_arrow == 6 ){
		if(deviceModel == 4 ){
			if(  hitobashiraFlag ){
				pauseGameTest();
			}
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
			if( tmp !=  1 ) readSepluginsText(3,true,config.basePath);
		}
	//REMOVER
	}else if( now_arrow == 7 ){

		fileManager(config.basePath, "rm FILE", 0, NULL);
	}

	wait_button_up(&padData);
	
	return now_arrow;
	
}

#endif

#define swap_pdataLine(first,second) \
        strcpy( tmp_pdataLine.path , first.path ); \
        tmp_pdataLine.toggle = first.toggle; \
        strcpy( first.path  , second.path ); \
        first.toggle = second.toggle; \
        strcpy( second.path  , tmp_pdataLine.path ); \
        second.toggle = tmp_pdataLine.toggle;





#define printALine(start_y, lineNumber, fgColor) \
{ \
	if( pdata[now_type].line[lineNumber].toggle ){ \
		libmPrint(15, 38 + (start_y)*(LIBM_CHAR_HEIGHT+2) ,ON_COLOR, BG_COLOR, "[O N]"); \
	}else{ \
		libmPrint(15, 38 + (start_y)*(LIBM_CHAR_HEIGHT+2) ,OF_COLOR, BG_COLOR, "[OFF]"); \
	} \
	libmPrintf( \
		15 + (LIBM_CHAR_WIDTH*5 + 4), 38 + (start_y)*(LIBM_CHAR_HEIGHT+2) ,fgColor, BG_COLOR, \
		"%s", pdata[now_type].line[lineNumber].print \
	); \
}

#define clearAndPrintALine(start_y, lineNumber, fgColor) \
{ \
	fillLine(38 + (start_y)*(LIBM_CHAR_HEIGHT+2),BG_COLOR); \
	printALine(start_y, lineNumber, fgColor) \
}



//flag == 0 描画しない
//flag == 1 描画する
inline int move_arrow(u32 buttons, int *now_arrow, int *headOffset, int flag)
{
	int i,tmp;
	
	tmp = *now_arrow; //現在の矢印の位置を覚えておく
	//矢印の位置を変更 / change position of arrow
	if( buttons & PSP_CTRL_DOWN ){
		if(  *now_arrow + 1 < pdata[now_type].num )
			(*now_arrow)++;
		else
			*now_arrow = 0;
	}else if( buttons & PSP_CTRL_UP ){
		if( *now_arrow - 1 >= 0 )
			(*now_arrow)--;
		else
			*now_arrow = pdata[now_type].num - 1;
	}

	//□が押されてるなら、並び替え(そして編集フラグ立てる) / if □ is pushed , sort ( and flag edit )
	if( buttons & PSP_CTRL_SQUARE ){
		
		printEditedMark();
		pdata[now_type].edit = true;
		/*
		tmp - *now_arrow ==  1 one up
		tmp - *now_arrow == -1 one down
		tmp - *now_arrow >   1 up top      ( tmp > *now_arrow )
		tmp - *now_arrow <  -1 down bottom ( tmp < *now_arrow )
		*/
		if( tmp - *now_arrow > 1 ){
			for( i = tmp; i > *now_arrow; i-- ){
				swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i-1]);
			}
			flag = 0;
//			return 2;
		}else if( tmp - *now_arrow < -1 ){
			for( i = tmp; i < *now_arrow; i++ ){
				swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i+1]);
			}
			flag = 0;
//			return 2;
		}else{
			swap_pdataLine(pdata[now_type].line[*now_arrow],pdata[now_type].line[tmp]);
		}
	}
	
	//one up && arrow is out of screen of top
	if( tmp - *now_arrow == 1 && *headOffset > *now_arrow ){
		(*headOffset)--;
		return 4;
	//one down && arrow is out of screen of bottom
	}else if( tmp - *now_arrow == -1 && *headOffset+MAX_DISPLAY_NUM <= *now_arrow ){
		(*headOffset)++;
		return 8;
	}else if( pdata[now_type].num  > MAX_DISPLAY_NUM ){
		//up to top
		if( tmp - *now_arrow > 1 ){
			*headOffset = 0;
			return 16;
		//down to bottom
		}else if( tmp - *now_arrow <  -1 ){
			*headOffset = pdata[now_type].num - MAX_DISPLAY_NUM;
			return 32;
		}

	}
	
	if( flag ){
		//画面に表示 / display on screen
//		fillLine(38 + (tmp-*headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);libmPrintf(15,38 + (tmp-*headOffset)*(LIBM_CHAR_HEIGHT+2) , FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[tmp].toggle?"O N":"OFF",pdata[now_type].line[tmp].print);
//		fillLine(38 + (*now_arrow-*headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);libmPrintf(15,38 + (*now_arrow-*headOffset)*(LIBM_CHAR_HEIGHT+2) , SL_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[*now_arrow].toggle?"O N":"OFF",pdata[now_type].line[*now_arrow].print);
		clearAndPrintALine(tmp-*headOffset, tmp, FG_COLOR);
		clearAndPrintALine(*now_arrow-*headOffset, *now_arrow, SL_COLOR);
	}
	
	return 0;

}

/*
 矢印(カーソル)を↑↓長押しでもうまくスクロールするようにというマクロ
 なんとなく関数でなくマクロで実装してしまった
 */
#define ALLORW_WAIT(button,firstWait,wait) \
if( (beforeButtons & (button) ) == (button) ){ \
	if( firstFlag ){ \
		if( (sceKernelLibcClock() - time) >= (firstWait) ){ \
			time = sceKernelLibcClock(); \
			firstFlag = false; \
		}else{ \
			continue; \
		} \
	}else{ \
		if( (sceKernelLibcClock() - time) >= (wait) ){ \
			time = sceKernelLibcClock(); \
		}else{ \
			continue; \
		} \
	} \
}else{ \
	firstFlag = true; \
	beforeButtons = button; \
	time = sceKernelLibcClock(); \
} \


/*
 ボタン長押し こういうマクロの使い方はよくないかも
 */
#define HOLD_BUTTONS_WAIT(button,firstWait) \
if( (beforeButtons & (button) ) == (button) ){ \
	if( firstFlag && (sceKernelLibcClock() - time) >= (firstWait) ){ \
			firstFlag = false; \
	}else{\
		continue; \
	} \
}else{ \
	firstFlag = true; \
	beforeButtons = button; \
	time = sceKernelLibcClock(); \
	continue; \
}


void main_menu(void)
{

	wait_button_up(&padData);

	int i,tmp = 0,headOffset = 0;
	int now_arrow = 0;//current position of arrow
	clock_t time = 0;
	u32 beforeButtons = 0;
	bool firstFlag = true;

	readSepluginsText(3,true,config.basePath);
	


	while( ! libmInitBuffers(LIBM_DRAW_BLEND,PSP_DISPLAY_SETBUF_IMMEDIATE) ){
		sceDisplayWaitVblankStart();
	}
	
	PRINT_SCREEN();
//	safelySuspendThreadsInit();
	while(1){
//		libmInitBuffers(LIBM_DRAW_INIT8888,PSP_DISPLAY_SETBUF_NEXTFRAME);
//		PRINT_SCREEN();
		libmFillRect(0 , 38, 480, 38 + MAX_DISPLAY_NUM*(LIBM_CHAR_HEIGHT+2), BG_COLOR);


		if( hitobashiraFlag ) libmPrint(416,10,FG_COLOR,BG_COLOR,(hitobashiraFlag == 1)?PPREFSMSG_HITOBASHIRA:PPREFSMSG_HITOBASHIRA_2);
		else libmPrint(424,10,FG_COLOR,BG_COLOR,"  ");

		libmPrintf(0, 254, EX_COLOR, BG_COLOR, PPREFSMSG_MAINMENU_HOTOUSE,buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);
		libmPrintf(0, 264, EX_COLOR, BG_COLOR, PPREFSMSG_MAINMENU_HOTOUSE_2);

		libmPrintf(15,24,BG_COLOR,FG_COLOR,"<<[L]  %s [R]>>",getSepluginsTextName(commonBuf,config.basePath,now_type));
		if( pdata[now_type].edit ) printEditedMark();
		for( i = 0; i < MAX_DISPLAY_NUM && i < pdata[now_type].num; i++ ){
//			libmPrintf(15,38 + i*(LIBM_CHAR_HEIGHT+2),(now_arrow == i+headOffset)?SL_COLOR:FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[i+headOffset].toggle?"O N":"OFF",pdata[now_type].line[i+headOffset].print);
			printALine(i, i+headOffset, (now_arrow == i+headOffset)?SL_COLOR:FG_COLOR);
		}

		if( beforeButtons == 0 ) wait_button_up(&padData);
		while(1){
			/*
			//フリーズしないようにするため、0.5秒のwaitをもってからsuspend
			if( safelySuspendThreads(5 * 100 * 1000) == 1 ){//まさにいまsuspendした
				//描画に失敗しているかチェック
				libmPoint(libmMakeDrawAddr(0,0),CHECKCOLOR);
				while( ! libmInitBuffers(LIBM_DRAW_BLEND,PSP_DISPLAY_SETBUF_NEXTFRAME) ){
					sceDisplayWaitVblankStart();
				}
				if( libmGetColor(libmMakeDrawAddr(0,0)) != CHECKCOLOR ){//失敗してたら再描画
					PRINT_SCREEN();
					break;
				}
			}
			*/
			
			
			get_button(&padData);

			if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_LEFT) && pdata[now_type].num > 0 )
			{
				ALLORW_WAIT((PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_LEFT),3 * 100 * 1000,1 * 100 * 1000);

				//ここの描画処理が適当すぎるのでいつかなおそう

				tmp = 0;//use as flag
				if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP) ){//↓ / ↑
					tmp = move_arrow(padData.Buttons, &now_arrow, &headOffset, 1);
				}else if( !( padData.Buttons & PSP_CTRL_SQUARE ) ){// ← / → 
//					fillLine(38 + (now_arrow-headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);libmPrintf(15,38 + (now_arrow-headOffset)*(LIBM_CHAR_HEIGHT+2) , FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].print);
					clearAndPrintALine(now_arrow-headOffset, now_arrow, FG_COLOR);
					if( padData.Buttons & PSP_CTRL_LEFT ){
						for ( i = 0; i < 5; i++ ) tmp |= move_arrow( PSP_CTRL_UP,&now_arrow,&headOffset,0);
					}else{
						for ( i = 0; i < 5; i++ ) tmp |= move_arrow( PSP_CTRL_DOWN,&now_arrow,&headOffset,0);
					}
					clearAndPrintALine(now_arrow-headOffset, now_arrow, SL_COLOR);
//					fillLine(38 + (now_arrow-headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);libmPrintf(15,38 + (now_arrow-headOffset)*(LIBM_CHAR_HEIGHT+2) , SL_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].print);
				}
				
				if( tmp > 0 ) break;
			}
			else if( (padData.Buttons & buttonData[buttonNum[0]].flag) && pdata[now_type].num > 0 )
			{
				if( beforeButtons & buttonData[buttonNum[0]].flag ) continue;
				beforeButtons = buttonData[buttonNum[0]].flag;
				
				pdata[now_type].edit = true;
				printEditedMark();
				pdata[now_type].line[now_arrow].toggle = !pdata[now_type].line[now_arrow].toggle;
//				libmPrintf(15,38 + (now_arrow-headOffset)*(LIBM_CHAR_HEIGHT+2),SL_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].print);
				printALine(now_arrow-headOffset, now_arrow, SL_COLOR);
			}
			else if( (padData.Buttons & buttonData[buttonNum[1]].flag) && pdata[now_type].num > 0 )
			{
				
				HOLD_BUTTONS_WAIT(buttonData[buttonNum[1]].flag,3 * 100 * 1000);
				
				pdata[now_type].edit = true;

				tmp = 1;//use as flag
				for( i = 0; i < pdata[now_type].num; i++ ){//すべてOFFにする
					if( pdata[now_type].line[i].toggle == true ){
						pdata[now_type].line[i].toggle = false;
						tmp = 0;
					}
				}
				if( tmp ){//一つもONのプラグインがなかったら 全部ONにする
					for( i = 0; i < pdata[now_type].num; i++ ) pdata[now_type].line[i].toggle = true;
				}
				
				break;//再描画
			}
			else if( padData.Buttons & PSP_CTRL_RTRIGGER && padData.Buttons & PSP_CTRL_LTRIGGER )
			{//select directory
			
				if( (beforeButtons & (PSP_CTRL_RTRIGGER|PSP_CTRL_LTRIGGER)) == (PSP_CTRL_RTRIGGER|PSP_CTRL_LTRIGGER) ) continue;
				beforeButtons |= PSP_CTRL_RTRIGGER|PSP_CTRL_LTRIGGER;

//				suspendThreads();
				if( confirm_save() == 0 ){
					selectBasePath(config.basePath);
					readSepluginsText(3,false,config.basePath);
				}
				
				PRINT_SCREEN();
				break;
			}
			else if( padData.Buttons & PSP_CTRL_RTRIGGER )
			{
				if( (beforeButtons & (PSP_CTRL_RTRIGGER|PSP_CTRL_LTRIGGER)) == PSP_CTRL_RTRIGGER ) continue;
				beforeButtons = PSP_CTRL_RTRIGGER;
				
				if( now_type == 0 ) now_type = 1;
				else if( now_type == 1 ) now_type = 2;
				else if( now_type == 2 ) now_type = 0;
				now_arrow = 0;
				headOffset = 0;
				
				PRINT_SCREEN();
				break;
			}
			else if( padData.Buttons & PSP_CTRL_LTRIGGER )
			{
				if( (beforeButtons & (PSP_CTRL_RTRIGGER|PSP_CTRL_LTRIGGER)) == PSP_CTRL_LTRIGGER ) continue;
				beforeButtons = PSP_CTRL_LTRIGGER;
				
				if( now_type == 0 ) now_type = 2;
				else if( now_type == 1 ) now_type = 0;
				else if( now_type == 2 ) now_type = 1;
				now_arrow = 0;
				headOffset = 0;
				
				PRINT_SCREEN();
				break;
			}
			else if( padData.Buttons &  PSP_CTRL_TRIANGLE )
			{
				if( beforeButtons & PSP_CTRL_TRIANGLE ) continue;
				beforeButtons = PSP_CTRL_TRIANGLE;
				
//				suspendThreads();
				tmp = sub_menu(now_arrow,( now_arrow < 10 )?148:46);
				if( tmp == 0 ){
					now_arrow = pdata[now_type].num - 1;
					headOffset = pdata[now_type].num - MAX_DISPLAY_NUM;
					if( headOffset < 0 ) headOffset = 0;
				}else if( tmp == 1 ){
					if( now_arrow != 0 ){
						move_arrow(PSP_CTRL_UP,&now_arrow,&headOffset,0);
					}
				}else if( tmp >= 0 ){
					now_arrow = 0;
					headOffset = 0;
				}
				PRINT_SCREEN();
				break;
			}
			else if( padData.Buttons &  PSP_CTRL_START )
			{
				if( beforeButtons & PSP_CTRL_START ) continue;
				beforeButtons = PSP_CTRL_START;

				wait_button_up(&padData);
				
				tmp = 0;//as flag
				if( ! config.onePushRestart ){
					makeWindowWithGettingButton(
						100 , 36 ,
						 100 + LIBM_CHAR_WIDTH*20 , 44 + LIBM_CHAR_HEIGHT*5,
						 FG_COLOR,BG_COLOR,
						 &padData
					);
					libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,PPREFSMSG_MAINMENU_REPUSHSTART);
					while(1){
						if( padData.Buttons & PSP_CTRL_START ){
							libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,"RESTARTING...    ");
							libmPrint(100 + LIBM_CHAR_WIDTH , 44 + LIBM_CHAR_HEIGHT*2 , FG_COLOR,BG_COLOR,"                 ");
							tmp = 1;
							break;
						}else if( padData.Buttons & (CHEACK_KEY & ~PSP_CTRL_START) ){
							break;
						}
						get_button(&padData);
					}
				}else{
					tmp = 1;
				}
				
				if( tmp ){
					
					wait_button_up(&padData);
					saveEdit();
					resumeThreads();
					sceKernelExitVSHVSH(NULL);
					return;
				}
				
				wait_button_up(&padData);
				PRINT_SCREEN();
				break;
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				if( beforeButtons & PSP_CTRL_HOME ) continue;
				beforeButtons = PSP_CTRL_HOME;
				
				saveEdit();
				wait_button_up(&padData);
				resumeThreads();
				return;
			}
			else if( padData.Buttons & PSP_CTRL_SELECT )
			{
				if( beforeButtons & PSP_CTRL_SELECT ) continue;
				beforeButtons = PSP_CTRL_SELECT;

//				suspendThreads();

				if( pdata[0].edit || pdata[1].edit || pdata[2].edit ){
					char *menu[] = { PPREFSMSG_YESORNO_LIST };
					
					i = pprefsMakeSelectBox(24, 70, PPREFSMSG_MAINMENU_RELOAD,menu, buttonData[buttonNum[0]].flag, 1);
					
					if ( i == 0 ){
						readSepluginsText(3,false,config.basePath);
						now_arrow = 0;
						headOffset = 0;
					}
					
				}else{
					readSepluginsText(3,false,config.basePath);
				}
				
				wait_button_up(&padData);
				PRINT_SCREEN();
				break;
			}
			else if( padData.Buttons & PSP_CTRL_NOTE )
			{
				beforeButtons = PSP_CTRL_NOTE;
//				suspendThreads();
				i = 0;
				while(1){
					wait_button_up_ex(&padData,PSP_CTRL_NOTE);
					get_button(&padData);
					if( !( padData.Buttons & PSP_CTRL_NOTE ) ) break;
					
					/*
					if( padData.Buttons & PSP_CTRL_RIGHT ){
						if( i == 0 ){
							i = 1;
						}else if( i == 1 ){
							 i = 2;
						}else if( i == 4 ){
							if( sceIoUnassign("flash0:") >= 0 && sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0) >= 0 )
								hitobashiraFlag = 2;
							break;
						}
					}else if( padData.Buttons & PSP_CTRL_LEFT ){
						if( i == 2 ){
							hitobashiraFlag = 1;
							break;
						}else if( i == 0 ){
							i = 3;
						}else if( i == 3 ){
							i = 4;
						}
					}
					*/
					if( padData.Buttons & PSP_CTRL_RIGHT ){
						if( i == 0 ){
							i = 1;
						}else if( i == 1 ){
							i = 2;
						}
					}else if( padData.Buttons & PSP_CTRL_LEFT ){
						if( i == 2 ){
							hitobashiraFlag = 1;
							break;
						}
					}

					
					
					
					
				}
				break;
			}
			else
			{
				beforeButtons = 0;
			}
			
		}
	}

}

void getRootPath(char *dst,char *src)
{
	int i;
	for( i = 0; src[i] != '\0'; i++ ){
		dst[i] = src[i];
		if( dst[i] == '/' ){
			dst[i+1] = '\0';
			return;
		}
	}
	return;
}






int module_start( SceSize arglen, void *argp )
{
	nidResolve();
	Get_FirstThreads();



	strcpy(ownPath, argp);
	getRootPath(rootPath, argp);

	//deviceModel 多分
	//0 -> 1000
	//1 -> 2000
	//2 -> 3000 03g?
	//3 -> 3000 04g?
	//4 -> go
	//8 -> 3000 09g t箱?
	deviceModel = sceKernelGetModel();
	if( deviceModel < 0 || deviceModel > 8) deviceModel = 9;//unknown


	char path[256], *temp;
	strcpy(path, argp);
	temp = strrchr(path, '/');
	if( temp != NULL ) *temp = '\0';
	strcat(path,"/hitobashira.txt");
	if( ! check_file(path) ) hitobashiraFlag = 1;



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





int confirm_save(void)
{
	char *menu[] = {  PPREFSMSG_COPYME_SELECTLIST };
	if( pdata[0].edit || pdata[1].edit || pdata[2].edit ){
		int selectNum = pprefsMakeSelectBox(24,  28, PPREFSMSG_COPYME_FIRSTCONFIRM,menu, buttonData[buttonNum[0]].flag, 1 );

		if( selectNum == 0 ){
			saveEdit();
			return 0;
		}else if( selectNum == 1 ){
			return 0;
//		}else if( selectNum == 2 ){
		}
		return 1;
	}

	return 0;
}

int check_ms()
{
  SceUID ms;
  int ret = 0;

  ms = MScmIsMediumInserted();
  if(ms <= 0)
  {
 	makeWindow(LIBM_CHAR_WIDTH*9 ,LIBM_CHAR_HEIGHT*9 ,LIBM_CHAR_WIDTH*30 ,LIBM_CHAR_WIDTH*14, FG_COLOR,BG_COLOR );
	libmPrint( LIBM_CHAR_WIDTH*10 ,LIBM_CHAR_HEIGHT*10 , FG_COLOR,BG_COLOR,"Please insert MS");

    ms = -1;
    while(ms <= 0)
    {
      if( isButtonDown(PSP_CTRL_HOME) ) return -2;
      sceKernelDelayThread(1000);
      ms = MScmIsMediumInserted();
    }
	libmPrint( LIBM_CHAR_WIDTH*10 ,LIBM_CHAR_HEIGHT*11 + 2 , FG_COLOR,BG_COLOR,"Mounting...");
	sceKernelDelayThread(1000000);
    ret = -1;
  }
  return ret;
}

int checkMs(void)
{
	int ret = 0;
	
	SceUID dp = sceIoDopen(rootPath);
	if(dp < 0){
		ret = check_ms();
	}else{
		sceIoDclose(dp);
	}
	
	return ret;
}

int copyMeProcess(void){
	libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);



	if( confirm_save() != 0 ) return 1;
	
	
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
		if( checkMs() == 0 ) break;
	}
	
	sceIoMkdir("ms0:/seplugins",0777);
	
	
#ifdef PPREFS_LITE
	if( (fd = sceIoOpen("ms0:/seplugins/pprefs_lite.prx",PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC,0777)) < 0 ){
		free(buf);
		return -4;
	}

#else
	if( (fd = sceIoOpen("ms0:/seplugins/pprefs.prx",PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC,0777)) < 0 ){
		free(buf);
		return -4;
	}
#endif	
	writeSize = sceIoWrite(fd,buf,readSize);
	sceIoClose(fd);


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
//			suspendThreads();
			while(1){
				if( writeSepluginsText(i,config.basePath) < 0 ){
					makeWindow(24, 28,24 + LIBM_CHAR_WIDTH*26, 28 + LIBM_CHAR_HEIGHT*5, FG_COLOR, BG_COLOR);
					libmPrintf(24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT      , FG_COLOR,BG_COLOR,PPREFSMSG_FAILTOWRITE,getSepluginsTextName(commonBuf,config.basePath,i));
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




