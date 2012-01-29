#include "common.h"
#include "button.h"
#include "pprefsmenu.h"
#include "language.h"
#include "fileselecter.h"

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
	}else if( tmp == 2 ){
		rtn |= SORT_TYPE_CATEGORIZES_LIGHT_620;
	}else if( tmp == 3){
		rtn |= SORT_TYPE_CATEGORIZES_LIGHT_63X;
	}


	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_ISOCSO ,yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
		rtn |= SORT_TYPE_ISOCSO;
	}
	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_GAME150 ,yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
		rtn |= SORT_TYPE_GAME150;
	}
	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_GAME5XX ,yesnoList, buttonData[buttonNum[0]].flag, 0 ) == 0 ){
		rtn |= SORT_TYPE_GAME5XX;
	}
	if( pprefsMakeSelectBox(24,  40, PPREFSMSG_CONFIG_SORTTYPE_NOTDISPLAY_ICON0, yesnoList, buttonData[buttonNum[0]].flag, 0) == 1 ){
		rtn |= SORT_TYPE_NOTDISPLAY_ICON0;
	}

	return rtn;
}


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
	makeWindowSpeedy(
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

static inline u32 getArrowPositionY(int pos)
{
	if( pos == PPREFS_CONF_NUM ){
		pos++;
	}
	return (pos * SPACE_BETWEEN_THE_LINES);
}

static void printExplanation(int pos)
{
	char *list[] = {
		PPREFSMSG_CONFIGMENU_WHATISTHIS
	};
	
	makeWindowQuick(0, 235, 472 ,260,  FG_COLOR, BG_COLOR );
	if( pos < PPREFS_CONF_NUM )
		libmPrintf(5, 240 , FG_COLOR, BG_COLOR, list[pos]);
}

int config_menu(void)
{
	Conf_Key newConfig = config;
	char *temp, **listPtr;
	char buf[256];
	int currentPos = 0,i;
	u32 fgColor, bgColor;

	
	while(1){
		
		//draw
		printScreen();
		libmPrint(15,18,BG_COLOR,FG_COLOR,PPREFSMSG_CONFIGMENU_TITLE);
		
		for( i = 0; i < PPREFS_CONF_NUM; i++ ){
			char *name = ILPGetNameAddressByKeynumber(conf, i);
			//set color
			if( strncasecmp("Color", name, 5) == 0){
				fgColor = *(u32 *)ILPGetValueAddressByKeynumber(conf, i);
				bgColor = (fgColor == BG_COLOR)?FG_COLOR:BG_COLOR;
			}else{
				fgColor = FG_COLOR;
				bgColor = BG_COLOR;
			}
			
			//set string
			snprintf(buf, 256, "%s = ", name);
			name = strchr(buf, '/0');
			ILPGetStringFormatValueByKeynumber(conf, name, i);
			
			//print
			libmPrint(15, 28 + i*SPACE_BETWEEN_THE_LINES, fgColor, bgColor, buf);
		}

		
		libmPrint (15, 28 +(i+1)*(SPACE_BETWEEN_THE_LINES), FG_COLOR, BG_COLOR, PPREFSMSG_CONFIGMENU_MENU_1);
		libmPrint (15, 28 +(i+2)*(SPACE_BETWEEN_THE_LINES), FG_COLOR, BG_COLOR, PPREFSMSG_CONFIGMENU_MENU_2);
		libmPrint (15, 28 +(i+3)*(SPACE_BETWEEN_THE_LINES), FG_COLOR, BG_COLOR, PPREFSMSG_CONFIGMENU_MENU_3);


		libmPrint(5, 28 + getArrowPositionY(currentPos), FG_COLOR, BG_COLOR, ">");
		libmPrintf(5, 264, EX_COLOR, BG_COLOR, PPREFSMSG_CONFIGMENU_HOWTOUSE,buttonData[buttonNum[0]].name);
		
		printExplanation(currentPos);
		
		while(1){
			get_button(&padData);
			
			if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP) )
			{
				libmPrint(5, 28 + getArrowPositionY(currentPos), FG_COLOR, BG_COLOR, " ");
				
				if( padData.Buttons & PSP_CTRL_DOWN){
					currentPos++;
					if( currentPos >= (PPREFS_CONF_NUM + 3) )
						currentPos = 0;
				}else{// if( padData.Buttons & PSP_CTRL_UP )
					currentPos--;
					if( currentPos < 0 )
						currentPos = (PPREFS_CONF_NUM + 2);
				}
				
				libmPrint(5, 28 + getArrowPositionY(currentPos), FG_COLOR, BG_COLOR, ">");
				printExplanation(currentPos);
				
				wait_button_up(&padData);
			}
			else if( padData.Buttons & buttonData[buttonNum[0]].flag )
			{
				if( currentPos < PPREFS_CONF_NUM ){
					char *name = ILPGetNameAddressByKeynumber(conf, currentPos);
					if( strcasecmp("BasePath", name) == 0 )
					{
						char *list[] = {
							PPREFSMSG_CONFIG_BASEPATH_LIST
						};
						int selectNum = pprefsMakeSelectBox(32,  32, PPREFSMSG_CONFIG_BASEPATH_TITLE,list, buttonData[buttonNum[0]].flag, 1 );
						switch (selectNum) {
							case 0:
								selectBasePath(newConfig.basePath);
								break;
							case 1:
								newConfig.basePath[0] = '\0';
								break;
						}
					}
					else if( strcasecmp("SortType", name) == 0  )
					{
						config.sortType = selectSorttypeByuser();
					}
					else if( strcasecmp("Color2", name) == 0 )
					{
						if( config.color2 == RED ){
							config.color2 = GREEN;
						}else if( config.color2 == GREEN ){
							config.color2 = 0xffff4040;
						}else if( config.color2 == 0xffff4040 ){
							config.color2 = RED;
						}
					}else{
						switch (ILPGetTypeByKeynumber(conf, currentPos)) {
							case ILP_TYPE_BUTTON:
							{
								u32 buttonsValue = detect_key();
								if( buttonsValue == 0 ){
									ILPSetDefaultOne(conf, currentPos);
								}else {
									*(u32 *)(ILPGetValueAddressByKeynumber(conf, currentPos)) = buttonsValue;
								}

								break;
							}
							case ILP_TYPE_LIST:
							{
//								listPtr = (char **)conf[currentPos].ex;
//								(*conf[currentPos].value.i)++;
//								if( listPtr[*conf[currentPos].value.i] == NULL ) *conf[currentPos].value.i = 0;
								break;
							}
							case ILP_TYPE_BOOL:
							{
								bool *boolValue = ILPGetValueAddressByKeynumber(conf, currentPos);
								*boolValue = ! *boolValue;
								break;
							}
						}
					}
				}else{
					switch (currentPos - PPREFS_CONF_NUM) {
						case 0:
							config = newConfig;
							
							strcpy(buf, ownPath);
							temp = strrchr(buf, '/');
							if( temp != NULL )
								*temp = '\0';
							strcat(buf, INI_NAME);
							
							ILPWriteToFile(conf, buf, lineFeedCode[config.lineFeedCode]);
							
							pprefsApplyConfig();
							
							wait_button_up(&padData);
							return 0;
						case 1:
							ILPSetDefaultAll(conf);
							
							wait_button_up(&padData);
							break;
						case 2:
							wait_button_up(&padData);
							return 1;
					}
				}
				break;
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				wait_button_up(&padData);
				return 1;
			}
		}
	}
	
}
