#include <pspsysevent.h>

#include "sepluginstxt.h"
#include "configmenu.h"
#include "pprefsmenu.h"
#include "fileselecter.h"
#include "editpergame.h"
#include "sortgame.h"
#include "backupmenu.h"
#include "common.h"


// モジュールの定義
PSP_MODULE_INFO("PPREFS", PSP_MODULE_KERNEL, 0, 0);


/*------------------------------------------------------*/

static int now_type = 0;//This means ... 0 : vsh.txt / 1 : game.txt / 2:pops.txt
static struct pdataLine tmp_pdataLine;
static int stop_flag;
static volatile int disable_suspend = 0;
static const char *sepluginsTxtFileName[] = {
	"vsh.txt",
	"game.txt",
	"pops.txt"
};

static char *lineFeedCodeList[] = {
	"CR+LF",
	"LF",
	NULL
};

/*------------------------------------------------------*/

int Callback_Suspend(int ev_id, char *ev_name, void *param, int *result);

static 	PspSysEventHandler events = { 
	.size			= 0x40,
	.name			= "MSE_Suspend",
	.type_mask		= 0x0000FF00,
	.handler		= Callback_Suspend
};


/*------------------------------------------------------*/


#define MAX_DISPLAY_NUM 21




#define is5xx(x) ( ( (x) >= PSP_FIRMWARE(500) ) &&  ( (x) <  PSP_FIRMWARE(600) ) )



/*----------------------------------------------------------------------------*/
int module_start( SceSize arglen, void *argp );
int module_stop( void );

int main_thread( SceSize arglen, void *argp );
void main_menu(void);
int confirm_save(void);

int loadLibraries(void);

int copyMeProcess(void);

void saveEdit(void);



/*----------------------------------------------------------------------------*/




#define PSP_SYSEVENT_SUSPEND_QUERY 0x100 

int Callback_Suspend(int ev_id, char *ev_name, void *param, int *result)
{
	// 返り値に 0 ではなく -1 を返すと、スリープを無効化できる
	int ret = 0;
	
	if (ev_id == PSP_SYSEVENT_SUSPEND_QUERY && disable_suspend ) ret = -1;

	return ret;
}





/////////////////////////////////////
/*
struct prxLibraryList{
	char *name;
	char *path;
};


#define NELEMS(a) (sizeof(a) / sizeof(a[0]))

int LoadStartModule(char *module)
{
	SceUID mod = sceKernelLoadModule(module, 0, NULL);
	
	if (mod < 0) return mod;
	
	return sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}


int loadLibraries( void )
{
	int i, j, ret;
	char path[256];
	char *root[] = { "ms0:/", "ef0:/", "flash0:/" };
	struct prxLibraryList list[] = {
		{ "cmlibMenu",  "seplugins/lib/cmlibmenu.prx"  }
	};
	
	if( strncasecmp(rootPath, "ef0:/", 5) == 0 ){
		char *tmp = root[0];
		root[0] = root[1];
		root[1] = tmp;
	}
	
	
	for( i = 0; i < NELEMS(list); i++ ){
		if( sceKernelFindModuleByName(list[i].name) ) continue;
		
		ret = -1;
		for( j = 0; j < NELEMS(root) && ret < 0; j++ ){
			strcpy(path, root[j]);
			strcat(path, list[i].path);
			ret = LoadStartModule(path);
		}
		if( ret < 0 ) return ret;
	}
	
	return 0;
}
*/
//////////////////////////////////////
//thanks to neur0n
void ClearCaches(void)
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

//takka氏の umd_dump より malloc を拝借
static void *p_malloc(u32 size)
{
//	u32 *p;
	void *ptr;
	SceUID h_block;
	
	if(size == 0)
		return NULL;
	
	h_block = sceKernelAllocPartitionMemory(1, "block", 0, size/* + sizeof(h_block)*/, NULL);
	
	if(h_block <= 0)
		return NULL;
	
//	p = (u32 *)sceKernelGetBlockHeadAddr(h_block);
//	*p = h_block;
	
//	return (void *)(p + 1);
	
	ptr = (void *)sceKernelGetBlockHeadAddr(h_block);
	return ptr;
}
/*
static s32 p_mfree(void *ptr)
{
	return sceKernelFreePartitionMemory((SceUID)*((u32 *)ptr - 1));
}
*/

