#include "common.h"

inline void libmPointEx(void *adr, u32 src)
{
	if( vinfo.format == PSP_DISPLAY_PIXEL_FORMAT_8888 )
	{
		*(u32*)adr = src;
	}
	else
	{
		*(u16*)adr = src;
	}
}