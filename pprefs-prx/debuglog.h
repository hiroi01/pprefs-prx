
#ifndef ___DEBUG_LOG__H___
#define ___DEBUG_LOG__H___





#if DEBUG >= 1

#include <stdio.h>
#define dbgprintf(format, ... ) \
printf( "[%s(%s:%d)]: "format, __func__, __FILE__, __LINE__,  ##__VA_ARGS__)

#else

#define dbgprintf(format, ... ) do{}while(0)

#endif





#endif
