#include "../common.h"

inline u32 libmGetColor(void *addr)
{
	return  (vinfo.format == PSP_DISPLAY_PIXEL_FORMAT_8888 ? *(u32*)addr : *(u16*)addr);
}