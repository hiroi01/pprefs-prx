#include "common.h"



inline void libmLine( int sx, int sy, int ex, int ey, uint32_t color )
{
	void *draw_addr;
	int       e, dx, dy;
	
	if( color == LIBM_NO_DRAW ) return;
	
	if( sx > ex ) SWAP( &sx, &ex );
	if( sy > ey ) SWAP( &sy, &ey );
	
	draw_addr = libmMakeDrawAddr( sx, sy );
	
	if( sx == ex )
	{
		for( ; sy <= ey; sy++, draw_addr += vinfo.lineSize ) libmPoint( draw_addr, color );
	}
	else if( sy == ey )
	{
		for( ; sx <= ex; sx++, draw_addr += vinfo.pixelSize ) libmPoint( draw_addr, color );
	}
	else
	{
		e  = 0;
		dx = ex - sx;
		dy = ey - sy;
		
		if( dx > dy )
		{
			int x;
			
			for( x = sx; x <= ex; x++, draw_addr += vinfo.pixelSize )
			{
				e += dy;
				
				if( e > dx )
				{
					e -= dx;
					draw_addr += vinfo.lineSize;
				}
				
				libmPoint( draw_addr, color );
			}
		}
		else
		{
			int y;
			
			for( y = sy; y <= ey; y++, draw_addr += vinfo.lineSize )
			{
				e += dx;
				
				if( e > dy )
				{
					e -= dy;
					draw_addr += vinfo.pixelSize;
				}
				
				libmPoint( draw_addr, color );
			}
		}
	}
}
