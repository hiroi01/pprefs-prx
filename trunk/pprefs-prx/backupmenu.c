#include "common.h"
#include "button.h"
#include "pprefsmenu.h"
#include "sepluginstxt.h"
#include "language.h"

static const char *textFileName[] = {
	"vsh",
	"game",
	"pops"
};


char* getBaseName(char *str)
{
	int len;
	for( len = strlen(str) - 1; len >= 0; len-- ){
		if( str[len] == '/' ) return &str[len+1];
	}
	return NULL;
}

#define printALine(i) \
{ \
	libmPrint( \
		15 + (LIBM_CHAR_WIDTH*2), 38 + (i)*(LIBM_CHAR_HEIGHT+2), \
		isExist[i]?FG_COLOR:OF_COLOR, BG_COLOR, \
		backupFilePath[i] \
	); \
	if(cursor == i){ \
		libmPrint( \
			15, 38 + (i)*(LIBM_CHAR_HEIGHT+2), \
			FG_COLOR, BG_COLOR, \
			">" \
		); \
	}else{ \
		libmPrint( \
			15, 38 + (i)*(LIBM_CHAR_HEIGHT+2), \
			FG_COLOR, BG_COLOR, \
			" " \
		); \
	} \
}


/*
void cursorDelay(u32 buttons, clock_t waitTime)
{
	
	if( (beforeButtons & (buttons) ) == (buttons) ){
		if( firstFlag ){
			if( (sceKernelLibcClock() - time) >= (firstWait) ){
				time = sceKernelLibcClock();
				firstFlag = false;
			}else{
				continue;
			}
		}else{
			if( (sceKernelLibcClock() - time) >= (wait) ){
				time = sceKernelLibcClock();
			}else{
				continue;
			}
		}
	}else{
		firstFlag = 1;
		beforeButtons = buttons;
		time = sceKernelLibcClock();
	}

}
*/

int copyFile(const char *src, const char *dst)
{
	SceUID fd = -1, fdw = -1;
	int readSize;
	char *buf = commonBuf;

	fd = sceIoOpen(src, PSP_O_RDONLY, 0777);
	if (fd < 0){
		goto error;
	}

	fdw = sceIoOpen(dst, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fdw < 0) {
		goto error;
	}


	while(1){
		readSize = sceIoRead(fd, buf, sizeof(buf));
		if( ! (readSize > 0) ) break;
		if( sceIoWrite(fdw, buf, readSize) != readSize ){
			goto error;
		}
	}


	sceIoClose(fd);
	sceIoClose(fdw);

	return 0;

error:
	if( fd > 0 ) sceIoClose(fd);
	if( fdw > 0 ) sceIoClose(fdw);

	return -1;
}

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

int backupmenu(char *basePath, int *now_type)
{
	char backupFilePath[10][128];
	SceIoStat stat;
	int isExist[10];
	char baseName[64];
	int cursor = 0, beforeCursor;
	int i;
	clock_t time = 0;
	u32 beforeButtons = 0;
	bool firstFlag = true;

	while(1){
		PRINT_SCREEN();
		libmPrintf(0, 264, EX_COLOR, BG_COLOR, PPREFSMSG_BACKUPMENU_HOWOTOUSE, buttonData[buttonNum[0]].name);
		for( i = 0; i < 10; i++ ){
			sprintf(baseName, "%s_%d.txt", textFileName[*now_type], i);
			strcpy(backupFilePath[i], basePath);
			strcat(backupFilePath[i], baseName);
			isExist[i] = (sceIoGetstat(backupFilePath[i], &stat) == 0)?1:0;
			printALine(i);
		}
		
		while(1){
			get_button(&padData);
			if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP) )
			{
				ALLORW_WAIT((PSP_CTRL_DOWN|PSP_CTRL_UP), 3 * 100 * 1000, 1 * 100 * 1000);
				
				beforeCursor = cursor;
				if( padData.Buttons & PSP_CTRL_DOWN ){
					cursor++;
					if( cursor > 9 ) cursor = 0;
				}else{
					cursor--;
					if( cursor < 0 ) cursor = 9;
				}
				printALine(cursor);
				printALine(beforeCursor);
			}
			else if( padData.Buttons & buttonData[buttonNum[0]].flag )
			{
				if( beforeButtons & buttonData[buttonNum[0]].flag ) continue;
				beforeButtons = buttonData[buttonNum[0]].flag;
				char *menu[] = PPREFSMSG_BACKUPMENU;
				int sel;
				sel = pprefsMakeSelectBox(24, 70, "menu", menu, buttonData[buttonNum[0]].flag, 1);
				if( sel == 0 ){
					if( isExist[cursor] ){
						readSepluginsText(4, 0, backupFilePath[cursor]);
						wait_button_up(&padData);
						return 1;
					}else{
						char *menu[] = { "OK", NULL };
						pprefsMakeSelectBox(24,  40, PPREFSMSG_BACKUPMENU_NOTEXITST ,menu, buttonData[buttonNum[0]].flag, 0);
					}
				}else if( sel == 1){
					char srcPath[128];
					sprintf(srcPath,"%s%s.txt", basePath, textFileName[*now_type]);
					copyFile(srcPath, backupFilePath[cursor]);
				}
				
				wait_button_up(&padData);
				break;
			}
			else if( padData.Buttons & PSP_CTRL_RTRIGGER )
			{
				if( (beforeButtons & PSP_CTRL_LTRIGGER) ) continue;
				beforeButtons = PSP_CTRL_RTRIGGER;
				
				if( *now_type == 0 ) *now_type = 1;
				else if( *now_type == 1 ) *now_type = 2;
				else if( *now_type == 2 ) *now_type = 0;
				
				wait_button_up(&padData);
				break;
			}
			else if( padData.Buttons & PSP_CTRL_LTRIGGER )
			{
				if( (beforeButtons & PSP_CTRL_LTRIGGER) ) continue;
				beforeButtons = PSP_CTRL_LTRIGGER;
				
				if( *now_type == 0 ) *now_type = 2;
				else if( *now_type == 1 ) *now_type = 0;
				else if( *now_type == 2 ) *now_type = 1;

				wait_button_up(&padData);
				break;
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				wait_button_up(&padData);
				return 0;
			}else{
				beforeButtons = 0;
			}
		}
	}

}
