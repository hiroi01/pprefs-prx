#ifndef __NID_RESOLVE_H_
#define __NID_RESOLVE_H_


#include <pspctrl.h>


//ñ≥óùÇ‚ÇËÉ]ÉãÉo

extern int (*sceKernelAllocPartitionMemory_Real)(SceUID id, const char *name, int type, SceSize size, void *addr);
extern void *(*sceKernelGetBlockHeadAddr_Real)(SceUID blockid);
extern int (*sceKernelFreePartitionMemory_Real)(SceUID blockid);
extern int (*sceKernelTotalFreeMemSize_Real)(void);
extern int (*sceKernelDevkitVersion_Real)(void);
extern int (*sceKernelGetModel_Real)(void);
extern time_t (*sceKernelLibcTime_Real)(time_t *t);

extern int (*sceCtrlPeekBufferPositive_Real)(SceCtrlData *pad_data, int count);
extern int (*sceCtrlSetSamplingCycle_Real)(int cycle);
extern int (*sceCtrlSetSamplingMode_Real)(int mode);

extern int (*sceDisplayGetFrameBuf_Real)(void **topaddr, int *bufferwidth, int *pixelformat, int sync);
extern int (*sceDisplaySetFrameBuf_Real)(void *topaddr, int bufferwidth, int pixelformat, int sync);
extern int (*sceDisplayWaitVblankStart_Real)(void);


#define sceKernelAllocPartitionMemory sceKernelAllocPartitionMemory_Real
#define sceKernelGetBlockHeadAddr sceKernelGetBlockHeadAddr_Real
#define sceKernelFreePartitionMemory sceKernelFreePartitionMemory_Real
#define sceKernelTotalFreeMemSize sceKernelTotalFreeMemSize_Real
#define sceKernelDevkitVersion sceKernelDevkitVersion_Real
#define sceKernelGetModel sceKernelGetModel_Real
#define sceKernelLibcTime sceKernelLibcTime_Real

#define sceCtrlPeekBufferPositive sceCtrlPeekBufferPositive_Real
#define sceCtrlSetSamplingCycle sceCtrlSetSamplingCycle_Real
#define sceCtrlSetSamplingMode sceCtrlSetSamplingMode_Real

#define sceDisplayGetFrameBuf sceDisplayGetFrameBuf_Real
#define sceDisplaySetFrameBuf sceDisplaySetFrameBuf_Real
#define sceDisplayWaitVblankStart sceDisplayWaitVblankStart_Real




void nidResolve(void);

#endif
