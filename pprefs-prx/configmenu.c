#include "common.h"
#include "button.h"
#include "pprefsmenu.h"
#include "language.h"
#include "fileselecter.h"

#ifndef PPREFS_LITE
u32 selectSorttypeByuser()
{
	u32 rtn = SORT_TYPE_GAME;
	int tmp;
	
	char *yesnoList[] = { PPREFSMSG_YESORNO_LIST };
	char *typeList[] = { PPREFSMSG_CONFIG_SORTTYPE_TYPE_LIST };

	tmp = pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_TYPE ,typeList, buttonData[buttonNum[0]].flag, 0 );
	
	if( tmp == 0 ){
		rtn |= SORT_TYPE_NORMAL_LIST;
	}else if( tmp == 1 ){
		rtn |= SORT_TYPE_CATEGORIZES;
	}else{
		rtn |= SORT_TYPE_CATEGORIZES_LIGHT;
	}


	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_ISOCSO ,yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
		rtn |= SORT_TYPE_ISOCSO;
	}
	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_GAME150 ,yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
		rtn |= SORT_TYPE_GAME150;
	}
	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_GAME500 ,yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
		rtn |= SORT_TYPE_GAME500;
	}
	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_NOTDISPLAY_ICON0, yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
		rtn |= SORT_TYPE_NOTDISPLAY_ICON0;
	}

	return rtn;
}
#else
u32 selectSorttypeByuser(){
	return 0;
}
#endif


void selectBasePath(char *path)
{
	wait_button_up(&padData);
		
	int selectNumber;
	
	char listBuf[16] = "**0:/seplugins/";
	char listBuf2[16] = "**0:/plugins/";
	/*
	6.20TN、6.35PRO-B2以降のパスのエイリアス機能(?)対策
	文字列リテラルで"ms0～"とすると"ef0～"と置き換えられるのでその対策
	*/
	listBuf[0] = 'm'; listBuf[1] = 's';
	listBuf2[0] = 'm'; listBuf2[1] = 's';

	if( deviceModel == 4 ){//if device is go
		char *list[] = {
			"ef0:/seplugins/" ,//0
			listBuf ,//1
			"ef0:/plugins/" ,//2
			listBuf2 ,//3
			PPREFSMSG_THEOTHER ,//4
			NULL
		};

		selectNumber = pprefsMakeSelectBoxSpeedy(8, 8, PPREFSMSG_SELECTBASEPATH,list, buttonData[buttonNum[0]].flag, 1 );
		if( selectNumber >= 0 &&  selectNumber != 4 ){
			strcpy(path,list[selectNumber]);
			wait_button_up(&padData);
			return;
		}

	}else{//1000 or 2000 or 3000
		char *list[] = {
			listBuf ,//0
			listBuf2 ,//1
			PPREFSMSG_THEOTHER ,//2
			NULL
		};
		
		selectNumber = pprefsMakeSelectBoxSpeedy(8, 8, PPREFSMSG_SELECTBASEPATH,list, buttonData[buttonNum[0]].flag, 1 );
		if( selectNumber >= 0 && selectNumber != 2 ){
			strcpy(path,list[selectNumber]);
			wait_button_up(&padData);
			return;
		}
	}


	//if selected PPREFSMSG_THEOTHER
	if( selectNumber >= 0 && fileSelecter(config.basePath,&dirTmp, PPREFSMSG_SELECTBASEPATH, 2, NULL ) == 0  ){
		strcpy(path,dirTmp.name);
	}
	
	wait_button_up(&padData);
}


//キー検出
u32 detect_key(void)
{
	
	clock_t timesec;
	char *temp;
	u32 beforeKey = 0;
	
	wait_button_up(&padData);
	makeWindow(
		24 , 28 ,
		480 - LIBM_CHAR_WIDTH*3 , 28 + LIBM_CHAR_HEIGHT*5,
		FG_COLOR,BG_COLOR
	);
	libmPrint( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT    , FG_COLOR, BG_COLOR, PPREFSMSG_DETECTKEY_1 );
	while(1){
		get_button(&padData);
		if( padData.Buttons != 0 ) break;
	}

	timesec = sceKernelLibcClock();
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
		libmPrintf  ( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT    , FG_COLOR, BG_COLOR, PPREFSMSG_DETECTKEY_2, (int)( 3 -  ( (sceKernelLibcClock() - timesec)/1000000) )  );
		if( (sceKernelLibcClock() - timesec) >= (3 * 1000 * 1000) ) break;
	}
	
	return padData.Buttons;
}

