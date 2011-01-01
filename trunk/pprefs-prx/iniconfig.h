
#ifndef __INICONFIG__
#define __INICONFIG__

#include "libini.h"

void strutilRemoveChar( char *__restrict__ str, const char *__restrict__ search );
void callbackToSplit(const char *val,void *opt);
bool callbackForIni(const char *key, char *val,void *opt);

#endif
