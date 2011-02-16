#include "common.h"
#include "memory.h"
#include "button.h"
#include "fileselecter.h"
#include "pprefsmenu.h"
#include "ciso.h"
#include "language.h"

/*-----------------------------------------------------------------*/


// http://katsura-kotonoha.sakura.ne.jp/prog/c/tip00010.shtml
#define jms1(c) \
(((0x81 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0x9F)) || ((0xE0 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC) ))

#define jms2(c) \
((0x7F != (unsigned char)(c)) && (0x40 <= ((unsigned char)(c))) && (((unsigned char)(c)) <= 0xFC))


#define MAXDISPLAY_X 60
#define MAXDISPLAY_Y 27

#define TOP_PRINT_Y 36

#define PERGAMEBUF_SIZE (1024*5 + 1)

enum
{
	SELECTED_TOP		= 1,
	SELECTED_BOTTOM		= 2,
	TAIL_IS_EOF			= 4,
	TAIL_IS_N			= 8,
};
/*-----------------------------------------------------------------*/

char *pergameBuf = NULL;

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

//GAME ID か FULL PAHT かを選ぶダイアログ
int is_idOrPath(void){
	int i = 0;

	makeWindow(24, 28, 24 + LIBM_CHAR_WIDTH*26, 28 + LIBM_CHAR_HEIGHT*5, FG_COLOR, BG_COLOR);
	while(1){
		libmPrint(24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT  , (i == 0)?SL_COLOR:FG_COLOR, BG_COLOR, "GAME ID");
		libmPrint(24 + LIBM_CHAR_WIDTH, 28 + LIBM_CHAR_HEIGHT*3, (i == 1)?SL_COLOR:FG_COLOR, BG_COLOR, "FULL PATH");
		while(1){
			wait_button_up(&padData);
			get_button(&padData);
			if( padData.Buttons & buttonData[buttonNum[0]].flag ){
				return i;
			}else if( padData.Buttons & (PSP_CTRL_UP|PSP_CTRL_DOWN) ){
				i = (i == 0)?1:0;
				break;
			}
		}
	}
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
	int ret = 1;
	char tmp[] = { str[0], str[1], '\0' };
	if( ! jms1(str[0]) ){//2byte文字じゃないなら
		tmp[2] = '\0';
		ret = 0;
	}
	libmPrint( x, y, fg, bg, tmp );
	return ret;
}


u16  printText(char *pergameBuf, int headOffset, int lineCount, int selectedLine, int end_y)
{
	int i,x_count = 0,y_count = 0,ret = 0;
	
	//最上に表示されている行が選択されている
	if( selectedLine  == lineCount ) ret |= SELECTED_TOP;
	
	for( i = headOffset; pergameBuf[i] != '\0'; i++ ){
		if( pergameBuf[i] == '\n' ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
			lineCount++;
			continue;
		}
		
		if( pprefsPutChar(LIBM_CHAR_WIDTH*x_count ,TOP_PRINT_Y+LIBM_CHAR_HEIGHT*y_count ,(lineCount==selectedLine)?SL_COLOR:FG_COLOR, BG_COLOR, &pergameBuf[i])
		   == 1 ) i++; //2byte文字なら
		
		x_count++;
		if( x_count >= MAXDISPLAY_X ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
		}
	}
	
	if( lineCount == selectedLine ) ret |= SELECTED_BOTTOM; //最下に表示されている行が選択されている
	if( pergameBuf[i] == '\0' ) ret |= TAIL_IS_EOF;
	else if( pergameBuf[i] == '\n' ) ret |= TAIL_IS_N;
	
	return ret;
}


