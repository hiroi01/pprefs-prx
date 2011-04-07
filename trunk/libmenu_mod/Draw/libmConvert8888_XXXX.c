#include "common.h"

inline u32 libmConvert8888_5650( u32 src )
{
	return ((src & 0x00f80000) >> 8) | ((src & 0x0000fc00) >> 5) | ((src & 0x000000f8) >> 3);
}

inline u32 libmConvert8888_5551( u32 src )
{
	return (( ( src >> 24 ) ? 1 : 0 ) << 15) | ((src & 0x00f80000) >> 9 ) | ((src & 0x0000f800) >> 6 ) | ((src & 0x000000f8) >> 3 );
}

inline u32 libmConvert8888_4444( u32 src )
{
	return ((src & 0xf0000000) >> 16) | ((src & 0x00f00000) >> 12) | ((src & 0x0000f000) >> 8 ) | ((src & 0x000000f0) >> 4 );
}

