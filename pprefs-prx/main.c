/*
	
	���肪�Ƃ��A
	maxem����Atakka����(�A���t�@�x�b�g��)

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
#include "iniconfig.h"
#include "button.h"

// ���W���[���̒�`
PSP_MODULE_INFO( "PLUPREFS", PSP_MODULE_KERNEL, 0, 0 );



#define INI_PATH "/pprefs.ini"

#define FG_COLOR WHITE
#define BG_COLOR BLACK

#define LEN_PER_LINE 256
#define MAX_LINE 21



int readConfig(const char *file_name);


int module_start( SceSize arglen, void *argp );
int module_stop( void );

int main_thread( SceSize arglen, void *argp );
void main_menu(void);

int read_line_file(SceUID fp, char* line, int num);

int removeAnItem(int type,int num);
int addNewItem(int type,char *str);

int readSepluginsText( int ptype );
int writeSepluginsText(int ptype);

int copyMeProcess(void);

//dir_t *dirBuf = NULL;
dir_t dirBuf[128];

int stop_flag;


SceCtrlData padData;


int buttonNumBuf[2];
int *buttonNum = buttonNumBuf;

struct {
	unsigned int flag;
	char *name;
}buttonData[] = {
	{PSP_CTRL_CROSS,"�~"},
	{PSP_CTRL_CIRCLE,"��"}
};


#define COMMON_BUF_LEN 256
char commonBuf[COMMON_BUF_LEN];

#define libmPrintf(x,y,fg,bg,format, ... ) libmPrintf(x,y,fg,bg,commonBuf,COMMON_BUF_LEN,format, ##__VA_ARGS__)


int now_type = 0;


//�����̕ϐ��͂ЂƂ̍\���̂ɂ܂Ƃ߂�ׂ����E�E�E
char *sepluginsTextPath[] = {
	"ms0:/seplugins/vsh.txt",
	"ms0:/seplugins/game.txt",
	"ms0:/seplugins/pops.txt"
};

struct {
	int num;//number of structure of line
	bool exist;
	bool edit;
	struct{
		char path[LEN_PER_LINE];
		bool toggle;//= ture ON / = false OFF
	}line[MAX_LINE];
} pdata[3];






char ownPath[256];

void saveEditing(void){
	int i;

//	check_ms();

	for( i = 0; i < 3; i++ ){
		if ( pdata[i].edit ){
			while(1){
				if( writeSepluginsText(i) < 0 ){

					makeWindow(
						24 , 28 ,
						24 + LIBM_CHAR_WIDTH*26 , 28 + LIBM_CHAR_HEIGHT*5,
						FG_COLOR,BG_COLOR
					);
					libmPrintf( 24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"%s",sepluginsTextPath[i]);
					libmPrint(  24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"�̏������݂Ɏ��s���܂���");
					libmPrintf(  24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:���g���C %s:�X�L�b�v ",buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);

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
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

int displayBootMassageThread( SceSize arglen, void *argp ){
	double timesec = gettimeofday_sec();
	unsigned int *key = (unsigned int *)argp;
	char *temp;
	char button[256] = "";


	if( *key & PSP_CTRL_SELECT ) strcat(button,"SELECT + ");
	if( *key & PSP_CTRL_START ) strcat(button,"START + ");
	if( *key & PSP_CTRL_UP ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_RIGHT ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_DOWN ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_LEFT ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_LTRIGGER ) strcat(button,"[L] + ");
	if( *key & PSP_CTRL_RTRIGGER ) strcat(button,"[R] + ");
	if( *key & PSP_CTRL_TRIANGLE ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_CIRCLE ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_CROSS ) strcat(button,"�~ + ");
	if( *key & PSP_CTRL_SQUARE ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_NOTE ) strcat(button,"�� + ");
	if( *key & PSP_CTRL_HOME ) strcat(button,"HOME + ");

	temp = strrchr(button, '+');
	if( temp != NULL ){
		temp[-1] = '\0';
	}
	
	while( stop_flag & 2 ){
		if( libmInitBuffers(false,PSP_DISPLAY_SETBUF_NEXTFRAME) ){
			libmPrintf(0,264,SetAlpha(WHITE,0xFF),SetAlpha(BLACK,0xFF)," pprefs �N����������! / �N���{�^��:%s ",button);
			sceDisplayWaitVblankStart();
		}
		sceKernelDelayThread( 1000 );
		if( (gettimeofday_sec() - timesec) >= 8 ) stop_flag &= ~2;
	}
	
	sceKernelExitDeleteThread(0);
	return 0;
}

int main_thread( SceSize arglen, void *argp )
{
	unsigned int key;
	char *temp;
	char path[256];
	SceUID thid;
	

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
	strcpy(path, argp);
	temp = strrchr(path, '/');
	temp[0] = '\0';
	strcat(path, INI_PATH);
	key = readConfig(path);
	
	stop_flag |= 2;
	thid = sceKernelCreateThread( "PPREFS_DISPLAY_BOOT_MESSAGE", displayBootMassageThread, 31, 0x6000, PSP_THREAD_ATTR_CLEAR_STACK /*PSP_THREAD_ATTR_NO_FILLSTACK*/, 0 );
	if( thid ) sceKernelStartThread( thid, sizeof(key), &key );

	
	while( stop_flag ){
		sceKernelDelayThread( 50000 );
		//    sceCtrlPeekBufferPositive( &data, 1 );
		//    padData.Buttons ^= XOR_KEY;
		get_button( &padData);
		if((padData.Buttons & key) == key){
			stop_flag &= ~2;
			main_menu();
		}

	}

  return 0;
}

