#include "../common.h"

inline void libmFillRect( int sx, int sy, int ex, int ey, u32 color )
{
	void *start_addr, *draw_addr;
	u32 offset;
	int w, h, x;
	
	if( color == LIBM_NO_DRAW ) return;
	
	if( sx > ex ) SWAP( &sx, &ex );
	if( sy > ey ) SWAP( &sy, &ey );
	
	w = ex - sx;
	h = ey - sy;
	
	start_addr = libmMakeDrawAddr(sx, sy);
	
	for( ; h; h--, start_addr += vinfo.lineSize )
	{
		offset = 0;
		
		for( x = 0; x < w; x++, offset += vinfo.pixelSize )
		{
			draw_addr = start_addr + offset;
			libmPoint(draw_addr, color );
		}
	}
}
