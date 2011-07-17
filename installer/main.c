//thanks to http://oku.edu.mie-u.ac.jp/~okumura/compression/zlib.html

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <pspreg.h>
#include <psprtc.h>
#include <psputils.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psputility_sysparam.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <zlib.h>

#include <vlf.h>
#include "utf8text.h"

#include "pprefs_jp_bin.h"
#include "pprefs_lite_jp_bin.h"
#include "pprefs_en_bin.h"
#include "pprefs_lite_en_bin.h"


PSP_MODULE_INFO("pprefs_installer", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(0);



int languageNumber = 0;
char rootPath[16];
VlfPicture splashscreen;

int flagToGoBackTop = 0;


int Decompress(void* inbuf, unsigned int inbufSize, void* outbuf, unsigned int outbufSize, const char* outPath)
{
	z_stream z;
	//default
    z.zalloc = NULL;
    z.zfree = NULL;
    z.opaque = NULL;
	
    // init
    if (inflateInit(&z) != Z_OK) {
		return -1;
    }
	
	SceUID fd = sceIoOpen(outPath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if( fd < 0 ){
		inflateEnd(&z);
		return -2;
	}
	
	
	z.next_in = inbuf;
	z.avail_in = inbufSize;
	
	int ret, writtenSize;
    while(1){
		//reset
		z.next_out = outbuf;
		z.avail_out = outbufSize;

		ret = inflate(&z, Z_NO_FLUSH);//decompress
		if( ret != Z_OK && ret != Z_STREAM_END ){//error
			ret = -3;
			break;
		}
		
		writtenSize = sceIoWrite(fd, outbuf, outbufSize - z.avail_out);
		if( writtenSize != (outbufSize - z.avail_out) ){//write error
			ret = -4;
			break;
		}
		
		if( ret == Z_STREAM_END ){//done
			ret = 0;
			break;
		}
    }
	
	sceIoClose(fd);
	inflateEnd(&z);
	return ret;
}






//thanks to takka
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

/*
int copyFile(const char *src, const char *dst)
{
	SceUID fd = -1, fdw = -1;
	int readSize;
	char buf[1024 * 10];

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
 */

//thanks for plum
//int Check_EOF(SceUID fd)
int isEOF(SceUID fd)
{
	char c;

	if(sceIoRead(fd, &c, 1) == 1)
	{
		sceIoLseek(fd, -1, PSP_SEEK_CUR);
		return 0;
	}

	return 1;
}

#define LEN_PER_LINE 256

int install(int sel){
	
	SceUID fd, fdw;

	int ret;
	char line[LEN_PER_LINE];
	char *ptr;
	char pprefsPath[256];
	char pprefsPath_another[256];
	char *vshPath = "ms0:/seplugins/vsh.txt";
	char *vshtmpPath = "ms0:/seplugins/vsh.txt.pprefstmp";
	

	fdw = sceIoOpen(vshtmpPath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
	if( fdw < 0 ){
		return -1;
	}

	fd = sceIoOpen(vshPath, PSP_O_RDONLY, 0777);
	
	strcpy(pprefsPath, rootPath);
	strcat(pprefsPath, (sel == 1 || sel == 3)?"seplugins/pprefs_lite.prx":"seplugins/pprefs.prx");

	strcpy(pprefsPath_another, rootPath);
	strcat(pprefsPath_another, (sel == 1 || sel == 3)?"seplugins/pprefs.prx":"seplugins/pprefs_lite.prx");

	int flag = 0;
	if( fd >= 0 ){
		while( ! isEOF(fd) ){
			read_line_file(fd, line, LEN_PER_LINE - 1);
			if( (ptr = strchr(line,' ')) != NULL ){
				*ptr = '\0';
				if( strcasecmp(line, pprefsPath) == 0 ){
					flag = 1;
					strcat(line, " 1");
				}else if( strcasecmp(line, pprefsPath_another) == 0 ){
					strcat(line, " 0");
				}else{
					*ptr = ' ';
				}
			}
			strcat(line, "\n");
			sceIoWrite(fdw, line, strlen(line));
		}
		sceIoClose(fd);
	}
	
	if( ! flag ){
		strcpy(line, pprefsPath);
		strcat(line, " 1\n");
		sceIoWrite(fdw, line, strlen(line));
	}
	
	
	sceIoClose(fdw);

	sceIoRemove(vshPath);
	sceIoRename(vshtmpPath, vshPath);
	
	
	
	unsigned char *srcData[] = {
		pprefs_jp, pprefs_lite_jp, pprefs_en, pprefs_lite_en
	};
	unsigned int srcSize[] = {
		size_pprefs_jp, size_pprefs_lite_jp, size_pprefs_en, size_pprefs_lite_en
	};
	char *buf;
	unsigned int bufSize = 1024 * 100;
	while(1){
		buf = malloc(bufSize);
		if( buf != NULL ) break;
		bufSize -= 1024;
	}
	ret = Decompress(srcData[sel], srcSize[sel], buf, bufSize, pprefsPath);
	free(buf);
	

	return ret;

}

int menu_sel(int sel)
{
	char *dialogMessage[] = {DONE_JPN, "Done"};
	char *dialogMessageFailed[] = {FAILED_JPN, "Failed"};
	char message[64];
	VlfShadowedPicture waitIcon;
	int ret;
	
	switch (sel)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			
			waitIcon = vlfGuiAddWaitIcon();
			ret = install(sel);
			vlfGuiRemoveShadowedPicture(waitIcon);
			if( ret < 0 ){
				sprintf(message, "%s : %d", dialogMessageFailed[languageNumber], ret);
			}else{
				strcpy(message, dialogMessage[languageNumber]);
			}
				vlfGuiMessageDialog(message, VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_NONE);
//			flagToGoBackTop = 1;
			
//		return VLF_EV_RET_REMOVE_HANDLERS | VLF_EV_RET_REMOVE_OBJECTS;
		return VLF_EV_RET_NOTHING;
		
		case 4:
			sceKernelExitGame();
			return VLF_EV_RET_NOTHING;
		break;
	}
	
	return VLF_EV_RET_NOTHING;
}

void getRootPath(char *dst,char *src)
{
	int i;
	for( i = 0; src[i] != '\0'; i++ ){
		dst[i] = src[i];
		if( dst[i] == '/' ){
			dst[i+1] = '\0';
			return;
		}
	}
	return;
}

void sleep_(clock_t sleepTime)
{
	clock_t time = sceKernelLibcClock();
	
	while( (sceKernelLibcClock()- time) < sleepTime ) vlfGuiDrawFrame();
}

int app_main(int argc, char *argv[])
{
	char *mainmenuItems[][5] = {
		{
			"pprefs(日本語版)インストール",
			"pprefs lite(日本語版)インストール",
			"pprefs(英語版)インストール",
			"pprefs lite(英語版)インストール",
			"終了"
		},{
			"Install pprefs(Japanese ver.)",
			"Install pprefs lite(Japanese ver.)",
			"Install pprefs(English ver.)",
			"Install pprefs lite(English ver.)",
			"Exit"
		}
	};
	
	//init
	languageNumber = vlfGuiGetLanguage();
	if( languageNumber > 0 ) languageNumber = 1;
	getRootPath(rootPath, argv[0]);
	vlfGuiSystemSetup(1, 1, 1);
	
	
	//splashscreen
/*	splashscreen = vlfGuiAddPictureFile("splashscreen.png", 0, 0);
	

	if( splashscreen ){
		sleep_(1 * 1000 * 1000);
		vlfGuiSetPictureFade(splashscreen,VLF_FADE_MODE_OUT, VLF_FADE_SPEED_FAST, 0);
	}
*/	
	vlfGuiSetTitleBar(vlfGuiAddText(0,0,"pprefs ver. 1130 / pprefs_lite ver.1030"),0,1,0);
	
	while(1){
		flagToGoBackTop = 0;
		vlfGuiCentralMenu(5, mainmenuItems[languageNumber], 0, menu_sel, 0, 0);
		
		while(!flagToGoBackTop)
		{
			vlfGuiDrawFrame();
		}
	}
	
	return 0;
}

