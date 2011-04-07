#include "common.h"

inline void libmFrame( int sx, int sy, int ex, int ey, u32 color )
{
	if( color == LIBM_NO_DRAW ) return;
	
	libmLine( sx, sy, ex, sy, color );
	libmLine( ex, sy, ex, ey, color );
	libmLine( ex, ey, sx, ey, color );
	libmLine( sx, ey, sx, sy, color );
}
