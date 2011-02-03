/*
	
	ありがとう、
	maxemさん、plumさん、takkaさん(アルファベット順)

*/


#include <pspkernel.h>
#include <pspsdk.h>
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>

#include <systemctrl.h>
#include <pspinit.h>

#include <psprtc.h>



#include "common.h"
#include "libmenu.h"
#include "memory.h"
#include "file.h"
#include "button.h"
#include "thread.h"
#include "pprefsmenu.h"
#include "language.h"

// モジュールの定義
PSP_MODULE_INFO( "PTEXTVIEWER", PSP_MODULE_KERNEL, 0, 0 );



/*------------------------------------------------------
 COMMON
------------------------------------------------------*/

char commonBuf[COMMON_BUF_LEN];
char sepluginsTextPath[3][64];
const char *sepluginsBasePath[] = {
	"ms0:/seplugins/",
	"ms0:/plugins/"
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

int now_type = 0;

bool now_state = false; // = true suspending   = false no suspending

bool hitobashiraFlag = false;

int deviceModel = 9;



/*------------------------------------------------------*/

// http://katsura-kotonoha.sakura.ne.jp/prog/c/tip00010.shtml
#define jms1(c) \
(((0x81 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0x9F)) || ((0xE0 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC) ))

#define jms2(c) \
((0x7F != (unsigned char)(c)) && (0x40 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC))


#define MAXDISPLAY_X 60
#define MAXDISPLAY_Y 30

#define PTXTBUF_SIZE (1024*1 + 1)

enum
{
	SELECTED_TOP		= 1,
	SELECTED_BOTTOM		= 2,
	TAIL_IS_EOF			= 4,
	TAIL_IS_N			= 8,
	ERROR_TOOBIG		= 16,
	USE_MOREBUF			= 32,
};

/*-----------------------------------------------------*/


int stop_flag;

char ptxtBuf_stack[PTXTBUF_SIZE];
char *ptxtBuf = ptxtBuf_stack;
char *ptxtBuf_more = NULL;

int ptxt_headOffset = 0;
int ptxt_readSize = 0;
int ptxt_readSize_more = 0;




/*-----------------------------------------------------------------*/

//文字列から\rを取り除く
int remove_r(char *str)
{
	int i,j;
	
	for( i = 0; str[i] != '\0'; i++ ){
		if( str[i] == '\r' ){
			for( j = i; str[j] != '\0'; j++ ) str[j] = str[j+1]; //1文字詰める
		}
	}
	
	return i;
}


//次の行のオフセットをget
int getOffsetOfTrueNextLine(char *str,int offset)
{
	int i;
	for( i = offset; str[i] != '\0'; i++ ){
		if( str[i] == '\n' ) return i+1;
	}
	//if str[i] == '\0'
	return offset;
}

//(表示上の)次の行のオフセットをget
int getOffsetOfNextLine(char *str,int offset)
{
	int i;
	int count = 0;
	for( i = offset; str[i] != '\0'; i++ ){
		if( str[i] == '\n' ) return i+1;
		//2byte文字なら
		if( jms1(str[i]) && jms2(str[i+1]) ) i++;
		count++;
		if( count >= MAXDISPLAY_X ) return i+1;
	}
	//if str[i] == '\0'
	return offset;
}

int getOffsetOfTruePreviousLine(char *str,int offset)
{

	if( offset <= 0 ) return offset;

	int i;
	
	for( i = (str[offset-1] == '\n')?(offset-2):(offset-1); i >= 0 && str[i] != '\n' ; i-- );
	i++;
	return i;
}

int getOffsetOfPreviousLine(char *str,int offset)
{
	
	if( offset <= 0 ) return offset;
	int i,tmp;
	i = getOffsetOfTruePreviousLine(str, offset);
	while(1){
		tmp = getOffsetOfNextLine(str,i);
		if( tmp == offset ) return i;
		i = tmp;
	}
}


//1文字描画
int pprefsPutChar( int x, int y, u32 fg, u32 bg, const char *str )
{
	int ret;
	char tmp[3] = { str[0], 0, '\0' };

	if( jms1(str[0]) ){//2byte文字なら
		tmp[1] = str[1];
		ret = 1;
	}else{
		tmp[2] = '\0';
		ret = 0;
	}
	libmPrint( x, y, fg, bg, tmp );
	return ret;
}


u16  printText( int headOffset,  int end_y)
{
	int i,x_count = 0,y_count = 0,ret = 0;
	char *pergameBuf;
	if( headOffset >= ptxt_readSize ){
		if( ptxtBuf_more == NULL ) return ERROR_TOOBIG;

		pergameBuf = ptxtBuf_more;
		i = headOffset - ptxt_readSize;
	}else{
		pergameBuf = ptxtBuf;
		i = headOffset;
	}
	
	for( ; ; i++ ){
		if( pergameBuf[i] == '\0' ){
			if( pergameBuf == ptxtBuf && ptxtBuf_more != NULL ){
				pergameBuf = ptxtBuf_more;
				i = 0;
				ret |= USE_MOREBUF;
			}else{
				ret |= TAIL_IS_EOF;
				break;
			}
		}
		
		if( pergameBuf[i] == '\n' ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
			continue;
		}
		
		if( pprefsPutChar(LIBM_CHAR_WIDTH*x_count ,19+LIBM_CHAR_HEIGHT*y_count ,FG_COLOR, BG_COLOR, &pergameBuf[i])
		   == 1 ) i++; //2byte文字なら
		
		x_count++;
		if( x_count >= MAXDISPLAY_X ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
		}
	}
	
	if( pergameBuf[i] == '\n' ) ret |= TAIL_IS_N;
	
	return ret;
}

//buf, bufの始まりの位置, 始まりの行数, 現在の選択されている行数, yの最大数
u16  noPrintText( int headOffset,  int end_y)
{
	int i,x_count = 0,y_count = 0,ret = 0;
	char *pergameBuf;
	if( headOffset >= ptxt_readSize ){
		if( ptxtBuf_more == NULL ) return ERROR_TOOBIG;

		pergameBuf = ptxtBuf_more;
		i = headOffset - ptxt_readSize;
	}else{
		pergameBuf = ptxtBuf;
		i = headOffset;
	}
	
	for( ; ; i++ ){
		if( pergameBuf[i] == '\0' ){
			if( pergameBuf == ptxtBuf && ptxtBuf_more != NULL ){
				pergameBuf = ptxtBuf_more;
				i = 0;
			}else{
				ret |= TAIL_IS_EOF;
				break;
			}
		}
		
		if( pergameBuf[i] == '\n' ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
			continue;
		}
		
//		if( pprefsPutChar(LIBM_CHAR_WIDTH*x_count ,19+LIBM_CHAR_HEIGHT*y_count ,FG_COLOR, BG_COLOR, &pergameBuf[i])
//		   == 1 ) i++; //2byte文字なら
		if( jms1(pergameBuf[i]) ) i++;
		
		x_count++;
		if( x_count >= MAXDISPLAY_X ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
		}
	}
	
	if( pergameBuf[i] == '\n' ) ret |= TAIL_IS_N;
	
	return ret;
}

//start = 行の先頭のオフセット
//指定された行を削除
void deleteLineOfString(char *str,int start)
{
	int i;
	int flag = 1;
	
	while( flag != 0 ){
		if( str[start] == '\n' ||  str[start] == '\0' ) flag = 0;
		for( i = start; str[i] != '\0'; i++ ) str[i] = str[i+1]; //1文字詰める
	}
}

//渡された行のコメントアウトを入れ替え
void changeHeadChar(char *str, int start,char c, int maxSize)
{
	int i;
	if( str[start] == c ){
		for( i = start; str[i] != '\0'; i++ ) str[i] = str[i+1]; //1文字詰める
	}else{
		if( start+1 >= maxSize ) return;
		for( i = strlen(str); i >= start; i-- ) str[i+1] = str[i];//1文字右にシフト
		str[start] = c;
	}
}

/*
 shiftというよりswapな気がする
 str[start]の文字を左にnumだけ入れ替えずらす
 str = "abcdEf"; start = 4; num = 2;なら strは "abEcdf"になる / if str = "abcdEf"; start = 4; num = 2; , str will become "abEcdf"
 */
void leftShiftChar(char *str, int start, int num)
{
	int i;
	char tmp;
	for( i = 0; num + i > 0; i-- ){
		tmp = str[start+i];
		str[start+i] = str[start+i-1];
		str[start+i-1] = tmp;
	}
}

void rightShiftChar(char *str, int start, int num)
{
	int i;
	char tmp;
	for( i = 0; i < num; i++ ){
		tmp = str[start+i];
		str[start+i] = str[start+i+1];
		str[start+i+1] = tmp;
	}
}

//不正な文字列を渡すとどうなるか分からない
//str = "abcdefg\nhijklmn\nopqrstu\0" のようなタイプの文字列の行を入れ替える
void swapLineOfString(char *str, int first, int second)
{
	char tmp;
	int i,shiftNum;
	
	i = 0;
	while(1){
		tmp = str[first+i];
		str[first+i] = str[second+i];
		str[second+i] = tmp;
		
		if( str[first+i] == '\n' || str[first+i] == '\0' || str[second+i] == '\n' || str[second+i] == '\0' ) break;
		i++;
	}
	
	if( str[second+i] == '\n' || str[second+i] == '\0' ){
		i = second + i + 1;
		shiftNum = i-second;
		tmp = 1;//tmp is used as flag here
		while( tmp != 0 ){
			if( str[i] == '\n' || str[i] == '\0' ) tmp = 0;
			leftShiftChar(str,i,shiftNum);
			i++;
		}
	}else if( str[first+i] == '\n' || str[first+i] == '\0'){
		i = first + i + 1;
		shiftNum = second - first -1;
		tmp = 1;//tmp is used as flag here
		while( tmp != 0 ){
			if( str[i] == '\n' || str[i] == '\0'  ) tmp = 0;
			rightShiftChar(str,i,shiftNum);
		}
	}
	
}

int getNumberOfLineOfString(char *str)
{
	int lineCount = 0,i = 0;
	
	for( i = 0; str[i] != '\0'; i++ ){
		if( str[i] == '\n' ) lineCount++;
	}
	lineCount++;
	return lineCount;
}
int getOffsetOfSelecteLine(char *str, int selectedLine)
{
	int i,tmp;
	i = 0;
	tmp = 0;
	while( i != selectedLine ){
		tmp = getOffsetOfTrueNextLine(str, tmp);
		i++;
	}
	return tmp;
}

int getNumberOfLineOfStringFormOffset(char *str, int offset)
{
	int lineCount = 0,i = 0;
	
	for( i = 0; i <= offset; i++ ){
		if( str[i] == '\n' ) lineCount++;
	}
	//	lineCount++;
	return lineCount;
}


/*-----------------------------------------------------------------*/

//ファイルリスト(ファイルブラウザ)のための変数
dir_t dir_buf[128];
dir_t now_open_file;
int dir_print_position = 0;
int dir_now_arrow = 0;
int dir_num = 0;
char currentPath[256];

//in file.c
extern const char dir_type_sort_default[];

int now_display_flag = 0;

clock_t safely_suspend_time;


/*-----------------------------------------------------------------*/

#define MAX_DISPLAY_NUM 21

#define DIR_READ() \
dir_num = read_dir(dir_buf,currentPath, 0,NULL); \
dir_now_arrow = 0; \
dir_print_position = 0; \


#define DIR_PRINT() \
PRINT_SCREEN(); \
libmPrintf(15,24,FG_COLOR,BG_COLOR, titleLabel); \
libmPrintf(15,32,FG_COLOR,BG_COLOR," [%s] [%d] ",currentPath,dir_num); \
libmPrintf(5,264,FG_COLOR,BG_COLOR, (selectType == 0)?PPREFSMSG_ADD_HOWTOUSE:PPREFSMSG_ADD_HOWTOUSE_2, buttonData[buttonNum[0]].name); \


#define CONTINUE_UPDOWN(flag,before,time,wait) \
if( (before & (flag)) == (flag) ){ \
	if( (sceKernelLibcClock() - time) >= (wait) ){ \
		time = sceKernelLibcClock(); \
	}else{ \
		continue; \
	} \
}else{ \
	before  = flag; \
	time = sceKernelLibcClock(); \
} \


#define SAFELY_SUSPEND() \
if( ! now_state ){ \
	if( (sceKernelLibcClock() - safely_suspend_time) >= (5 * 100 * 1000) ){ \
		SUSPEND_THREADS(); \
	} \
} \


#define SAFELY_SUSPEND_INIT() \
safely_suspend_time = sceKernelLibcClock(); \


/*---------------------------------------------------------------------------*/

void selectStrage(char *path)
{
	
	int num = 0;
	
	char *menu_fat[] = {
		"ms0:/",
		"disk0:/",
		NULL
	};
	char *menu_fat_hitobashira[] = {
		"ms0:/",
		"disk0:/",
		"flash0:/",
		"flash1:/",
		"flash2:/",
		"flash3:/",
		NULL
	};
	char *menu_go[] = { 
		"ms0:/",
		"ef0:/",
		"fatms0:/",
		"disk0:/",
		NULL
	};
	char *menu_go_hitobashira[] = { 
		"ms0:/",
		"ef0:/",
		"fatms0:/",
		"disk0:/",
		"flash0:/",
		"flash1:/",
		"flash2:/",
		"flash3:/",
		"eh0:/",
		"isofs0:/",
		NULL
	};
	char **menu;
	
	
	if( deviceModel == 4 ){
		if( hitobashiraFlag ){
			menu = menu_go_hitobashira;
		}else{
			menu = menu_go;
		}
		/*
		 6.20TNのパスのエイリアス機能対策(?)
		 goで(vshのときだけ?)ef0:/からプラグインを読み込ませると"ms0:/"という文字列を"ef0:/"と書き換えるっぽいので、
		 更に上書きしてやる、、、けど文字列リテラルって本当は書き換えたらまずいよね・・・
		 */
		menu[0][0] = 'm';
		menu[0][1] = 's';
		menu[2][3] = 'm';
		menu[2][4] = 's';
	}else{
		if( hitobashiraFlag ){
			menu = menu_fat_hitobashira;
		}else{
			menu = menu_fat;
		}
	}
	
	num = pprefsMakeSelectBox(8, 8, "SELECT STORAGE",menu, buttonData[buttonNum[0]].flag, 1 );
	
	if( num >= 0 ){
		strcpy(path,menu[num]);
	}
	
}

int fileSelecter(const char *startPath, dir_t *rtn, char* titleLabel,int selectType )
{
	int i;
	u32 beforeButtons = 0;
	clock_t time = 0;
	
	if( currentPath[0] == '\0' ){
		strcpy(currentPath,config.startPath);
		if( currentPath[strlen(currentPath)-1] != '/' ) strcat(currentPath,"/");
		DIR_READ();
	}

	DIR_PRINT();
	
	SAFELY_SUSPEND_INIT();
	while(1){


		//描画
		libmFillRect( 0 , 46 , 480 , 46 + MAX_DISPLAY_NUM*(LIBM_CHAR_HEIGHT+2),BG_COLOR );
		if( dir_num != 0 ){
			for( i = 0; i < dir_num && i < MAX_DISPLAY_NUM; i++ ){
				if( dir_buf[i+dir_print_position].type == TYPE_DIR )
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s/",dir_buf[i+dir_print_position].name);
				else
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dir_buf[i+dir_print_position].name);
			}
			libmPrintf(5,46 + dir_now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		}
		
		//ボタンgetと処理
		while(1){
			SAFELY_SUSPEND();
			
			get_button(&padData);
			
			if( padData.Buttons & PSP_CTRL_DOWN )
			{
				CONTINUE_UPDOWN(PSP_CTRL_DOWN,beforeButtons,time,2 * 100 * 1000);
				
				
				if( dir_now_arrow + 1 < MAX_DISPLAY_NUM && dir_now_arrow + 1 < dir_num ){
					libmPrintf(5,46 + dir_now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					dir_now_arrow++;
					libmPrintf(5,46 + dir_now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( dir_print_position+MAX_DISPLAY_NUM < dir_num ) dir_print_position++;
					break;
				}
			}
			else if( padData.Buttons & PSP_CTRL_UP )
			{
				CONTINUE_UPDOWN(PSP_CTRL_UP,beforeButtons,time,2 * 100 * 1000);
				
				if( dir_now_arrow - 1 >= 0 ){
					libmPrintf(5,46 + dir_now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					dir_now_arrow--;
					libmPrintf(5,46 + dir_now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( dir_print_position > 0 ) dir_print_position--;
					break;
				}
			}
			else if( padData.Buttons & (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER) && dir_num != 0 )
			{
				beforeButtons = (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER);

				//選択されたものがフォルダ
				if( dir_buf[dir_print_position+dir_now_arrow].type == TYPE_DIR ){
					wait_button_up(&padData);

					strcat(currentPath,dir_buf[dir_print_position+dir_now_arrow].name);
					strcat(currentPath,"/");
					DIR_READ();
					DIR_PRINT();
					break;
				//選択されたものがフォルダではない && buttonData[buttonNum[0]].flagボタンが押されている
				}else if( (padData.Buttons & buttonData[buttonNum[0]].flag) ){
					wait_button_up(&padData);

					*rtn = dir_buf[dir_print_position+dir_now_arrow];
					strcpy( rtn->name , currentPath );
					strcat( rtn->name , dir_buf[dir_print_position+dir_now_arrow].name );
					return 0;
				}
			}
			else if( padData.Buttons & PSP_CTRL_LTRIGGER )
			{
				beforeButtons = PSP_CTRL_LTRIGGER;

				wait_button_up(&padData);
				if( currentPath[strlen(currentPath) - 2] == ':' ){
					selectStrage(currentPath);
					DIR_READ();
					DIR_PRINT();
					break;
				}else if( up_dir(currentPath) >= 0 ){
					DIR_READ();
					DIR_PRINT();
					break;
				}
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				wait_button_up(&padData);
				return 1;
			}
			else if( selectType == 1 && padData.Buttons & PSP_CTRL_START )
			{
				wait_button_up(&padData);

				strcpy( rtn->name , currentPath );
				rtn->type = TYPE_DIR;
				rtn->sort_type = dir_type_sort_default[TYPE_DIR];
				return 0;
			}
			else
			{
				beforeButtons = 0;
			}
		}
	}
}


int main_menu()
{
	wait_button_up(&padData);

	SceUID fd;
	int i,printState;
	u32 beforeButtons;
	clock_t timesec;
	SceIoStat stat;

	//画面初期化
	libmInitBuffers(false,PSP_DISPLAY_SETBUF_NEXTFRAME);
	libmClearBuffers();
	libmFillRect( 0 , 0 , 480 , 272 , BG_COLOR); 

	/*
	//バッファ確保
	if( ptxtBuf == NULL ){
		ptxtBuf = malloc(PTXTBUF_SIZE);
		if( ptxtBuf == NULL ) return -1;
	}
	*/



	while(1){
		
		//ファイルセレクト
		if( now_display_flag == 0){
			if( ptxtBuf_more != NULL ){
				free(ptxtBuf_more);
				ptxtBuf_more = NULL;
			}

			if( fileSelecter(config.startPath, &now_open_file, PTXTMSG_SELECTER_TITLE, 0) != 0 ){
				return 0;
			}

			SUSPEND_THREADS();
			
			sceIoGetstat(now_open_file.name, &stat);
			fd = sceIoOpen(now_open_file.name, PSP_O_RDONLY, 0777);
			if( fd < 0 ){
				return -2;
			}
			
			ptxt_readSize = sceIoRead(fd, ptxtBuf, PTXTBUF_SIZE - 1);
			if( ptxt_readSize == PTXTBUF_SIZE - 1){
				if( ptxtBuf_more == NULL ){
					ptxtBuf_more = malloc(stat.st_size - ptxt_readSize + 1);
					if( ptxtBuf_more == NULL ) return -1;
				}
				
				ptxt_readSize_more = sceIoRead(fd, ptxtBuf_more, stat.st_size - ptxt_readSize );
				ptxtBuf_more[ptxt_readSize_more] = '\0';
				ptxt_readSize_more = remove_r(ptxtBuf_more);
			}

			ptxtBuf[ptxt_readSize] = '\0';
			ptxt_readSize = remove_r(ptxtBuf);

			sceIoClose(fd);
			if( ptxt_readSize < 0 ){
				return -3;
			}
			
			ptxt_headOffset = 0;
			now_display_flag = 1;
		}
		
		
		
		

		PRINT_SCREEN();
		libmFillRect( 0 , 15 , 480 , 16 ,FG_COLOR );
		libmFillRect( 0 , 262 , 480 , 263 ,FG_COLOR );
		libmPrintf(0, 264, FG_COLOR,BG_COLOR, PTXTMSG_VIEW_HOTOUSE,  buttonData[buttonNum[1]].name);
		
		beforeButtons = 0;
		timesec = 0;
		SAFELY_SUSPEND_INIT();
		while(1){
			
			libmFillRect( 0 , 17 , 480 , 261 , BG_COLOR); 
			printState = printText( ptxt_headOffset, MAXDISPLAY_Y );
			libmPrintf(0, 264, FG_COLOR,BG_COLOR, "readSize:%d readSize_more:%d moreIs:%s usemoreIs:%s  ",ptxt_readSize,ptxt_readSize_more,ptxtBuf_more?"true":"NULL",(printState & USE_MOREBUF)?"true":"false"  );

			
			if( beforeButtons == 0 ) wait_button_up(&padData);
			while(1){
				SAFELY_SUSPEND();
		
				get_button(&padData);
		
				if( padData.Buttons & PSP_CTRL_UP && ptxt_headOffset != 0 )
				{
					if( padData.Buttons & PSP_CTRL_SQUARE ){
						CONTINUE_UPDOWN(PSP_CTRL_UP|PSP_CTRL_SQUARE,beforeButtons,timesec,5 * 1 * 1000);
					}else{
						CONTINUE_UPDOWN(PSP_CTRL_UP,beforeButtons,timesec,2 * 100 * 1000);
					}
					
					ptxt_headOffset = getOffsetOfPreviousLine(ptxtBuf,ptxt_headOffset);
					break;
				}
				else if( padData.Buttons & PSP_CTRL_DOWN && !(printState & TAIL_IS_EOF) )
				{
					if( padData.Buttons & PSP_CTRL_SQUARE ){
						CONTINUE_UPDOWN(PSP_CTRL_DOWN|PSP_CTRL_SQUARE,beforeButtons,timesec,5 * 1 * 1000);
					}else{
						CONTINUE_UPDOWN(PSP_CTRL_DOWN,beforeButtons,timesec,2 * 100 * 1000);
					}
					
					ptxt_headOffset = getOffsetOfNextLine(ptxtBuf,ptxt_headOffset);
					
					
					break;
				}
				else if( padData.Buttons & PSP_CTRL_LEFT )
				{
					beforeButtons = PSP_CTRL_LEFT;
					for( i = 0; i < MAXDISPLAY_Y; i++ ){
						if( ptxt_headOffset == 0 ) break;
						ptxt_headOffset = getOffsetOfPreviousLine(ptxtBuf,ptxt_headOffset);
					}
					wait_button_up(&padData);
					break;
				}
				else if( padData.Buttons & PSP_CTRL_RIGHT )
				{
					beforeButtons = PSP_CTRL_RIGHT;
					for( i = 0; i < MAXDISPLAY_Y; i++ ){
						if( printState & TAIL_IS_EOF ) break;
						ptxt_headOffset = getOffsetOfNextLine(ptxtBuf,ptxt_headOffset);
						printState = noPrintText( ptxt_headOffset, MAXDISPLAY_Y);
					}
					wait_button_up(&padData);
					break;
				}
				else if( padData.Buttons & buttonData[buttonNum[1]].flag )
				{
					wait_button_up(&padData);
					now_display_flag = 0;
					break;
				}
				else if( padData.Buttons & PSP_CTRL_HOME )
				{
					wait_button_up(&padData);
					return 0;
				}
				else if( padData.Buttons & PSP_CTRL_RTRIGGER )
				{
					beforeButtons = PSP_CTRL_RTRIGGER;
					
					while(1){
						if( printState & TAIL_IS_EOF ) break;
						ptxt_headOffset = getOffsetOfNextLine(ptxtBuf,ptxt_headOffset);
						printState = noPrintText( ptxt_headOffset, MAXDISPLAY_Y);
					}
					
					wait_button_up(&padData);
					break;
				}
				else if( padData.Buttons & PSP_CTRL_LTRIGGER )
				{
					beforeButtons = PSP_CTRL_LTRIGGER;
					
					ptxt_headOffset = 0;
					
					wait_button_up(&padData);
					break;
				}
				else
				{
					beforeButtons = 0;
				}
				
			}
			if( now_display_flag == 0 ) break;
	}

	}
	
	return 0;
}


int main_thread( SceSize arglen, void *argp )
{
	
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
	

	Read_Conf(argp,&config);
	currentPath[0] = '\0';
	
	deviceModel = sceKernelGetModel();
	if( deviceModel < 0 || deviceModel > 8){
		deviceModel = 9;
	}
	
	padData.Buttons = 0;
	
	
	while( stop_flag ){
		sceKernelDelayThread( 50000 );
		sceCtrlPeekBufferPositive( &padData, 1 );
		if((padData.Buttons & config.bootKey) == config.bootKey){
			main_menu();
			RESUME_THREADS();
		}
	}

  return 0;
}


int module_start( SceSize arglen, void *argp )
{
	
	Get_FirstThreads();
	
	SceUID thid;
	
	//umd dumpとは逆で flag == 0 の時にストップする仕様
	stop_flag = 1;
	thid = sceKernelCreateThread( "PTEXTVIEWER", main_thread, 30, 0x6000, PSP_THREAD_ATTR_NO_FILLSTACK, 0 );
	if( thid ) sceKernelStartThread( thid, arglen, argp );

  return 0;
}

int module_stop( void )
{
	stop_flag = 0;
//	if( ptxtBuf != NULL ) free(ptxtBuf);
	return 0;
}













