#include "../common.h"

void libmClearBuffers()
{
	memset(vinfo.buffer ,0, vinfo.frameSize );
}
