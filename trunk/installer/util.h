#ifndef __UTIL__
#define __UTIL__

#define NELEMS(a) (sizeof(a) / sizeof(a[0]))

void* malloc_max(unsigned int *size);
int Decompress(void* inbuf, unsigned int inbufSize, void* outbuf, unsigned int outbufSize, const char* outPath);


#endif