//buf, bufの始まりの位置, 始まりの行数, 現在の選択されている行数, yの最大数
u16  noPrintText(char *pergameBuf, int headOffset, int lineCount, int selectedLine, int end_y)
{
	int i,x_count = 0,y_count = 0,ret = 0;

	//最上に表示されている行が選択されている
	if( selectedLine  == lineCount ) ret |= SELECTED_TOP;

	for( i = headOffset; pergameBuf[i] != '\0'; i++ ){
		if( pergameBuf[i] == '\n' ){
			x_count = 0;
			y_count++;
			if( y_count >= end_y ) break;
			lineCount++;
			continue;
		}
		
//		if( pprefsPutChar(LIBM_CHAR_WIDTH*x_count ,36+LIBM_CHAR_HEIGHT*y_count ,(lineCount==selectedLine)?SL_COLOR:FG_COLOR, BG_COLOR, &pergameBuf[i])
//		    == 1 ) i++; //2byte文字なら
		if(  jms1(pergameBuf[i]) ){//2byte文字なら
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
	if( pergameBuf[i] == '\0' ) ret |= TAIL_IS_EOF;
	else if( pergameBuf[i] == '\n' ) ret |= TAIL_IS_N;

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
		if(  str[first+i] == '\0'  || str[second+i] == '\0' )break;
		
		tmp = str[first+i];
		str[first+i] = str[second+i];
		str[second+i] = tmp;
		
		if( str[first+i] == '\n' || str[second+i] == '\n' ) break;
		
		i++;
	}
	
	if( str[second+i] == '\n' || str[second+i] == '\0' ){
		i = second + i + 1;
		shiftNum = i-second;
		tmp = 1;//tmp is used as flag here
		while( tmp != 0 ){
			if( str[i] == '\n' ) tmp = 0;
			else if( str[i] == '\0' ) break;
			leftShiftChar(str,i,shiftNum);
			i++;
		}
	}else if( str[first+i] == '\n' || str[first+i] == '\0'){
		i = first + i + 1;
		shiftNum = second - first -1;
		tmp = 1;//tmp is used as flag here
		while( tmp != 0 ){
			if( str[i] == '\n' ) tmp = 0;
			else if( str[i] == '\0' ) break;
			rightShiftChar(str,i,shiftNum);
		}
	}
	
}

int saveEditPergame(char *str, char *path)
{
	SceUID fp;
	int i,tmp;
	
	
	fp = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if( fp < 0 ) return fp;
	
	i = 0;
	while(1){
		tmp = getOffsetOfTrueNextLine(str, i);
		if( tmp == i ) break;
		str[tmp-1] = '\0';
		sprintf(commonBuf,"%s%s",&str[i],lineFeedCode[config.lineFeedCode]);
		sceIoWrite(fp,commonBuf,strlen(commonBuf));
		i = tmp;
	}
	sceIoClose(fp);
	
	return 0;
	
	
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


int editPergameMenu(void)
{
	SceUID fd;
	int readSize,headLine,headOffset,selectedLine,i;
	u16 printState;
	char idBuf[11];
	char *pnt;
	u32 beforeButtons;
	clock_t timesec;
	char pergametxtPath[256];
	
	strcpy(pergametxtPath,rootPath);
	strcat(pergametxtPath,"seplugins/pergame.txt");
	
	if( pergameBuf == NULL ){
		pergameBuf = malloc(PERGAMEBUF_SIZE);
		if( pergameBuf == NULL ) return -1;
	}
	
	while(1){
		fd = sceIoOpen(pergametxtPath, PSP_O_RDONLY, 0777);
		if( fd >= 0 ) break;
		if( fileSelecter(rootPath,&dirTmp, "select pergame.txt", 0, NULL ) != 0 ) return -2;
		strcpy(pergametxtPath,dirTmp.name);
	}
	readSize = sceIoRead(fd, pergameBuf, PERGAMEBUF_SIZE - 1);
	sceIoClose(fd);
	if( readSize < 0 ) return -3;
	pergameBuf[readSize] = '\0';
	
	readSize = remove_r(pergameBuf);
	
	selectedLine = 0;
	headOffset = 0;
	headLine = 0;
	beforeButtons = 0;
	timesec = 0;
	i = 0;


	PRINT_SCREEN();
	libmPrint(24, 22,  BG_COLOR, FG_COLOR, PPREFSMSG_EDITPERGAME_TOP);
	libmPrintf(0, 256,  EX_COLOR , BG_COLOR, PPREFSMSG_EDITPERGAME_HOTOUSE,buttonData[buttonNum[1]].name);
	libmPrint (0, 264, EX_COLOR , BG_COLOR, PPREFSMSG_EDITPERGAME_HOTOUSE_2);
	while(1){


		libmFillRect( 0 , TOP_PRINT_Y , 480 , TOP_PRINT_Y + LIBM_CHAR_HEIGHT*MAXDISPLAY_Y , BG_COLOR);
		printState = printText(pergameBuf, headOffset, headLine, selectedLine,MAXDISPLAY_Y );

		
		
		if( beforeButtons == 0 ) wait_button_up(&padData);
		while(1){
			get_button(&padData);
			if( padData.Buttons & buttonData[buttonNum[1]].flag ){
				if( beforeButtons & buttonData[buttonNum[1]].flag ){
					if( (sceKernelLibcClock() - timesec) >= (5 * 100 * 1000) ){
						timesec = sceKernelLibcClock();
					}else{
						continue;
					}
				}else{
					beforeButtons = buttonData[buttonNum[1]].flag;
					timesec = sceKernelLibcClock();
					continue;
				}

				deleteLineOfString(pergameBuf,getOffsetOfSelecteLine(pergameBuf,selectedLine));

				if( getNumberOfLineOfString(pergameBuf) == 1 ) selectedLine = 0;

				beforeButtons = 0;
				break;
			}else if( padData.Buttons & (PSP_CTRL_RIGHT|PSP_CTRL_LEFT) ){
				beforeButtons = (PSP_CTRL_RIGHT|PSP_CTRL_LEFT);
				
				changeHeadChar(pergameBuf,getOffsetOfSelecteLine(pergameBuf,selectedLine),( padData.Buttons & PSP_CTRL_RIGHT)?'#':'!',PERGAMEBUF_SIZE);

				wait_button_up(&padData);
				break;
			}else if( padData.Buttons & PSP_CTRL_UP && selectedLine > 0 ){
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

				if( printState & SELECTED_TOP){
					i = getOffsetOfPreviousLine(pergameBuf,headOffset);
					//前の行に移動したなら
					if( pergameBuf[getOffsetOfNextLine(pergameBuf,i)-1] == '\n' && i != headOffset ){
						headLine--;
						if( padData.Buttons & PSP_CTRL_SQUARE ) swapLineOfString(pergameBuf, getOffsetOfSelecteLine(pergameBuf,selectedLine-1),getOffsetOfSelecteLine(pergameBuf,selectedLine));
						selectedLine--;
					}
					headOffset = i;
				}else{
					if( padData.Buttons & PSP_CTRL_SQUARE ) swapLineOfString(pergameBuf, getOffsetOfSelecteLine(pergameBuf,selectedLine-1),getOffsetOfSelecteLine(pergameBuf,selectedLine));
					selectedLine--;
				}

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
				
				if( printState & SELECTED_BOTTOM && (!(printState & TAIL_IS_EOF)) ){
					i = getOffsetOfNextLine(pergameBuf,headOffset);
					//次の行に移動したなら
					if( i > 0 && pergameBuf[i-1] == '\n' && i != headOffset ) headLine++;
					if( printState & TAIL_IS_N  ){
						if( padData.Buttons & PSP_CTRL_SQUARE ) swapLineOfString(pergameBuf, getOffsetOfSelecteLine(pergameBuf,selectedLine),getOffsetOfSelecteLine(pergameBuf,selectedLine+1));
						selectedLine++;
					}
					headOffset = i;
				}else if( ! ((printState & TAIL_IS_EOF) && (printState & SELECTED_BOTTOM)) ){
					if( padData.Buttons & PSP_CTRL_SQUARE ) swapLineOfString(pergameBuf, getOffsetOfSelecteLine(pergameBuf,selectedLine),getOffsetOfSelecteLine(pergameBuf,selectedLine+1));
					selectedLine++;
				}
				
				
				
				break;
			}else if( padData.Buttons & PSP_CTRL_TRIANGLE ){
				beforeButtons = PSP_CTRL_TRIANGLE;
				//プラグインを選択
				if( fileSelecter(config.basePath,&dirTmp, PPREFSMSG_EDITPERGAME_SELECTPRX, 0, "ccbcccac" ) == 0 ){
					for( i = 0; i+readSize < PERGAMEBUF_SIZE && dirTmp.name[i] != '\0'; i++ ){
						//too big size
						if( i+readSize+1 >= PERGAMEBUF_SIZE ){
							pergameBuf[i+readSize-1] = '\0';
							break;
						}
						pergameBuf[i+readSize] = dirTmp.name[i];
					}
					pergameBuf[i+readSize] = ' ';
					pergameBuf[i+readSize+1] = '\0';
					readSize += i+1;
					//ゲームを選択
					if(  fileSelecter(rootPath,&dirTmp, PPREFSMSG_EDITPERGAME_SELECTGAME, 1, NULL ) == 0  ){
						pnt = dirTmp.name;
						//ISOやCSOなどゲームIDがgetできたら == 10
						if( get_umd_id(idBuf, dirTmp.name, dirTmp.type) == 10 ){
							if( is_idOrPath() == 0 ) pnt = idBuf;
						}
						for( i = 0; pnt[i] != '\0'; i++ ){
							//too big size
							if( i+readSize+1 >= PERGAMEBUF_SIZE ){
								pergameBuf[i+readSize-1] = '\0';
								break;
							}
							pergameBuf[i+readSize] = pnt[i];
						}
						pergameBuf[i+readSize] = '\n';
						pergameBuf[i+readSize+1] = '\0';
						readSize += i+1;
					}
				}
				wait_button_up(&padData);
				break;
			}else if( padData.Buttons & PSP_CTRL_HOME ){
				wait_button_up(&padData);
				if(pergameBuf != NULL ) free(pergameBuf);
				return 0;
			}else if( padData.Buttons & PSP_CTRL_START ){
				saveEditPergame(pergameBuf,pergametxtPath);
				wait_button_up(&padData);
				if(pergameBuf != NULL ) free(pergameBuf);
				return 0;
			}else if( padData.Buttons & PSP_CTRL_RTRIGGER ){
				beforeButtons = PSP_CTRL_RTRIGGER;
				while(1){

					printState = noPrintText(pergameBuf, headOffset, headLine, selectedLine,MAXDISPLAY_Y );
				
					if( printState & SELECTED_BOTTOM && (!(printState & TAIL_IS_EOF)) ){
						i = getOffsetOfNextLine(pergameBuf,headOffset);
						//次の行に移動したなら
						if( i > 0 && pergameBuf[i-1] == '\n' && i != headOffset ) headLine++;
						if( printState & TAIL_IS_N  ) selectedLine++;
						headOffset = i;
					}else if( ! ((printState & TAIL_IS_EOF) && (printState & SELECTED_BOTTOM)) ){
						selectedLine++;
					}else{
						break;
					}
				}			/*
				tmp = getNumberOfLineOfString(pergameBuf);
				headOffset = 0;
				i = 0;
				headLine = 0;
				//
				while( headLine + MAXDISPLAY_Y < tmp ){
					headOffset = getOffsetOfNextLine(pergameBuf,i);
					if( headOffset == i ) break;
					i = headOffset;
					headLine++;
				}
				selectedLine = tmp - 1;
				*/

				/*
				tmp = strlen(pergameBuf);
				for( i = 0; i < MAXDISPLAY_Y; i++ ){
					headOffset = getOffsetOfPreviousLine(pergameBuf,tmp);
					if( headOffset == tmp ) break;
					tmp = headOffset;
				}
				headLine = getNumberOfLineOfStringFormOffset(pergameBuf,headOffset);
				selectedLine = getNumberOfLineOfString(pergameBuf) - 1;
				*/
				/*
				tmp = strlen(pergameBuf);
				headOffset = getOffsetOfPreviousLine(pergameBuf,tmp);
				headLine = getNumberOfLineOfStringFormOffset(pergameBuf,tmp);
				selectedLine = headLine;
				*/
	
				
				wait_button_up(&padData);
				break;
			}else if( padData.Buttons & PSP_CTRL_LTRIGGER ){				
				beforeButtons = PSP_CTRL_LTRIGGER;
				
				headOffset = 0;
				headLine = 0;
				selectedLine = 0;

				wait_button_up(&padData);
				break;
			}else{
				beforeButtons = 0;
			}
		}
	}

	if(pergameBuf != NULL ) free(pergameBuf);
	
	return 0;
}