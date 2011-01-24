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
#include "fileselecter.h"
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




/*------------------------------------------------------*/

// http://katsura-kotonoha.sakura.ne.jp/prog/c/tip00010.shtml
#define jms1(c) \
(((0x81 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0x9F)) || ((0xE0 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC) ))

#define jms2(c) \
((0x7F != (unsigned char)(c)) && (0x40 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC))


#define MAXDISPLAY_X 60
#define MAXDISPLAY_Y 28

#define PTXTBUF_SIZE (1024*5 + 1)

enum
{
	SELECTED_TOP		= 1,
	SELECTED_BOTTOM		= 2,
	TAIL_IS_EOF			= 4,
	TAIL_IS_N			= 8,
};

/*-----------------------------------------------------*/


int stop_flag;

char *ptxtBuf;






/*----------------------------------------------------------------------------*/
int module_start( SceSize arglen, void *argp );
int module_stop( void );

int main_thread( SceSize arglen, void *argp );







/*----------------------------------------------------------------------------*/


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
	int i;
	for( i = (str[offset-1] == '\n')?(offset-2):(offset-1); i >= 0 && str[i] != '\n' ; i-- );
	i++;
	return i;
}

int getOffsetOfPreviousLine(char *str,int offset)
{
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
	int ret = 1;
	char tmp[] = { str[0], str[1], '\0' };
	if( ! jms1(str[0]) ){//2byte文字じゃないなら
		tmp[2] = '\0';
		ret = 0;
	}
	libmPrint( x, y, fg, bg, tmp );
	return ret;
}


u16  printText(char *ptxtBuf, int headOffset, int end_y)
{
	int i,x_count = 0,y_count = 0;
	
	
	for( i = headOffset; ptxtBuf[i] != '\0'; i++ ){
		if( ptxtBuf[i] == '\n' ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
			continue;
		}
		
		if( pprefsPutChar(LIBM_CHAR_WIDTH*x_count ,36+LIBM_CHAR_HEIGHT*y_count ,FG_COLOR, BG_COLOR, &ptxtBuf[i])
		   == 1 ) i++; //2byte文字なら
		
		x_count++;
		if( x_count >= MAXDISPLAY_X ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
		}
	}
	
	return 0;
	
}


