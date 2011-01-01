/*
	
	ありがとう、
	maxemさん、takkaさん(アルファベット順)

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdisplay.h>


#include "main.h"
#include "libmenu.h"
#include "memory.h"
#include "file.h"
#include "button.h"
#include "thread.h"

// モジュールの定義
PSP_MODULE_INFO( "PTEXTVIEWER", PSP_MODULE_KERNEL, 0, 0 );



#define INI_PATH "/ptextviewer.ini"


#define LEN_PER_LINE 256
#define MAX_LINE 21







int module_start( SceSize arglen, void *argp );
int module_stop( void );

int main_thread( SceSize arglen, void *argp );
void main_menu(void);

int read_line_file(SceUID fp, char* line, int num);

void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);


//dir_t *dirBuf = NULL;
dir_t dirBuf[128];

int stop_flag;


SceCtrlData padData;

/*
int buttonNumBuf[2];
 = buttonNumBuf;
*/

struct {
	unsigned int flag;
	char *name;
}buttonData[] = {
	{PSP_CTRL_CROSS,"×"},
	{PSP_CTRL_CIRCLE,"○"}
};


int buttonNum[2] = {0,1};

#define COMMON_BUF_LEN 256
char commonBuf[COMMON_BUF_LEN];

#define libmPrintf(x,y,fg,bg,format, ... ) libmPrintf(x,y,fg,bg,commonBuf,COMMON_BUF_LEN,format, ##__VA_ARGS__)



char ownPath[256];

char currentPath[256] = "ms0:/";



int main_thread( SceSize arglen, void *argp )
{
	unsigned int key;

	

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
	
//	Get_FirstThreads();


	strcpy(ownPath, argp);

	key = PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER | PSP_CTRL_NOTE;

	
	while( stop_flag ){
		sceKernelDelayThread( 50000 );
		sceCtrlPeekBufferPositive( &padData, 1 );
		if((padData.Buttons & key) == key){
			main_menu();
		}

	}

  return 0;
}

#define MAX_DISPLAY_NUM 21

#define PRINT_SCREEN() \
libmFillRect( 0 , 0 , 480 , 272 , BG_COLOR); \
libmPrint(13,5,FG_COLOR,BG_COLOR,"ptextviewer β Ver. 0.01   by hiroi01");

#define fillLine(sy,color) libmFillRect( 0 , sy , 480 , sy + LIBM_CHAR_HEIGHT ,color);


// http://katsura-kotonoha.sakura.ne.jp/prog/c/tip00010.shtml

#define jms1(c) \
(((0x81 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0x9F)) || ((0xE0 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC) ))

#define jms2(c) \
((0x7F != (unsigned char)(c)) && (0x40 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC))

#define MAX_LINE_NUM 28


void text_viewer(char *path)
{
	SceUID fd;
	int readSize,x_count,y_count,i;
	char buf[129];
	char tmp[3];// 60文字 * 2byte文字 + \0 = 121
	

	
	PRINT_SCREEN();
	libmPrintf(5,264,FG_COLOR,BG_COLOR," HOME:終了 ",buttonData[buttonNum[0]].name);
	libmPrint(5,20,FG_COLOR,BG_COLOR,path);
	libmFillRect( 0 , 30 , 480 , 32 ,FG_COLOR);
	
	
	
	fd = sceIoOpen(path,PSP_O_RDONLY, 0777);
	if( fd < 0 ){
		libmPrint( 0 , 38 , FG_COLOR , BG_COLOR ,"ファイルの読み込みに失敗しました");
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_HOME ){
				return;
			}
		}
	}
	
	x_count = 0; y_count = 0;
	while(1){
		
		readSize = sceIoRead( fd, buf, 128 );
		if( readSize <= 0 ) break;

		i = 0;
		while(  i < readSize  ){
			
			//2バイト文字の1文字目
			if( jms1(buf[i]) ){
				if( !( (i + 1) < readSize ) ){
					sceIoLseek(fd, -1, SEEK_CUR);
					break;
				}else{
					tmp[0] = buf[i];
					tmp[1] = buf[i+1];
					tmp[2] = '\0';
					i += 2;
				}
			//2バイト文字の2文字目 この条件に引っかかったらバグ
			}else if( buf[i] == '\n'){
				x_count = 0;
				y_count++;
				if( y_count >= MAX_LINE_NUM ) break;
				i++;
				continue;
			}else{
				tmp[0] = buf[i];
				tmp[1] = '\0';
				i++;
			}

			libmPrint( x_count*LIBM_CHAR_WIDTH , 35 + y_count*LIBM_CHAR_HEIGHT , FG_COLOR , BG_COLOR ,tmp);
			x_count++;
			if( x_count >= 60 ){
				x_count = 0;
				y_count++;
				if( y_count >= MAX_LINE_NUM ) break;
			}
		}
		if( y_count >= MAX_LINE_NUM ) break;
	}

	sceIoClose(fd);


	
	while(1){
		get_button(&padData);
		if( padData.Buttons & PSP_CTRL_HOME ){
			return;
		}
	}

}


