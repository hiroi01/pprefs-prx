#include "../common.h"

bool libmDebugScreenSetXY( int x ,int y )
{
	if( x < 0 || x > LIBM_SETX_MAX || y < 0 || y > LIBM_SETY_MAX ) return false;
	
	psx = x * LIBM_CHAR_WIDTH;
	psy = y * LIBM_CHAR_HEIGHT;
	
	return true;
}
