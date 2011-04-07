//Thank you plum

#include <pspkernel.h>
#include <systemctrl.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspsysmem_kernel.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*-------------------------------------------------------------------*/

static int (*sceKernelDevkitVersion_Real)(void);
static int alreadyResolved = 0;

int (*sceDisplayGetFrameBuf_Real)(void **topaddr, int *bufferwidth, int *pixelformat, int sync);
int (*sceDisplaySetFrameBuf_Real)(void *topaddr, int bufferwidth, int pixelformat, int sync);
int (*sceDisplayWaitVblankStart_Real)(void);

/*-------------------------------------------------------------------*/

// Thanks to Davee
#define PSP_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)

/*-------------------------------------------------------------------*/

void libmenu_nidResolve(void)
{
	if( alreadyResolved ) return;
	
	sceKernelDevkitVersion_Real = (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x3FC9AE6A);

	//resolve only 6.35
	if( sceKernelDevkitVersion_Real() == PSP_FIRMWARE(0x635) ){
		sceDisplayGetFrameBuf_Real				= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0x08A10838);
		sceDisplaySetFrameBuf_Real				= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0x37533141);
		sceDisplayWaitVblankStart_Real			= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0xC30D327D);
	}else{
		sceDisplayGetFrameBuf_Real				= sceDisplayGetFrameBuf;
		sceDisplaySetFrameBuf_Real				= sceDisplaySetFrameBuf;
		sceDisplayWaitVblankStart_Real			= sceDisplayWaitVblankStart;
	}
	
	alreadyResolved = 1;

}