void *RedirectFunction(void *addr, void *func)
{
	u32 orgaddr = (u32)addr;
	if( orgaddr != 0 )
	{
		u32 buff = (u32)p_malloc( 4 * 4 );
		if( buff == 0 ) return 0;
		
		memcpy( (void *)buff, (void *)orgaddr, 4 * 2 );
		MAKE_JUMP( (buff + 4 * 2) , (orgaddr + 4 * 2) );
		_sw( NOP, buff + 4 * 3 );
		
		MAKE_JUMP( (u32)(orgaddr), (u32)(func) );
		_sw( NOP, orgaddr + 4 * 1 );
		
		orgaddr = (u32)buff;
		
		ClearCaches();
	}
	
	return (void *)orgaddr;
}

static SceUID (*_sceKernelLoadModule)(const char *path, int flags, SceKernelLMOption *option);
static u32 sceKernelLoadModule_orgaddr;
SceUID sceKernelLoadModule_Patched(const char *path, int flags, SceKernelLMOption *option)
{
	
	int i;
	for( i = 0; i < pdata[0].num; i++ ){
		if(!strcasecmp(path, pdata[0].line[i].path)){
			return 0x80010002;
		}
	}

	return _sceKernelLoadModule(path, flags, option);
}

void hookSceKernelLoadModule(void)
{
	sceKernelLoadModule_orgaddr = FindProc("sceModuleManager", "ModuleMgrForKernel", 0x977DE386);
	_sceKernelLoadModule = RedirectFunction((void *)sceKernelLoadModule_orgaddr, sceKernelLoadModule_Patched);
}
/////////////////////////////////////

inline void bootMenu()
{
	disable_suspend = 1;
		
	threadCtrlSuspend();
	main_menu();
	threadCtrlResume();
	
	disable_suspend = 0;	
}

int pprefsInit(SceSize arglen, void *argp)
{
	nidResolve();
	threadCtrlInit();
	
	//set own path & root path
	char *ptr = strchr(argp, '/');
	if( ptr != NULL ){
		int len = ptr - (char *)argp + 1;
		memcpy(rootPath, argp, len);
		rootPath[len] = '\0';
		
		strcpy(ownPath, argp);
	}else{
		strcpy(rootPath, "ms0:/");
		
		strcpy(ownPath, "ms0:/");
	}
	
	//set device model number & name
	deviceModel = sceKernelGetModel();
	if( deviceModel == 4 ){
		strcpy(modelName, "[g o]");
	}else{
		snprintf(modelName, 6, "[%02d]", deviceModel + 1);
	}
	
	//check hitobashira
	char path[257];
	strcpy(path, argp);
	ptr = strrchr(path, '/');
	if( ptr != NULL ){
		*ptr = '\0';
	}else{
		ptr = strrchr(path, '\0');
	}
	
	strcat(path, "/hitobashira.txt");
	if( check_file(path) == 0 ){
		hitobashiraFlag = 1;
	}
	
	//load INI
	*ptr = '\0';
	strcat(path, INI_NAME);
	
	strcpy(basePathDefault, rootPath);
	if( sceKernelDevkitVersion() == PSP_FIRMWARE(0x635) && sctrlHENGetVersion() == 0x1001 && sceKernelFindModuleByName("VshCtrl") == NULL ){
		//if it's 6.35PRO(HEN)
		strcat(basePathDefault,"plugins/");
	}else{
		strcat(basePathDefault, "seplugins/");
	}
	
	ILPInitKey(conf);
	
	ILPRegisterButton(conf, "BootKey", &config.bootKey, PSP_CTRL_HOME, NULL);//0
	ILPRegisterBool(conf, "SwapButton", &config.swapButton, false );//1
	ILPRegisterBool(conf, "OnePushRestart", &config.onePushRestart, false );//2
	ILPRegisterList(conf, "LineFeedCode", &config.lineFeedCode, 0, lineFeedCodeList);//3
	ILPRegisterString(conf, "BasePath", config.basePath, basePathDefault);//4
	ILPRegisterHex(conf, "Color0", &config.color0, FG_COLOR_DEFAULT);//5
	ILPRegisterHex(conf, "Color1", &config.color1, BG_COLOR_DEFAULT);//6
	ILPRegisterHex(conf, "Color2", &config.color2, SL_COLOR_DEFAULT);//7
	ILPRegisterHex(conf, "Color3", &config.color3, EX_COLOR_DEFAULT);//8
	ILPRegisterHex(conf, "Color4", &config.color4, ON_COLOR_DEFAULT);//9
	ILPRegisterHex(conf, "Color5", &config.color5, OF_COLOR_DEFAULT);//10
	ILPRegisterButton(conf, "DisablePluginsKey", &config.disablePluginsKey, PSP_CTRL_LTRIGGER, NULL);//11
	
#ifndef PPREFS_LITE	
	ILPRegisterBool(conf, "BootMessage", &config.bootMessage, true );//1
	ILPRegisterHex(conf, "SortType", &config.sortType, 0);//15
#endif
	
	ILPReadFromFile(conf, path);
	
	pprefsApplyConfig();
	
	//init
	int i;
	for( i = 0; i < 3; i++ ){
		pdata[i].num = 0;
		pdata[i].edit = false;
		pdata[i].exist = false;
	}
	
	readSepluginsText(3,false,config.basePath);
	
	return 0;
}


