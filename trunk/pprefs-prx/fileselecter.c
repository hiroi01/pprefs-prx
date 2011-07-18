#include "fileselecter.h"
#include "common.h"
#include "button.h"
#include "language.h"
#include "pprefsmenu.h"
#include "memory.h"
#include "charConv.h"

#define DIR_BUF_NUM 256



//in file.c
extern const char dir_type_sort_default[];




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
		"disk0:/",
		NULL
	};
	char *menu_go_hitobashira[] = { 
		"ms0:/",
		"ef0:/",
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
		goで(vshのときだけ?)ef0:/からプラグインを読み込ませるとms0:/という文字列をef0:/と書き換えるっぽいので、
		更に上書きしてやる、、、けど文字列リテラルって本当は書き換えたらまずいよね・・・
		*/
		menu[0][0] = 'm';
		menu[0][1] = 's';
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

#define MAX_DISPLAY_NUM 21
//selectType == 0 通常
//selectType == 1 ディレクトリも選択できる
//selectType == 2 ディレクトリしか選択できない


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
 @param : startPath 
		  ファイルセレクタが最初に閲覧するフォルダのパス
 @param : rtn 
		   選択されたら返す情報
 @param : titleLabel 
 		      ファイルセレクタの上部に表示されるタイトル
 @param : selectType 
 		      == 0 ファイルだけを選択できる / ==1 フォルダもファイルも選択できる / == 2 フォルダだけ選択できる
 @param : dir_type_sort 
 		  		  ファイルをソートするための情報 NULL にすればデフォルトのソート順になる / 詳しくはfile.cに
 
 @retrun : 
           == 0 no problem
           == 1 なにも選ばれなかった(quit by user)
           
*/


int fileSelecter(const char *startPath, dir_t *rtn, char* titleLabel,int selectType, char *dir_type_sort)
{
	
	if( selectType < 0 || selectType > 2 ) selectType = 0;

	dir_t *dirBuf = malloc(sizeof(dir_t)*DIR_BUF_NUM);
	if( dirBuf == NULL ) return -1;
	
	int dir_num,offset,i,now_arrow,tmp;
	u32 beforeButtons = 0;
	clock_t time = 0;
	char currentPath[256];
	bool firstFlag = true;
	strcpy(currentPath,startPath);

	while(1){

		
		dir_num = read_dir(dirBuf,currentPath, 0,dir_type_sort,DIR_BUF_NUM);
		offset = 0;
		now_arrow = 0;

		PRINT_SCREEN();
		libmPrintf(15,28,BG_COLOR,FG_COLOR, titleLabel);
		libmPrintf(15,36,BG_COLOR,FG_COLOR,"[%d] [%s]",dir_num,currentPath);
		libmPrintf(5,264,EX_COLOR,BG_COLOR, (selectType == 0)?PPREFSMSG_ADD_HOWTOUSE:(selectType == 1)?PPREFSMSG_ADD_HOWTOUSE_2:PPREFSMSG_ADD_HOWTOUSE_3, buttonData[buttonNum[0]].name);

PRINT_LIST:
		libmFillRect( 0 , 46 , 480 , 46 + MAX_DISPLAY_NUM*(LIBM_CHAR_HEIGHT+2),BG_COLOR );
		if( dir_num != 0 ){
			for( i = 0; i < dir_num && i < MAX_DISPLAY_NUM; i++ ){
#ifdef PPREFS_CHARCONV
				psp2chUTF82Sjis(commonBuf,dirBuf[i+offset].name);
				if( dirBuf[i+offset].type == TYPE_DIR )
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s/",commonBuf);
				else
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",commonBuf);
#else
				if( dirBuf[i+offset].type == TYPE_DIR )
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s/",dirBuf[i+offset].name);
				else
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dirBuf[i+offset].name);

#endif
			}
			libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		}
		
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_DOWN )
			{
				ALLORW_WAIT(PSP_CTRL_DOWN,3 * 100 * 1000,1 * 100 * 1000);
				
				if( now_arrow + 1 < MAX_DISPLAY_NUM && now_arrow + 1 < dir_num ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow++;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset+MAX_DISPLAY_NUM < dir_num ){
						offset++;
						goto PRINT_LIST;
					}
				}
			}
			else if( padData.Buttons & PSP_CTRL_UP )
			{
				ALLORW_WAIT(PSP_CTRL_UP,3 * 100 * 1000,1 * 100 * 1000);
				
				if( now_arrow - 1 >= 0 ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow--;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset > 0 ){
						offset--;
						goto PRINT_LIST;
					}
				}
			}
			else if( padData.Buttons & PSP_CTRL_RIGHT )
			{
				ALLORW_WAIT(PSP_CTRL_RIGHT,2 * 100 * 1000,1 * 100 * 1000);

				libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				tmp = 0;
				for( i = 0; i < 5; i++ ){
					if( now_arrow + 1 < MAX_DISPLAY_NUM && now_arrow + 1 < dir_num ){
						now_arrow++;
					}else if( offset+MAX_DISPLAY_NUM < dir_num ){
						offset++;
						tmp = 1;
					}
				}
				if( tmp ) goto PRINT_LIST;
				libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				/*
				i = (dir_num >= MAX_DISPLAY_NUM)?dir_num - MAX_DISPLAY_NUM:0;//tmp value
				if( i == offset ){
					now_arrow = (i == 0)?dir_num-1:MAX_DISPLAY_NUM-1;
				}else{
					offset += MAX_DISPLAY_NUM;
					now_arrow=0;
					if( offset+MAX_DISPLAY_NUM >= dir_num ) offset = i;
				}
				goto PRINT_LIST;
				*/
				
			}
			else if( padData.Buttons & PSP_CTRL_LEFT )
			{
				ALLORW_WAIT(PSP_CTRL_LEFT,2 * 100 * 1000,1 * 100 * 1000);
				
				
				libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				tmp = 0;
				for( i = 0; i < 5; i++ ){
					if( now_arrow - 1 >= 0 ){
						now_arrow--;
					}else if( offset > 0 ){
						offset--;
						tmp = 1;
					}
				}
				if( tmp ) goto PRINT_LIST;
				libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				/*
				if( offset == 0 ){
					now_arrow = 0;
				}else{
					offset -= MAX_DISPLAY_NUM;
					if( offset < 0 ) offset = 0;
					now_arrow = 0;
				}
				goto PRINT_LIST;
				*/
			}
			else if( padData.Buttons & (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER) )
			{
				beforeButtons = (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER);

				//空のフォルダーではない
				if(  dir_num != 0 ){
					//選択されたものがフォルダー
					if( dirBuf[offset+now_arrow].type == TYPE_DIR ){
						strcat(currentPath,dirBuf[offset+now_arrow].name);
						strcat(currentPath,"/");
						wait_button_up(&padData);
						break;
					//選択されたものがフォルダーではない && buttonData[buttonNum[0]].flagボタンが押されている
					}else if( (padData.Buttons & buttonData[buttonNum[0]].flag) && dir_num != 0 && selectType != 2 ){
						*rtn = dirBuf[offset+now_arrow];
						strcpy( rtn->name , currentPath );
						strcat( rtn->name , dirBuf[offset+now_arrow].name );
						wait_button_up(&padData);
						free(dirBuf);
						return 0;
					}
				}
				wait_button_up(&padData);
			}
			else if( padData.Buttons & PSP_CTRL_LTRIGGER )
			{
				beforeButtons = PSP_CTRL_LTRIGGER;
				
				wait_button_up(&padData);
				if( currentPath[strlen(currentPath) - 2] == ':' ){
					selectStrage(currentPath);
					break;
				}else if( up_dir(currentPath) >= 0 ){
					break;
				}
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				wait_button_up(&padData);
				free(dirBuf);
				return 1;
			}
			else if( (selectType == 1 || selectType == 2) && padData.Buttons & PSP_CTRL_START )
			{
				beforeButtons = PSP_CTRL_START;

				strcpy( rtn->name , currentPath );
				rtn->type = TYPE_DIR;
				rtn->sort_type = dir_type_sort_default[TYPE_DIR];

				wait_button_up(&padData);
				free(dirBuf);
				return 0;
				

			}
			else
			{
				beforeButtons = 0;
			}
