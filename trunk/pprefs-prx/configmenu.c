#include "common.h"
#include "libmenu.h"
#include "button.h"
#include "main.h"
#include "language.h"
#include "pprefs.h"

u32 detect_key(void)
{
	
	double timesec;
	char *temp;
	u32 beforeKey = 0;
	
	wait_button_up(&padData);
	makeWindow(
		24 , 28 ,
		480 - LIBM_CHAR_WIDTH*3 , 28 + LIBM_CHAR_HEIGHT*5,
		FG_COLOR,BG_COLOR
	);
	libmPrint( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT    , FG_COLOR, BG_COLOR, "キー検出 残り 秒" );
	while(1){
		get_button(&padData);
		if( padData.Buttons != 0 ) break;
	}

	timesec = gettimeofday_sec();
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
		libmPrintf  ( 24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT    , FG_COLOR, BG_COLOR, "キー検出 残り%d秒", (int)(3 - (gettimeofday_sec() - timesec)) );
		if( (gettimeofday_sec() - timesec) >= 3 ) break;
	}
	
	return padData.Buttons;
}

int config_menu(void)
{
	Conf_Key newConfig = config;
	char *temp;
	int now_arrow = 0,menuNum = 9;
	char *lineFeedCodeName[] = {
		"CR+LF",
		"LF",
//		"CF"
	};
	
	while(1){
		PRINT_SCREEN();
		
		libmPrint(15,28,BG_COLOR,FG_COLOR," 設定 ");
		strcpy(commonBuf,"起動キー:");
		GET_KEY_NAME(newConfig.bootKey, commonBuf);
		temp = strrchr(commonBuf, '+');
		if( temp != NULL ) temp[-1] = '\0';
		libmPrint (15, 46                         , FG_COLOR, BG_COLOR,commonBuf);
		libmPrint (15, 46 + 1*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "pprefsを起動させるボタンの指定(デフォルトはHOME)");

		libmPrintf(15, 46 + 2*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "本体起動時メッセージ:%s",newConfig.bootMessage?"O N":"OFF");
		libmPrint (15, 46 + 3*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "本体を起動したときに左下に表\示される「pprefs起動準備完了!～」を表\示するか?(デフォルトはON)");
	
		libmPrintf(15, 46 + 4*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "×/○の役割入れ替え:%s", newConfig.swapButton?"O N":"OFF");
		libmPrint (15, 46 + 5*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "×/○ボタンの役割を入れ替える ON→○決定/×キャンセル OFF→×決定/○キャンセル(デフォルトはOFF)");
		
		libmPrintf(15, 46 + 6*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "一度押しでVSH再起動:%s", newConfig.onePushRestart?"O N":"OFF");
		libmPrint (15, 46 + 7*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "VSH再起動するのにSTARTを一度押すか二度押すか ON→一度押し OFF→二度押し(デフォルトはOFF)");

		libmPrintf(15, 46 + 8*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "改行コード:%s", lineFeedCodeName[newConfig.lineFeedCode]);
		libmPrint (15, 46 + 9*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "このプラグインがテキストを書き出すときに使う改行コード(デフォルトはCR+LF)");

		libmPrintf(15, 46 + 10*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "デフォルトパス:%s", (newConfig.defaultPath == 0)?"ms0:/seplugins/":"ms0:/plugins/" );
		libmPrint (15, 46 + 11*(LIBM_CHAR_HEIGHT+2), SILVER  , BG_COLOR, "vsh.txt,game.txt,pops.txtのパス(この項目を変更したら手動でリロードして下さい)");

		libmPrint (15, 46 +12*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "上記の設定で保存する");

		libmPrint (15, 46 +14*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "デフォルト値にする");

		libmPrint (15, 46 +16*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, "やめる");

		libmPrintf(5, 46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2), FG_COLOR, BG_COLOR, ">");

		libmPrintf(5,264,FG_COLOR,BG_COLOR," %s:選択 HOME:やめる",buttonData[buttonNum[0]].name);

		wait_button_up(&padData);
		while(1){
			get_button(&padData);
			if( padData.Buttons & PSP_CTRL_DOWN ){
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				if( now_arrow + 1 < menuNum ) now_arrow++;
				else now_arrow = 0;
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				wait_button_up(&padData);
			}else if( padData.Buttons & PSP_CTRL_UP ){
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				if( now_arrow - 1 >= 0 ) now_arrow--;
				else now_arrow = menuNum - 1;
				libmPrintf(5,46 + now_arrow*2*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				wait_button_up(&padData);
			}else if( padData.Buttons & buttonData[buttonNum[0]].flag ){
				if( now_arrow == 0 ){
					newConfig.bootKey = detect_key();
					if( newConfig.bootKey == 0 ) newConfig.bootKey = PSP_CTRL_HOME;
					break;
				}else if( now_arrow == 1 ){
					newConfig.bootMessage = ! newConfig.bootMessage;
					break;
				}else if( now_arrow == 2 ){
					newConfig.swapButton = ! newConfig.swapButton;
					break;
				}else if( now_arrow == 3 ){
					newConfig.onePushRestart = ! newConfig.onePushRestart;
					break;
				}else if( now_arrow == 4 ){
					newConfig.lineFeedCode++;
					if( newConfig.lineFeedCode > 1 ) newConfig.lineFeedCode = 0;
					break;
				}else if( now_arrow == 5 ){
					newConfig.defaultPath++;
					if( newConfig.defaultPath > 1 ) newConfig.defaultPath = 0;
					break;
				}else if( now_arrow == (menuNum -3) ){
					SUSPEND_THREADS();
					config = newConfig;
					if( config.swapButton ){
						buttonNum[0] = 1;
						buttonNum[1] = 0;
					}else{
						buttonNum[0] = 0;
						buttonNum[1] = 1;
					}
					Set_Default_Path( sepluginsTextPath,config.defaultPath);
					Write_Conf(ownPath,&newConfig);
					wait_button_up(&padData);
					return 0;
				}else if( now_arrow == (menuNum -2) ){
					Set_Default_Conf(&newConfig);
					break;
				}else if( now_arrow == (menuNum -1) ){
					wait_button_up(&padData);
					return 1;
				}
			}else if( padData.Buttons & PSP_CTRL_HOME ){
					wait_button_up(&padData);
					return 1;
			}
		}

	}
	
}