int main_thread(SceSize arglen, void *argp)
{
	
	sceKernelRegisterSysEventHandler(&events);
	//wait
	while( sceKernelFindModuleByName("sceKernelLibrary") == NULL ){
		sceKernelDelayThread(1000);
	}
	
	//load prx common libraries
    if( loadLibraries() < 0 ){
		return sceKernelExitDeleteThread(0);
	}
	sceKernelDelayThread(5 * 100 * 1000);
	
	//init cmlibmenu
	dinfo.vinfo = &vinfo;
	libmLoadFont(LIBM_FONT_SJIS);
	sceKernelDelayThread(5 * 100 * 1000);
	libmLoadFont(LIBM_FONT_CG);
	sceKernelDelayThread(5 * 100 * 1000);
	

#ifndef PPREFS_LITE
	//BOOT MESSAGE
	if( config.bootMessage ){
		clock_t timesec;
		char *temp;

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

	
	while( stop_flag ){
		if((padData.Buttons & config.bootKey) == config.bootKey){
			bootMenu();
		}
		
		sceCtrlPeekBufferPositive( &padData, 1 );
		sceKernelDelayThread( 50000 );
	}
	
	
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


int sub_menu(int currentSelected, int position){
	int now_arrow;

#ifdef PPREFS_LITE	
	if( deviceModel == 4 ){//if device is 'go'
		char *menu[] = {PPREFSMSG_SUBMENU_LIST_GO};
		now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
	}else{
		char *menu[] = {PPREFSMSG_SUBMENU_LIST};
		now_arrow = pprefsMakeSelectBox(8,  position, PPREFSMSG_SUBMENU_TITLE,menu, buttonData[buttonNum[0]].flag, 1 );
	}
#else	
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
#endif

	wait_button_up(&padData);

	switch (now_arrow) {
		case 0://add
		{
			if( fileSelecter(config.basePath, &dirTmp, PPREFSMSG_ADD_TOP, 0, "ccbcccac") == 0 ){
				strcpy(tmp_pdataLine.path, dirTmp.name);
				tmp_pdataLine.toggle = false;
				addNewItem(now_type,&tmp_pdataLine);
				pdata[now_type].edit = true;
			}
			break;
		}
		case 1://remove
		{
			removeAnItem(now_type, currentSelected);
			pdata[now_type].edit = true;
			break;
		}
		case 2://backup
		{
			if( confirm_save() == 0 ){
				if( backupmenu(config.basePath, &now_type) != 0 ){
					pdata[now_type].edit = true;
					printEditedMark();
				}
			}
			break;
		}
		case 3://config
		{
			if( confirm_save() == 0 ){
				config_menu();
				readSepluginsText(3,false,config.basePath);//re-read vsh.txt game.txt pops.txt
			}			
			break;
		}
		case 4://sortgame
		{
			sortgame_menu();
			break;
		}
		case 5://edit pergame
		{
			editPergameMenu();
			break;
		}
		case 6:
		{
			if(deviceModel != 4){
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
			}else {				
				fileManager(config.basePath, "remove FILE", 0, NULL);
			}
		}
		case 7:
		{
			fileManager(config.basePath, "rm FILE", 0, NULL);
		}

	}
	
	wait_button_up(&padData);
	
	return now_arrow;
	
}

/*
inline void swapPdataLine()
{
	strcpy(tmp_pdataLine.path , first.path);
	tmp_pdataLine.toggle = first.toggle;
	strcpy(first.path  , second.path);
	first.toggle = second.toggle;
	strcpy(second.path  , tmp_pdataLine.path);
	second.toggle = tmp_pdataLine.toggle;
}
*/
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
		"%s", (pdata[now_type].line[lineNumber].path + pdata[now_type].line[lineNumber].print) \
	); \
}

#define clearAndPrintALine(start_y, lineNumber, fgColor) \
{ \
	fillLine(38 + (start_y)*(LIBM_CHAR_HEIGHT+2),BG_COLOR); \
	printALine(start_y, lineNumber, fgColor) \
}