#define MAX_DISPLAY_NUM 21

#define PRINT_SCREEN() \
libmClearBuffers(); \
libmPrint(10,10,FG_COLOR,BG_COLOR,"pprefs Ver. 1.00   by hiroi01");



int file_selecter(void){
	int dir_num,offset,i,now_arrow;
	char currentPath[256] = "ms0:/seplugins/";

	
	while(1){

		
		dir_num = read_dir(dirBuf,currentPath, 0);
		offset = 0;
		now_arrow = 0;

		PRINT_SCREEN();
		libmPrintf(15,28,BG_COLOR,FG_COLOR," <<�ǋL>> : �ǋL�������v���O�C����I�����Ă������� ");
		libmPrintf(15,36,BG_COLOR,FG_COLOR," [%s] [%d] ",currentPath,dir_num);
		libmPrintf(5,264,FG_COLOR,BG_COLOR," %s:�I�� HOME:��߂� L:�t�H���_�K�w��� R:�t�H���_�J�� ",buttonData[buttonNum[0]].name);

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
				//����̃t�H���_�[�ł͂Ȃ�
				if(  dir_num != 0 ){
					//�I�����ꂽ���̂��t�H���_�[
					if( dirBuf[offset+now_arrow].type == TYPE_DIR ){
						strcat(currentPath,dirBuf[offset+now_arrow].name);
						strcat(currentPath,"/");
						wait_button_up(&padData);
						break;
					//�I�����ꂽ���̂��t�H���_�[�ł͂Ȃ� && buttonData[buttonNum[0]].flag�{�^����������Ă���
					}else if( (padData.Buttons & buttonData[buttonNum[0]].flag) && dir_num != 0 ){
						strcat(currentPath,dirBuf[offset+now_arrow].name);
						addNewItem(now_type,currentPath);
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


return: �ǂ̃��j���[�����s���ꂽ��
(0 < : �G���[)
 0 = : �������s����Ȃ�����
 1 = : ���j���[No.1�����s���ꂽ
 2 = : ���j���[No.2�����s���ꂽ
�E
�E
�E
*/


int editTextMenu(int currentSelected,int position){
	int i,now_arrow;
	int menunum = 3;
	char *menu[] ={
		"�ǋL",
		"�폜",
		"COPY ME"
	};

	now_arrow = 0;

	makeWindow( 8 , position , 8 + LIBM_CHAR_WIDTH*12 , position + LIBM_CHAR_HEIGHT+2 + (LIBM_CHAR_HEIGHT+2) * menunum + 2 ,FG_COLOR,BG_COLOR);

	//�ŉ��̍s�̕\���������ď�������
	libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);
	libmPrintf(5,264,FG_COLOR,BG_COLOR," %s:�I�� %s:�߂� ",buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);


	libmPrint( 8 + LIBM_CHAR_WIDTH  , position - 2 ,FG_COLOR,BG_COLOR,"���j���[");
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
		}else if( padData.Buttons & PSP_CTRL_UP ){
			libmFillRect( 8 + LIBM_CHAR_WIDTH , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) , 8 + LIBM_CHAR_WIDTH*2 , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) + LIBM_CHAR_HEIGHT , BG_COLOR );
			if( now_arrow - 1 >= 0 ){
				now_arrow--;
			}else{
				now_arrow = menunum - 1;
			}
			libmPrint( 8 + LIBM_CHAR_WIDTH , position + LIBM_CHAR_HEIGHT+2 + now_arrow*(LIBM_CHAR_HEIGHT+2) , FG_COLOR , BG_COLOR , ">");
		}else if( padData.Buttons & buttonData[buttonNum[0]].flag ){
			wait_button_up(&padData);
			break;
		}else if( padData.Buttons & (PSP_CTRL_HOME|buttonData[buttonNum[1]].flag) ){
			wait_button_up(&padData);
			return 0;
		}
		wait_button_up(&padData);
	}

	if( now_arrow == 0 ){
		file_selecter();
	}else if( now_arrow == 1 ){
		removeAnItem(now_type,currentSelected);
		pdata[now_type].edit = true;
	}else if( now_arrow == 2 ){
		int tmp = copyMeProcess();
		if( tmp < 0 ){
			makeWindow(
				24 , 28 ,
				 24 + LIBM_CHAR_WIDTH*23 , 28 + LIBM_CHAR_HEIGHT*5,
				 FG_COLOR,BG_COLOR
			);
			libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"�G���[���������܂���");
			if( tmp == -1 )
				libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"pprefs�������Ă���MS�����Ă�������");
			else
				libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"ErrorNo.:%d",tmp);
			libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:OK ",buttonData[buttonNum[0]].name);
			wait_button_down(&padData,buttonData[buttonNum[0]].flag);
		}
		if( tmp !=  1 ) readSepluginsText(3);
	}
	
	return now_arrow + 1;
	
}


