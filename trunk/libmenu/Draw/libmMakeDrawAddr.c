#include "../common.h"

inline void* libmMakeDrawAddr( int x, int y )
{
	return (void*)( vinfo.buffer + (( x + y * vinfo.lineWidth ) * vinfo.pixelSize) );
}