#define MOVE_ARROW_UP 1
#define MOVE_ARROW_DRAW 2
#define MOVE_ARROW_SORT 4
//if MOVE_ARROW_UP flag is down, the arrow will move to down
inline int move_arrow(u32 flag, int *now_arrow, int *headOffset)
{
	int i,before_arrow;
	
	before_arrow = *now_arrow; //現在の矢印の位置を覚えておく
	//矢印の位置を変更 / change position of arrow
	if( flag & MOVE_ARROW_UP ){
		if( *now_arrow - 1 >= 0 ) (*now_arrow)--;
		else *now_arrow = pdata[now_type].num - 1;
	}else{
		if(  *now_arrow + 1 < pdata[now_type].num ) (*now_arrow)++;
		else *now_arrow = 0;
	}
	
	//□が押されてるなら、並び替え(そして編集フラグ立てる) / if □ is pushed , sort ( and flag edit )
	if( flag & MOVE_ARROW_SORT ){

		printEditedMark();
		pdata[now_type].edit = true;
		/*
		 before_arrow - *now_arrow ==  1 <==> up once
		 before_arrow - *now_arrow == -1 <==> down once
		 before_arrow - *now_arrow >   1 <==> up to top      ( before_arrow > *now_arrow )
		 before_arrow - *now_arrow <  -1 <==> down to bottom ( before_arrow < *now_arrow )
		 */
		if( before_arrow - *now_arrow > 1 ){//up to top
			for( i = before_arrow; i > *now_arrow; i-- ){
				swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i-1]);
			}
//			flag = 0;
			//			return 2;
		}else if( before_arrow - *now_arrow < -1 ){//down ot bottom
			for( i = before_arrow; i < *now_arrow; i++ ){
				swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i+1]);
			}