//			wait_button_up(&padData);
		}
	}
}





int fileManager(const char *startPath ,char* titleLabel,int selectType, char *dir_type_sort)
{
	
	if( selectType < 0 || selectType > 2 ) selectType = 0;

	dir_t *dirBuf = malloc(sizeof(dir_t)*DIR_BUF_NUM);
	if( dirBuf == NULL ) return -1;
	
	int dir_num,offset,i,now_arrow;
	u32 beforeButtons = 0;
	clock_t time = 0;
	char currentPath[256];
	bool firstFlag = true;
	strcpy(currentPath,startPath);

	while(1){

		
		dir_num = read_dir(dirBuf,currentPath, 0,dir_type_sort,DIR_BUF_NUM);
		offset = 0;
		now_arrow = 0;

		PRINT_SCREEN();
		libmPrintf(15,28,BG_COLOR,FG_COLOR, titleLabel);
		libmPrintf(15,36,BG_COLOR,FG_COLOR,"[%d] [%s]",dir_num,currentPath);
		libmPrintf(5,264,EX_COLOR,BG_COLOR, (selectType == 0)?PPREFSMSG_ADD_HOWTOUSE:(selectType == 1)?PPREFSMSG_ADD_HOWTOUSE_2:PPREFSMSG_ADD_HOWTOUSE_3, buttonData[buttonNum[0]].name);

PRINT_LIST:
		libmFillRect( 0 , 46 , 480 , 46 + MAX_DISPLAY_NUM*(LIBM_CHAR_HEIGHT+2),BG_COLOR );
		if( dir_num != 0 ){
			for( i = 0; i < dir_num && i < MAX_DISPLAY_NUM; i++ ){
#ifdef PPREFS_CHARCONV
				psp2chUTF82Sjis(commonBuf,dirBuf[i+offset].name);
				if( dirBuf[i+offset].type == TYPE_DIR )
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s/",commonBuf);
				else
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",commonBuf);
#else
				if( dirBuf[i+offset].type == TYPE_DIR )
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s/",dirBuf[i+offset].name);
				else
					libmPrintf(15,46 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dirBuf[i+offset].name);

#endif
			}
			libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		}
		
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_DOWN )
			{
				ALLORW_WAIT(PSP_CTRL_DOWN,3 * 100 * 1000,1 * 100 * 1000);
				
				if( now_arrow + 1 < MAX_DISPLAY_NUM && now_arrow + 1 < dir_num ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow++;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset+MAX_DISPLAY_NUM < dir_num ){
						offset++;
						goto PRINT_LIST;
					}
				}
			}
			else if( padData.Buttons & PSP_CTRL_UP )
			{
				ALLORW_WAIT(PSP_CTRL_UP,3 * 100 * 1000,1 * 100 * 1000);
				
				if( now_arrow - 1 >= 0 ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow--;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset > 0 ){
						offset--;
						goto PRINT_LIST;
					}
				}
			}
			else if( padData.Buttons & PSP_CTRL_RIGHT )
			{
				ALLORW_WAIT(PSP_CTRL_RIGHT,2 * 100 * 1000,1 * 100 * 1000);
				
				i = (dir_num >= MAX_DISPLAY_NUM)?dir_num - MAX_DISPLAY_NUM:0;//tmp value
				if( i == offset ){
					now_arrow = (i == 0)?dir_num-1:MAX_DISPLAY_NUM-1;
				}else{
					offset += MAX_DISPLAY_NUM;
					now_arrow=0;
					if( offset+MAX_DISPLAY_NUM >= dir_num ) offset = i;
				}
				goto PRINT_LIST;
				
				
			}
			else if( padData.Buttons & PSP_CTRL_LEFT )
			{
				ALLORW_WAIT(PSP_CTRL_LEFT,2 * 100 * 1000,1 * 100 * 1000);
				if( offset == 0 ){
					now_arrow = 0;
				}else{
					offset -= MAX_DISPLAY_NUM;
					if( offset < 0 ) offset = 0;
					now_arrow = 0;
				}
				goto PRINT_LIST;
			}
			else if( padData.Buttons & (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER) )
			{
				beforeButtons = (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER);

				//現在地が空のフォルダではない
				if(  dir_num != 0 ){
					//選択されたものがフォルダー
					if( dirBuf[offset+now_arrow].type == TYPE_DIR ){
						strcat(currentPath,dirBuf[offset+now_arrow].name);
						strcat(currentPath,"/");
						wait_button_up(&padData);
						break;
					//選択されたものがファイル && buttonData[buttonNum[0]].flagボタンが押されている
					}else if( (padData.Buttons & buttonData[buttonNum[0]].flag) && dir_num != 0 && selectType != 2 ){
						char tmpPath[256];
						char *yesnoList[] = { PPREFSMSG_YESORNO_LIST };

						strcpy( tmpPath , currentPath );
						strcat( tmpPath , dirBuf[offset+now_arrow].name );
						if( pprefsMakeSelectBox(24,  40, "Really?", yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
							sceIoRemove(tmpPath);
							wait_button_up(&padData);
							break;
						}else{
							wait_button_up(&padData);
							goto PRINT_LIST;
						}
					}
				}
				wait_button_up(&padData);
			}
			else if( padData.Buttons & PSP_CTRL_LTRIGGER )
			{
				beforeButtons = PSP_CTRL_LTRIGGER;
				
				wait_button_up(&padData);
				if( currentPath[strlen(currentPath) - 2] == ':' ){
					selectStrage(currentPath);
					break;
				}else if( up_dir(currentPath) >= 0 ){
					break;
				}
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				wait_button_up(&padData);
				free(dirBuf);
				return 1;
			}
			else
			{
				beforeButtons = 0;
			}
//			wait_button_up(&padData);
		}
	}
}
