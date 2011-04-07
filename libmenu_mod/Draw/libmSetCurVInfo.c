#include "common.h"

void libmSetCurVInfo(int format, void *buf, int width)
{
	vinfo.format	= format;
	vinfo.buffer	= buf;
	vinfo.width		= width;
}
