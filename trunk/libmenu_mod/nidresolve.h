#ifndef __NID_RESOLVE_H_
#define __NID_RESOLVE_H_


//ñ≥óùÇ‚ÇËÉ]ÉãÉo

extern int (*sceDisplayGetFrameBuf_Real)(void **topaddr, int *bufferwidth, int *pixelformat, int sync);
extern int (*sceDisplaySetFrameBuf_Real)(void *topaddr, int bufferwidth, int pixelformat, int sync);
extern int (*sceDisplayWaitVblankStart_Real)(void);

#define sceDisplayGetFrameBuf sceDisplayGetFrameBuf_Real
#define sceDisplaySetFrameBuf sceDisplaySetFrameBuf_Real
#define sceDisplayWaitVblankStart sceDisplayWaitVblankStart_Real


void libmenu_nidResolve(void);
#endif