//buf, bufの始まりの位置, 始まりの行数, 現在の選択されている行数, yの最大数
u16  noPrintText(char *ptxtBuf, int headOffset, int lineCount, int selectedLine, int end_y)
{
	int i,x_count = 0,y_count = 0,ret = 0;
	
	//最上に表示されている行が選択されている
	if( selectedLine  == lineCount ) ret |= SELECTED_TOP;
	
	for( i = headOffset; ptxtBuf[i] != '\0'; i++ ){
		if( ptxtBuf[i] == '\n' ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
			lineCount++;
			continue;
		}
		
		//		if( pprefsPutChar(LIBM_CHAR_WIDTH*x_count ,36+LIBM_CHAR_HEIGHT*y_count ,(lineCount==selectedLine)?RED:FG_COLOR, BG_COLOR, &ptxtBuf[i])
		//		    == 1 ) i++; //2byte文字なら
		if(  jms1(ptxtBuf[i]) ){//2byte文字なら
			i++;
		}
		
		
		x_count++;
		if( x_count >= MAXDISPLAY_X ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
		}
	}
	
	if( lineCount == selectedLine ) ret |= SELECTED_BOTTOM; //最下に表示されている行が選択されている
	if( ptxtBuf[i] == '\0' ) ret |= TAIL_IS_EOF;
	else if( ptxtBuf[i] == '\n' ) ret |= TAIL_IS_N;
	
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



/*---------------------------------------------------------------------------*/
int main_menu()
{
	wait_button_up(&padData);


		
	SceUID fd;
	int readSize,headOffset,i;
	u32 beforeButtons;
	clock_t timesec;

	libmInitBuffers(false,PSP_DISPLAY_SETBUF_NEXTFRAME);

	libmClearBuffers();
	libmFillRect( 0 , 0 , 480 , 272 , BG_COLOR); 

	if( ptxtBuf == NULL ){
		ptxtBuf = malloc(PTXTBUF_SIZE);
		if( ptxtBuf == NULL ) return -1;
	}

SELECTFILE:


	if( fileSelecter(config.startPath,&dirTmp, PTXTMSG_SELECTER_TITLE, 0, NULL) != 0 ){
		RESUME_THREADS();
		return 0;
	}
	SUSPEND_THREADS();
	 
	fd = sceIoOpen(dirTmp.name, PSP_O_RDONLY, 0777);
	if( fd < 0 ){
		RESUME_THREADS();
		return -2;
	}
	readSize = sceIoRead(fd, ptxtBuf, PTXTBUF_SIZE - 1);
	sceIoClose(fd);
	if( readSize < 0 ){
		RESUME_THREADS();
		return -3;
	}
	ptxtBuf[readSize] = '\0';
	
	readSize = remove_r(ptxtBuf);
	
	headOffset = 0;
	beforeButtons = 0;
	timesec = 0;
	
	
	PRINT_SCREEN();
	libmPrint(24, 22,  FG_COLOR,BG_COLOR ,PTXTMSG_VIEW_TOP);
	libmPrint (0, 264, FG_COLOR,BG_COLOR, PTXTMSG_VIEW_HOTOUSE);
	
	
	while(1){
		
		
		libmFillRect( 0 , 31 , 480 , 263 , BG_COLOR); 

		printText(ptxtBuf, headOffset, MAXDISPLAY_Y );
		
		
		if( beforeButtons == 0 ) wait_button_up(&padData);
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_UP && headOffset != 0 ){
				if( beforeButtons & PSP_CTRL_UP ){
					if( (sceKernelLibcClock() - timesec) >= (2 * 100 * 1000) ){
						timesec = sceKernelLibcClock();
					}else{
						continue;
					}
				}else{
					beforeButtons = PSP_CTRL_UP;
					timesec = sceKernelLibcClock();
				}
				
				headOffset = getOffsetOfPreviousLine(ptxtBuf,headOffset);
				break;
			}else if( padData.Buttons & PSP_CTRL_DOWN ){
				if( beforeButtons & PSP_CTRL_DOWN ){
					if( (sceKernelLibcClock() - timesec) >= (2 *100 * 1000) ){
						timesec = sceKernelLibcClock();
					}else{
						continue;
					}
				}else{
					beforeButtons = PSP_CTRL_DOWN;
					timesec = sceKernelLibcClock();
				}
				
				headOffset = getOffsetOfNextLine(ptxtBuf,headOffset);
				
				
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				wait_button_up(&padData);
				goto SELECTFILE;//goto文って使っちゃだめかな?
			}else if( padData.Buttons & PSP_CTRL_RTRIGGER ){
				beforeButtons = PSP_CTRL_RTRIGGER;
				
				while(1){
					i = getOffsetOfNextLine(ptxtBuf,headOffset);
					if( i == headOffset ) break;
					headOffset = i;
				}
				
				wait_button_up(&padData);
				break;
			}else if( padData.Buttons & PSP_CTRL_LTRIGGER ){				
				beforeButtons = PSP_CTRL_LTRIGGER;
				
				headOffset = 0;
				
				wait_button_up(&padData);
				break;
			}else{
				beforeButtons = 0;
			}
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
	if( config.startPath[strlen(config.startPath)-1] != '/' ) strcat(config.startPath,"/");
	
	
	padData.Buttons = 0;
	
	
	while( stop_flag ){
		sceKernelDelayThread( 50000 );
		sceCtrlPeekBufferPositive( &padData, 1 );
		if((padData.Buttons & config.bootKey) == config.bootKey){
			main_menu();
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
	if( ptxtBuf != NULL ) free(ptxtBuf);
	return 0;
}