int file_selecter(void){
	int dir_num,offset,i,now_arrow = 0;

	
	while(1){
		
		dir_num = read_dir(dirBuf,currentPath, 0);
		offset = 0;
		now_arrow = 0;

PRINT_ALL:
		PRINT_SCREEN();
		libmPrintf(5,20,FG_COLOR,BG_COLOR," [%s] [%d] ",currentPath,dir_num);
		libmFillRect( 0 , 30 , 480 , 32 ,FG_COLOR);

		libmPrintf(5,264,FG_COLOR,BG_COLOR," %s:選択 HOME:終了 L:フォルダ階層上へ R:フォルダ開く ",buttonData[buttonNum[0]].name);

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
				wait_button_up(&padData);
				if( now_arrow + 1 < MAX_DISPLAY_NUM && now_arrow + 1 < dir_num ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow++;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset+MAX_DISPLAY_NUM < dir_num ) offset++;
					goto PRINT_LIST;
				}
			}else if( padData.Buttons & PSP_CTRL_UP ){
				wait_button_up(&padData);
				if( now_arrow - 1 >= 0 ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow--;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset > 0 ) offset--;
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
						strcat( currentPath , dirBuf[offset+now_arrow].name );
						text_viewer(currentPath);
						up_dir(currentPath);
						wait_button_up(&padData);
						goto PRINT_ALL;
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



void main_menu(void)
{
	// wait till releasing buttons
	wait_button_up(&padData);

	// suspend XMB
	Suspend_Resume_Threads(SUSPEND_MODE);
	
	//prepare for displaying and display
	libmInitBuffers(false,PSP_DISPLAY_SETBUF_NEXTFRAME);
	libmClearBuffers();
	PRINT_SCREEN();

	file_selecter();

	// resume XMB
	Suspend_Resume_Threads(RESUME_MODE);
	return;

}


int module_start( SceSize arglen, void *argp )
{
	Get_FirstThreads();

	SceUID thid;
	
//	while(1){
//		dirBuf = (dir_t *)memoryAlloc( sizeof(dir_t) * 128 );
//		if( dir != NULL ) break;
//		sceKernelDelayThread(10000);
//	}
	
	//umd dumpとは逆で flag == 0 の時にストップする仕様
	stop_flag = 1;
	thid = sceKernelCreateThread( "PTEXTVIEW", main_thread, 30, 0x6000, PSP_THREAD_ATTR_NO_FILLSTACK, 0 );
	if( thid ) sceKernelStartThread( thid, arglen, argp );

  return 0;
}

int module_stop( void )
{
//	memoryFree(dir);
	stop_flag = 0;
	  return 0;
}



int read_line_file(SceUID fp, char* line, int num)
{
  char buff[num];
  char* end;
  int len;
  int tmp;

  tmp = 0;
  len = sceIoRead(fp, buff, num);
  // エラーの場合 / on error
  if(len == 0)
    return -1;

  end = strchr(buff, '\n');

  // \nが見つからない場合 / not found \n
  if(end == NULL)
  {
    buff[num - 1] = '\0';
    strcpy(line, buff);
    return len;
  }

  end[0] = '\0';
  if((end != buff) && (end[-1] == '\r'))
  {
    end[-1] = '\0';
    tmp = -1;
  }

  strcpy(line, buff);
  sceIoLseek(fp, - len + (end - buff) + 1, SEEK_CUR);
  return end - buff + tmp;
}







void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor){
	int nowx = sx,nowy = sy;
	while(1){
		nowx += 8;
		nowy += 8;
		if( nowx > ex ) nowx = ex;
		if( nowy > ey ) nowy = ey;
		libmFillRect(sx , sy , nowx , nowy , bgcolor );
		libmFrame(sx , sy , nowx ,nowy , fgcolor );
		if( nowx == ex && nowy == ey ) break;
		sceKernelDelayThread(8000);
	}

}


