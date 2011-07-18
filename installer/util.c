#include <pspsdk.h>
#include <pspkernel.h>

#include <malloc.h>
#include <zlib.h>

#if 0
void* malloc_max(unsigned int *size)
{
	void *buf;
	
	while(1){
		buf = malloc(*size);
		if( buf != NULL ) break;
		*size -= 1024;
		if( *size <= 0 ) break;
	}
	
	return buf;
	
}
#endif


int Decompress(void* inbuf, unsigned int inbufSize, void* outbuf, unsigned int outbufSize, const char* outPath)
{
	int ret;
	z_stream z;
	//default
    z.zalloc = NULL;
    z.zfree = NULL;
    z.opaque = NULL;

    // init
	z.next_in = Z_NULL;
	z.avail_in = 0;
	ret = inflateInit(&z);
    if (ret != Z_OK) {
		return -1;
    }
	
	SceUID fd = sceIoOpen(outPath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if( fd < 0 ){
		inflateEnd(&z);
		return -2;
	}
	
	
	z.next_in = inbuf;
	z.avail_in = inbufSize;
	
	int writtenSize;
    while(1){
		//reset
		z.next_out = outbuf;
		z.avail_out = outbufSize;
		
		ret = inflate(&z, Z_NO_FLUSH);//decompress
		if( ret != Z_OK && ret != Z_STREAM_END ){//error
			ret = -3;
			break;
		}
		
		writtenSize = sceIoWrite(fd, outbuf, outbufSize - z.avail_out);
		if( writtenSize != (outbufSize - z.avail_out) ){//write error
			ret = -4;
			break;
		}
		
		if( ret == Z_STREAM_END ){//done
			ret = 0;
			break;
		}
    }
	
	sceIoClose(fd);
	inflateEnd(&z);
	return ret;
}