void main_menu(void)
{
	// wait till releasing buttons
	wait_button_up(&padData);

	// suspend XMB
	libmExecTCmd(LIBM_TCMD_SUSPEND);

	//prepare for displaying and display
	libmInitBuffers(false,PSP_DISPLAY_SETBUF_NEXTFRAME);
	PRINT_SCREEN();

	int i;
	int now_arrow = 0;

	char *typeName[] = {
		"vsh ",
		"game",
		"pops"
	};
	
	

	readSepluginsText(3);

	while(1){
		PRINT_SCREEN();
		libmPrintf(0,264,FG_COLOR,BG_COLOR," %s:�I�� SELECT:�ҏW�j��&�����[�h ��/START:���j���[ HOME:�ۑ�&�I�� ",buttonData[buttonNum[0]].name);
		libmPrintf(15,28,BG_COLOR,FG_COLOR,"<<[L]  %s [R]>>",typeName[now_type]);
		if( pdata[now_type].edit ) libmPrint(63 , 28 , BG_COLOR , FG_COLOR,"*");
		libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		for( i = 0; i < pdata[now_type].num; i++ ){
			libmPrintf(15,38 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,
			"[%s] %s",pdata[now_type].line[i].toggle?"O N":"OFF",pdata[now_type].line[i].path);
		}
		
		
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_DOWN && pdata[now_type].num > 0 ){
				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				if( now_arrow + 1 < pdata[now_type].num ){
					now_arrow++;
				}else{
					now_arrow = 0;
				}
				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
			}else if( padData.Buttons & PSP_CTRL_UP && pdata[now_type].num > 0 ){
				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				if( now_arrow - 1 >= 0 ){
					now_arrow--;
				}else{
					now_arrow = pdata[now_type].num - 1;
				}
				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				
			}else if( padData.Buttons & buttonData[buttonNum[0]].flag && pdata[now_type].num > 0 ){
				pdata[now_type].edit = true;
				libmPrint(63 , 28 , BG_COLOR , FG_COLOR,"*");
				pdata[now_type].line[now_arrow].toggle = !pdata[now_type].line[now_arrow].toggle;
				libmPrintf(15,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,
				"[%s] %s",pdata[now_type].line[now_arrow].toggle?"O N":"OFF",pdata[now_type].line[now_arrow].path);
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
			}else if( padData.Buttons & ( PSP_CTRL_TRIANGLE | PSP_CTRL_START ) ){
				if( editTextMenu(now_arrow,( now_arrow < 10 )?148:46) != 0 ){
					
					now_arrow = 0;
				}
				wait_button_up(&padData);
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				wait_button_up(&padData);
				saveEditing();

				wait_button_up(&padData);
				
				// resume XMB
				libmExecTCmd(LIBM_TCMD_RESUME);
				return;
			}else if( padData.Buttons & PSP_CTRL_SELECT ){
				wait_button_up(&padData);
				
				libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);
				makeWindow(8 , 28 , 8 + LIBM_CHAR_WIDTH*18 , 28 + LIBM_CHAR_HEIGHT*5,FG_COLOR,BG_COLOR);
				libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"�ҏW��j�����āA");
				libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"�����[�h���Ă���낵���ł���?");
				libmPrintf(8 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:�͂� %s:������",buttonData[buttonNum[0]].name,buttonData[buttonNum[1]].name);
				while(1){
					get_button(&padData);
					if( padData.Buttons & buttonData[buttonNum[0]].flag ){
						wait_button_up(&padData);
						readSepluginsText(3);
						break;
					}else if( padData.Buttons & (buttonData[buttonNum[1]].flag | PSP_CTRL_HOME) ){
						wait_button_up(&padData);
						break;
					}
				}
				
				wait_button_up(&padData);
				break;
			}
			wait_button_up(&padData);
		}
	}

}


