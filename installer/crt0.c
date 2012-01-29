#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>
#include <malloc.h>

#include <vlf.h>
#include "util.h"

#include "vlf_bin.h"
#include "intraFontJPN_bin.h"
#include "iop_bin.h"


struct modules_data{
	void* data;
	unsigned int dataSize;
	char* path;
};


struct modules_data2 {
	void *data;
	unsigned int dataSize;
};


extern int app_main(int argc, char *argv[]);

void printErrorMessage(int errorNum)
{
	pspDebugScreenInit();

	pspDebugScreenPutChar(5 + 8 * 0, 5, 0xffffffff, 'e');
	pspDebugScreenPutChar(5 + 8 * 1, 5, 0xffffffff, 'r');
	pspDebugScreenPutChar(5 + 8 * 2, 5, 0xffffffff, 'r');
	pspDebugScreenPutChar(5 + 8 * 3, 5, 0xffffffff, 'o');
	pspDebugScreenPutChar(5 + 8 * 4, 5, 0xffffffff, 'r');
	pspDebugScreenPutChar(5 + 8 * 5, 5, 0xffffffff, ':');
	
	if( errorNum < 0 ){
		errorNum = -errorNum;
		pspDebugScreenPutChar(5 + 8 * 7, 5, 0xffffffff, '-');
	}
	if( errorNum > 9 ){
		errorNum = 0;
	}
	char printedChar = '0' + errorNum;
	pspDebugScreenPutChar(5 + 8 * 8, 5, 0xffffffff, printedChar);			
}



int loadModules(SceSize args, void *argp)
{
	SceUID mod;
	int i;
	/*
	struct modules_data2 modules[] = {
		{iop, size_iop},
		{intraFontJPN, size_intraFontJPN},
		{vlf, size_vlf}
	};
	
	for( i = 0; i < NELEMS(modules); i++ ){
		mod = sceKernelLoadModuleBuffer((void *)modules[i].dataSize, (SceSize)modules[i].data, 0, NULL);
		if( mod < 0 ){
			return -6;
		}
		
		mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
		if( mod < 0 ){
			return -7;
		}
	}
	
	return 0;
	*/
	
	
	struct modules_data modules[] = {
		{iop, size_iop, "iop.prx"},
		{intraFontJPN, size_intraFontJPN, "intraFontJPN.prx"},
		{vlf, size_vlf, "vlf.prx"}
	};

	for( i = 0; i < NELEMS(modules); i++ ){
		mod = sceKernelLoadModule(modules[i].path, 0, NULL);
		if( mod < 0 ){
			SceUID fd = sceIoOpen(modules[i].path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
			if( fd < 0 ) return -1;
			sceIoWrite(fd, modules[i].data, modules[i].dataSize);
			sceIoClose(fd);
			
			mod = sceKernelLoadModule(modules[i].path, 0, NULL);
			if( mod < 0 ) return -6;
		}
		mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
		if( mod < 0 ) return -7;		
	}
	return 0;
	
	/*
	for( i = 0; i < NELEMS(modules); i++ ){
		mod = sceKernelLoadModule(modules[i].path, 0, NULL);
		if( mod < 0 ){
			buf = malloc_max(&bufSize);
			if( buf == NULL ) return -1;
			ret = Decompress(modules[i].data, modules[i].dataSize, buf, bufSize, modules[i].path);
			free(buf);
			if( ret < 0 ) return -1 + ret;
			mod = sceKernelLoadModule(modules[i].path, 0, NULL);
			if( mod < 0 ) return -6;
		}
		mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
		if( mod < 0 ) return -7;
	}
	return 0;
	 */
}

int start_thread(SceSize args, void *argp)
{
	char *path = (char *)argp;
	int last_trail = -1;
	int i;
	
	if (path)
	{
		for (i = 0; path[i]; i++)
		{
			if (path[i] == '/')
				last_trail = i;
		}
	}

	if (last_trail >= 0)
		path[last_trail] = 0;

	sceIoChdir(path);
	path[last_trail] = '/';
	
	int ret = loadModules(args, argp);
	if( ret  < 0 ){
		printErrorMessage(ret);
		sceKernelDelayThread(3 * 1000 * 1000);
		sceKernelExitGame();
	}
	
	vlfGuiInit(15000, app_main);
	
	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	SceUID thid = sceKernelCreateThread("start_thread", start_thread, 0x10, 0x4000, 0, NULL);
	if (thid < 0)
		return thid;

	sceKernelStartThread(thid, args, argp);
	
	return 0;
}