//			flag = 0;
			//			return 2;
		}else{//up or down once
			swap_pdataLine(pdata[now_type].line[*now_arrow],pdata[now_type].line[before_arrow]);
		}
	}
	
	//one up && arrow is out of screen of top
	if( before_arrow - *now_arrow == 1 && *headOffset > *now_arrow ){
		(*headOffset)--;
		return 4;
		//one down && arrow is out of screen of bottom
	}else if( before_arrow - *now_arrow == -1 && *headOffset+MAX_DISPLAY_NUM <= *now_arrow ){
		(*headOffset)++;
		return 8;
	}else if( pdata[now_type].num  > MAX_DISPLAY_NUM ){
		//up to top
		if( before_arrow - *now_arrow > 1 ){
			*headOffset = 0;
			return 16;
			//down to bottom
		}else if( before_arrow - *now_arrow <  -1 ){
			*headOffset = pdata[now_type].num - MAX_DISPLAY_NUM;
			return 32;
		}
		
	}
	
	if( flag & MOVE_ARROW_DRAW ){
		//画面に表示 / display on screen
		//		fillLine(38 + (before_arrow-*headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);libmPrintf(15,38 + (before_arrow-*headOffset)*(LIBM_CHAR_HEIGHT+2) , FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[before_arrow].toggle?"O N":"OFF",pdata[now_type].line[before_arrow].print);
		//		fillLine(38 + (*now_arrow-*headOffset)*(LIBM_CHAR_HEIGHT+2),BG_COLOR);libmPrintf(15,38 + (*now_arrow-*headOffset)*(LIBM_CHAR_HEIGHT+2) , SL_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[*now_arrow].toggle?"O N":"OFF",pdata[now_type].line[*now_arrow].print);
		clearAndPrintALine(before_arrow-*headOffset, before_arrow, FG_COLOR);
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
	
	libmInitBuffers(LIBM_DRAW_INIT8888,PSP_DISPLAY_SETBUF_NEXTFRAME);
	printScreen();
	
	
	while(1){
		libmInitBuffers(LIBM_DRAW_INIT8888,PSP_DISPLAY_SETBUF_NEXTFRAME);
		printScreen();
		libmFillRect(0 , 38, 480, 38 + MAX_DISPLAY_NUM*(LIBM_CHAR_HEIGHT+2), BG_COLOR);

		if( hitobashiraFlag )
			libmPrint(416,10,FG_COLOR,BG_COLOR,(hitobashiraFlag == 1)?PPREFSMSG_HITOBASHIRA:PPREFSMSG_HITOBASHIRA_2);
		else 
			libmPrint(424,10,FG_COLOR,BG_COLOR,"  ");
		
		libmPrintf(0, 254, EX_COLOR, BG_COLOR, PPREFSMSG_MAINMENU_HOTOUSE, buttonData[buttonNum[0]].name, buttonData[buttonNum[1]].name);
		libmPrint(0, 264, EX_COLOR, BG_COLOR, PPREFSMSG_MAINMENU_HOTOUSE_2);

		libmPrintf(15,24,BG_COLOR,FG_COLOR,"<<[L]  %s%s [R]>>", basePathCurrent, sepluginsTxtFileName[now_type]);

		if( pdata[now_type].edit ) 
			printEditedMark();
		for( i = 0; i < MAX_DISPLAY_NUM && i < pdata[now_type].num; i++ ){
			libmPrintf(15,38 + i*(LIBM_CHAR_HEIGHT+2),(now_arrow == i+headOffset)?SL_COLOR:FG_COLOR,BG_COLOR,"[%s] %s",pdata[now_type].line[i+headOffset].toggle?"O N":"OFF",pdata[now_type].line[i+headOffset].print);
			printALine(i, i+headOffset, (now_arrow == i+headOffset)?SL_COLOR:FG_COLOR);
		}

		if( beforeButtons == 0 )
			wait_button_up(&padData);
		
		while(1){

			get_button(&padData);

			if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_LEFT) && pdata[now_type].num > 0 )
			{
				ALLORW_WAIT((PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_LEFT),3 * 100 * 1000,1 * 100 * 1000);

				tmp = 0;//use as flag here
				if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP) ){//↓ / ↑
					tmp = MOVE_ARROW_DRAW;
					if(padData.Buttons & PSP_CTRL_UP) tmp |= MOVE_ARROW_UP;
					if(padData.Buttons & PSP_CTRL_SQUARE) tmp |= MOVE_ARROW_SORT;
					tmp = move_arrow(tmp, &now_arrow, &headOffset);
				}else if( !( padData.Buttons & PSP_CTRL_SQUARE ) ){// ← / → 
					clearAndPrintALine(now_arrow-headOffset, now_arrow, FG_COLOR);
					if( padData.Buttons & PSP_CTRL_LEFT ){
						for ( i = 0; i < 5; i++ ) tmp |= move_arrow(MOVE_ARROW_UP, &now_arrow, &headOffset);
					}else{
						for ( i = 0; i < 5; i++ ) tmp |= move_arrow(0, &now_arrow, &headOffset);
					}
					clearAndPrintALine(now_arrow-headOffset, now_arrow, SL_COLOR);
				}
				
				if( tmp > 0 ) break;//re-draw all
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
				
				printScreen();
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
				
				printScreen();
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
				
				printScreen();
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
					if( now_arrow == pdata[now_type].num ){
						if( now_arrow > 0 ) now_arrow--;
					}
					if( headOffset > 0 ) headOffset--;
				}else if( tmp >= 0 ){
					now_arrow = 0;
					headOffset = 0;
				}
				printScreen();
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
					threadCtrlResume();
					sceKernelExitVSHVSH(NULL);
					return;
				}
				
				wait_button_up(&padData);
				printScreen();
				break;
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				if( beforeButtons & PSP_CTRL_HOME ) continue;
				beforeButtons = PSP_CTRL_HOME;
				
				saveEdit();
				wait_button_up(&padData);
				threadCtrlResume();
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
				printScreen();
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



int module_start( SceSize arglen, void *argp )
{		
	if( pprefsInit(arglen, argp) < 0 ){
		return 0;
	}
	
	sceCtrlPeekBufferPositive( &padData, 1 );	
	if( (padData.Buttons & config.disablePluginsKey) == config.disablePluginsKey ){
		hookSceKernelLoadModule();
	}
	
	stop_flag = 1;
	
	SceUID thid = sceKernelCreateThread( "PPREFS", main_thread, 30, 0x6000, PSP_THREAD_ATTR_NO_FILLSTACK, 0 );
	if( thid >= 0 ){
		sceKernelStartThread( thid, arglen, argp );
	}

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
					libmPrintf(24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT      , FG_COLOR,BG_COLOR,PPREFSMSG_FAILTOWRITE,basePathCurrent, sepluginsTxtFileName[i]);
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



int loadLibraries(void)
{
	int res = 0;
	char path[] = "ms0:/seplugins/lib/cmlibmenu.prx";
	
	if( sceKernelFindModuleByName("cmlibMenu") == NULL ){
		res = pspSdkLoadStartModule(path, PSP_MEMORY_PARTITION_KERNEL);
		if( res < 0 ){
			if( path[0] == 'm' && path[1] == 's' ){
				path[0] = 'e';
				path[1] = 'f';
			}else{
				path[0] = 'm';
				path[1] = 's';
			}
			
			res = pspSdkLoadStartModule(path, PSP_MEMORY_PARTITION_KERNEL);
		}
	}
	
	return res;
}