int module_start( SceSize arglen, void *argp )
{
	libmExecTCmd(LIBM_TCMD_DUMP);
	SceUID thid;
	
//	while(1){
//		dirBuf = (dir_t *)memoryAlloc( sizeof(dir_t) * 128 );
//		if( dir != NULL ) break;
//		sceKernelDelayThread(10000);
//	}
	
	//umd dump�Ƃ͋t�� flag == 0 �̎��ɃX�g�b�v����d�l
	stop_flag = 1;
	thid = sceKernelCreateThread( "PPREFS", main_thread, 30, 0x6000, PSP_THREAD_ATTR_NO_FILLSTACK, 0 );
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
  // �G���[�̏ꍇ / on error
  if(len == 0)
    return -1;

  end = strchr(buff, '\n');

  // \n��������Ȃ��ꍇ / not found \n
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

/*
	remove an item from pdata

	@param : type
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	@param : num
	removing data number

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
int addNewItem(int type,char *str){
	if( !(0 <= type &&  type <= 2) ) return -1;
	if( !(pdata[type].num < MAX_LINE) ) return -2;

	strcpy(pdata[type].line[pdata[type].num].path,str);
	pdata[type].line[pdata[type].num].toggle = false;
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
	FILE *fp;

	
	check_ms();


	fp = fopen(sepluginsTextPath[type],"w");
	if( fp == NULL  ) return (type+1);

	for( i = 0; i < pdata[type].num; i++ ){
		fprintf(fp,"%s %c\n",
			pdata[type].line[i].path,
			pdata[type].line[i].toggle?'1':'0'
		);
	}
	pdata[type].edit = false;
	fclose(fp);

	return 0;
}

/*
	@param : ptype 
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	= 3 all
	@return : 
	= 0 no problem
	< 0 on error
*/

int readSepluginsText( int ptype ){

	SceUID fp;
	int type,i,readSize,loopend;
	char line[LEN_PER_LINE],*ptr;

	if( 0 <= ptype &&  ptype <= 2 ){
		type = ptype;
		loopend = ptype + 1;
	}else if( ptype == 3 ){
		type = 0;
		loopend = 3;
	}else{
		 return -1;
	}
	
	check_ms();
	
	for( ; type < loopend; type++){
		fp = sceIoOpen(sepluginsTextPath[type], PSP_O_RDONLY, 0777);
		if( fp < 0 ){
			pdata[type].exist = false;
			pdata[type].num = 0;
			pdata[type].edit = false;
			continue;
		}else{
			pdata[type].exist = true;
		}

		i = 0;
		while( i < MAX_LINE ){
			readSize = read_line_file(fp,line,LEN_PER_LINE - 1);
			if( readSize < 0 ){
				break;
			}
			if( line[0] == '\0' || line[0] == ' ' ){
				continue;
			}else if( (ptr = strchr(line,' ')) == NULL){// ' '��������Ȃ��Ȃ� /  if ' ' is not found
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
		pdata[type].num = i;
		sceIoClose(fp);
		pdata[type].edit = false;
	}
	return 0;
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





int copyMeProcess(void){
	libmFillRect(0 , 264 , 480 , 272 ,BG_COLOR);

	if( pdata[0].edit || pdata[1].edit || pdata[2].edit ){
		makeWindow(
			24 , 28 ,
			24 + LIBM_CHAR_WIDTH*32 , 28 + LIBM_CHAR_HEIGHT*6,
			FG_COLOR,BG_COLOR
		);
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*1 , FG_COLOR,BG_COLOR,"\"COPY ME\"�����s����O�Ɍ��݂̕ҏW��ۑ����Ă�������");
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3, FG_COLOR,BG_COLOR,"START:�ۑ����đ����� SELECT:�ҏW�j����������");
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*4+ 2 ,FG_COLOR,BG_COLOR,"HOME:\"COPY ME\"����߂�");
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_START ){
				saveEditing();
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

	check_ms();

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
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"2���ڂ̃������[�X�e�B�b�N��");
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*2 + 2 , FG_COLOR,BG_COLOR,"����Ă�������");
		libmPrintf(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT*3 + 4 ,FG_COLOR,BG_COLOR,"%s:���ꂽ HOME:�R�s�[��߂�",buttonData[buttonNum[0]].name);
		while(1){
			get_button(&padData);
			if( padData.Buttons & buttonData[buttonNum[0]].flag ){
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				return 2;
			}
		}
		wait_button_up(&padData);
		if( !(check_ms() < 0) ) break;
	}
	
	sceIoMkdir("ms0:/seplugins",0777);
	

	if( (fd = sceIoOpen("ms0:/seplugins/pprefs.prx",PSP_O_WRONLY | PSP_O_CREAT,0777)) < 0 ){
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
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"�R�s�[���������܂���");
	}else{
		libmPrint(24 + LIBM_CHAR_WIDTH , 28 + LIBM_CHAR_HEIGHT , FG_COLOR,BG_COLOR,"�R�s�[�Ɏ��s�����\\��������܂�");
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