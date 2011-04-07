#include "common.h"

//Ç‡Ç§SwapÇ∑ÇÁÇµÇƒÇ»Ç¢ÅEÅEÅE

void libmSwapBuffers()
{
	sceDisplaySetFrameBuf( vinfo.buffer , vinfo.lineWidth , vinfo.format , PSP_DISPLAY_SETBUF_IMMEDIATE );
	sceDisplayWaitVblankStart();
	sceKernelDelayThread( 11500 );
}