#define SPACE_BETWEEN_THE_LINES (LIBM_CHAR_HEIGHT + 2)

/*
#define ARROW_POSITION ( \
( now_arrow < conf[0].keyNum )? \
( now_arrow *2 ): \
( conf[0].keyNum * 2 + now_arrow - conf[0].keyNum + 1 ) \
)
*/

#define ARROW_POSITION ((now_arrow < conf[0].keyNum)?(now_arrow):(now_arrow+1))

#define PRINT_EXPLANATION() \
makeWindowQuick(0, 235, 472 ,260,  FG_COLOR, BG_COLOR ); \
if( now_arrow < conf[0].keyNum ) libmPrintf(5, 240 , FG_COLOR, BG_COLOR, whatIsThis[now_arrow]);

int config_menu(void)
{
	Conf_Key oldConfig = config;
	char *temp,**listPtr;
	char iniPath[256];
	int now_arrow = 0,i;
	char *whatIsThis[] = {
		PPREFSMSG_CONFIGMENU_WHATISTHIS
	};

	
	while(1){
		PRINT_SCREEN();
		
		libmPrint(15,28,BG_COLOR,FG_COLOR,PPREFSMSG_CONFIGMENU_TITLE);
		for( i = 0; i < conf[0].keyNum; i++ ){
			if( 
				strcasecmp( "Color0", conf[i].key ) == 0 ||
				strcasecmp( "Color2", conf[i].key ) == 0 ||
				strcasecmp( "Color3", conf[i].key ) == 0
			){
				libmPrintf(15, 46 + i*SPACE_BETWEEN_THE_LINES, *conf[i].value.u, BG_COLOR,"%s = %x",conf[i].key, *conf[i].value.u);
				continue;
			}else if( conf[i].type & INI_TYPE_BUTTON ){
				sprintf(commonBuf,"%s = ",conf[i].key);
				GET_KEY_NAME(*conf[i].value.u, commonBuf);
				temp = strrchr(commonBuf, '+');
				if( temp != NULL ) temp[-1] = '\0';
			}else if( conf[i].type & INI_TYPE_LIST ){
				listPtr = (char **)conf[i].ex;
				sprintf(commonBuf,"%s = %s", conf[i].key, listPtr[*conf[i].value.i]);
			}else if( conf[i].type & INI_TYPE_BOOL ){
				sprintf(commonBuf,"%s = %s",conf[i].key, *conf[i].value.b?"true":"false");
			}else if( conf[i].type & INI_TYPE_HEX ){
				sprintf(commonBuf,"%s = %x",conf[i].key, *conf[i].value.u);
			}else if( conf[i].type & INI_TYPE_STRING ){
				sprintf(commonBuf,"%s = %s",conf[i].key, conf[i].value.s);
			}
			libmPrint (15, 46 + i*SPACE_BETWEEN_THE_LINES, FG_COLOR, BG_COLOR,commonBuf);
//			libmPrint (15, 46 + (i*2+1)*SPACE_BETWEEN_THE_LINES, EX_COLOR , BG_COLOR, whatIsThis[i]);
		}

		
		libmPrint (15, 46 +(i+1)*(SPACE_BETWEEN_THE_LINES), FG_COLOR, BG_COLOR, PPREFSMSG_CONFIGMENU_MENU_1);
		libmPrint (15, 46 +(i+2)*(SPACE_BETWEEN_THE_LINES), FG_COLOR, BG_COLOR, PPREFSMSG_CONFIGMENU_MENU_2);
		libmPrint (15, 46 +(i+3)*(SPACE_BETWEEN_THE_LINES), FG_COLOR, BG_COLOR, PPREFSMSG_CONFIGMENU_MENU_3);


		libmPrintf(5, 46 +ARROW_POSITION * SPACE_BETWEEN_THE_LINES, FG_COLOR, BG_COLOR, ">");
		libmPrintf(5,264,EX_COLOR ,BG_COLOR,PPREFSMSG_CONFIGMENU_HOWTOUSE,buttonData[buttonNum[0]].name);
		PRINT_EXPLANATION();
		wait_button_up(&padData);
		while(1){
			get_button(&padData);
			if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP) ){
				libmPrintf(5, 46 +ARROW_POSITION* SPACE_BETWEEN_THE_LINES, FG_COLOR, BG_COLOR, " ");
				if( padData.Buttons & PSP_CTRL_DOWN){
					now_arrow++;
					if( now_arrow >= conf[0].keyNum+3 ) now_arrow = 0;
				}else if( padData.Buttons & PSP_CTRL_UP ){
					now_arrow--;
					if( now_arrow < 0 ) now_arrow = conf[0].keyNum + 2;
				}
				libmPrintf(5, 46 +ARROW_POSITION * SPACE_BETWEEN_THE_LINES, FG_COLOR, BG_COLOR, ">");

				PRINT_EXPLANATION();
				wait_button_up(&padData);
			}else if( padData.Buttons & buttonData[buttonNum[0]].flag ){
				if( now_arrow < conf[0].keyNum ){
					if( strcasecmp( "BasePath", conf[now_arrow].key ) == 0 ){
						char *list[] = {PPREFSMSG_CONFIG_BASEPATH_LIST};
						int selectNum = pprefsMakeSelectBox(32,  32, PPREFSMSG_CONFIG_BASEPATH_TITLE,list, buttonData[buttonNum[0]].flag, 1 );
						
						if( selectNum == 0 ){
							selectBasePath(conf[now_arrow].value.s);
						}else if( selectNum == 1){
							conf[now_arrow].value.s[0] = '\0';
						}
					}else if( strcasecmp( "SortType", conf[now_arrow].key ) == 0  ){
						*conf[now_arrow].value.u = selectSorttypeByuser();
					}else if( strcasecmp( "Color2", conf[now_arrow].key ) == 0 ){
						if( *conf[now_arrow].value.u == RED ){
							*conf[now_arrow].value.u = GREEN;
						}else if( *conf[now_arrow].value.u == GREEN ){
							*conf[now_arrow].value.u = RED;
						}
					}else if( conf[now_arrow].type & INI_TYPE_BUTTON ){
						*conf[now_arrow].value.u = detect_key();
						if( *conf[now_arrow].value.u == 0 ) *conf[now_arrow].value.u = conf[now_arrow].defaultValue.u;
					}else if( conf[now_arrow].type & INI_TYPE_LIST ){
						listPtr = (char **)conf[now_arrow].ex;
						(*conf[now_arrow].value.i)++;
						if( listPtr[*conf[now_arrow].value.i] == NULL ) *conf[now_arrow].value.i = 0;
					}else if( conf[now_arrow].type & INI_TYPE_BOOL ){
						*conf[now_arrow].value.b = !*conf[now_arrow].value.b;
					}
				}else{
					if( now_arrow - conf[0].keyNum ==  0 ){
						strcpy(iniPath, ownPath);
						temp = strrchr(iniPath, '/');
						if( temp != NULL ) *temp = '\0';
						strcat(iniPath,INI_NAME);
						INI_Write_Conf(iniPath, conf, lineFeedCode[config.lineFeedCode]);
						SET_CONFIG();
						wait_button_up(&padData);
						return 0;
					}else if( now_arrow - conf[0].keyNum ==  1 ){//set default
						INI_Set_Default(conf);
						wait_button_up(&padData);
						break;
					}else if( now_arrow - conf[0].keyNum ==  2 ){//
						config = oldConfig;
						wait_button_up(&padData);
						return 1;
					}
				}
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				config = oldConfig;
				wait_button_up(&padData);
				return 1;
			}
		}

	}
	
}
