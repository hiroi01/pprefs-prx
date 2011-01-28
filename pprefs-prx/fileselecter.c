#include "fileselecter.h"
#include "common.h"
#include "button.h"
#include "language.h"
#include "pprefsmenu.h"

//ファイルリスト(ファイルブラウザ)のためのbuffer
dir_t dirBuf[128];
dir_t dirTmp;

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
		goで(vshのときだけ?)ef0:/からプラグインを読み込ませるとms0:/という文字列をef0:/と書き換えるっぽいので、
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

#define MAX_DISPLAY_NUM 21
//selectType == 0 通常
//selectType == 1 ディレクトリも選択できる
int fileSelecter(const char *startPath, dir_t *rtn, char* titleLabel,int selectType, char *dir_type_sort)
{
	int dir_num,offset,i,now_arrow;
	u32 beforeButtons = 0;
	clock_t time = 0;
	char currentPath[256];
	strcpy(currentPath,startPath);

	while(1){

		
		dir_num = read_dir(dirBuf,currentPath, 0,dir_type_sort);
		offset = 0;
		now_arrow = 0;

		PRINT_SCREEN();
		libmPrintf(15,28,BG_COLOR,FG_COLOR, titleLabel);
		libmPrintf(15,36,BG_COLOR,FG_COLOR," [%s] [%d] ",currentPath,dir_num);
		libmPrintf(5,264,FG_COLOR,BG_COLOR, (selectType == 0)?PPREFSMSG_ADD_HOWTOUSE:(selectType == 1)?PPREFSMSG_ADD_HOWTOUSE_2:PPREFSMSG_ADD_HOWTOUSE_3, buttonData[buttonNum[0]].name);

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
				if( beforeButtons & PSP_CTRL_DOWN ){
					if( (sceKernelLibcClock() - time) >= (2 * 100 * 1000) ){
						time = sceKernelLibcClock();
					}else{
						continue;
					}
				}else{
					beforeButtons = PSP_CTRL_DOWN;
					time = sceKernelLibcClock();
				}
				
				if( now_arrow + 1 < MAX_DISPLAY_NUM && now_arrow + 1 < dir_num ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow++;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset+MAX_DISPLAY_NUM < dir_num ) offset++;
					goto PRINT_LIST;
				}
			}else if( padData.Buttons & PSP_CTRL_UP ){
				if( beforeButtons & PSP_CTRL_UP ){
					if( (sceKernelLibcClock() - time) >= (2 * 100 * 1000) ){
						time = sceKernelLibcClock();
					}else{
						continue;
					}
				}else{
					beforeButtons = PSP_CTRL_UP;
					time = sceKernelLibcClock();
				}
				
				if( now_arrow - 1 >= 0 ){
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
					now_arrow--;
					libmPrintf(5,46 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				}else{
					if( offset > 0 ) offset--;
					goto PRINT_LIST;
				}
			}else if( padData.Buttons & (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER) ){
				beforeButtons = (buttonData[buttonNum[0]].flag | PSP_CTRL_RTRIGGER);
				if( selectType == 2 ){
					if( padData.Buttons & buttonData[buttonNum[0]].flag ){
						strcpy( rtn->name , currentPath );
						rtn->type = TYPE_DIR;
						rtn->sort_type = dir_type_sort_default[TYPE_DIR];
						wait_button_up(&padData);
						return 0;
					}
				}
				//空のフォルダーではない
				if(  dir_num != 0 ){
					//選択されたものがフォルダー
					if( dirBuf[offset+now_arrow].type == TYPE_DIR ){
						strcat(currentPath,dirBuf[offset+now_arrow].name);
						strcat(currentPath,"/");
						wait_button_up(&padData);
						break;
					//選択されたものがフォルダーではない && buttonData[buttonNum[0]].flagボタンが押されている
					}else if( (padData.Buttons & buttonData[buttonNum[0]].flag) && dir_num != 0 ){
						*rtn = dirBuf[offset+now_arrow];
						strcpy( rtn->name , currentPath );
						strcat( rtn->name , dirBuf[offset+now_arrow].name );
						wait_button_up(&padData);
						return 0;
					}
				}
				wait_button_up(&padData);
			}else if( padData.Buttons & PSP_CTRL_LTRIGGER ){
				beforeButtons = PSP_CTRL_LTRIGGER;
				
				wait_button_up(&padData);
				if( currentPath[strlen(currentPath) - 2] == ':' ){
					selectStrage(currentPath);
					break;
				}else if( up_dir(currentPath) >= 0 ){
					break;
				}
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				wait_button_up(&padData);
				return 1;
			}else if( selectType == 1 && padData.Buttons & PSP_CTRL_START ){
				beforeButtons = PSP_CTRL_START;

				strcpy( rtn->name , currentPath );
				rtn->type = TYPE_DIR;
				rtn->sort_type = dir_type_sort_default[TYPE_DIR];

				wait_button_up(&padData);
				return 0;
				

			}else{
				beforeButtons = 0;
			}
//			wait_button_up(&padData);
		}
	}
}
