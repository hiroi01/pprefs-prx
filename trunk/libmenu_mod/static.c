#include "common.h"


u32 (*convert)(u32)			= NULL;
u32 (*blend)(u8,u32,u32)	= NULL;
libm_vram_info vinfo;

int psx = 0,psy = 0;
