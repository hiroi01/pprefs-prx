//Thank you plum

#include <pspkernel.h>
#include <systemctrl.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspsysmem_kernel.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int (*sceKernelAllocPartitionMemory_Real)(SceUID id, const char *name, int type, SceSize size, void *addr);
void *(*sceKernelGetBlockHeadAddr_Real)(SceUID blockid);
int (*sceKernelFreePartitionMemory_Real)(SceUID blockid);
//int (*sceKernelTotalFreeMemSize_Real)(void);
int (*sceKernelDevkitVersion_Real)(void);
int (*sceKernelGetModel_Real)(void);
time_t (*sceKernelLibcTime_Real)(time_t *t);

int (*sceCtrlPeekBufferPositive_Real)(SceCtrlData *pad_data, int count);
int (*sceCtrlSetSamplingCycle_Real)(int cycle);
int (*sceCtrlSetSamplingMode_Real)(int mode);

int (*sceDisplayGetFrameBuf_Real)(void **topaddr, int *bufferwidth, int *pixelformat, int sync);
int (*sceDisplaySetFrameBuf_Real)(void *topaddr, int bufferwidth, int pixelformat, int sync);
int (*sceDisplayWaitVblankStart_Real)(void);

// Thanks to Davee
#define PSP_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)

u32 Get_GetModelNid()
{
	int ver = sceKernelDevkitVersion_Real();
	
	if(ver == PSP_FIRMWARE(0x500) || ver == PSP_FIRMWARE(0x550))
	{
		return 0xDA07DC6E;
	}
	else if(ver == PSP_FIRMWARE(0x620))
	{
		return 0x864EBFD7;
	}
	else if(ver == PSP_FIRMWARE(0x631) || ver == PSP_FIRMWARE(0x635))
	{
		return 0x458A70B5;
	}
	
	return (u32)sceKernelGetModel;
}


void nidResolve(void)
{
	sceKernelDevkitVersion_Real = (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x3FC9AE6A);

	//resolve only 6.35
	if( sceKernelDevkitVersion_Real() == PSP_FIRMWARE(0x635) ){
		sceKernelAllocPartitionMemory_Real		= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x237DBD4F);
		sceKernelGetBlockHeadAddr_Real			= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x9D9A5BA1);
		sceKernelFreePartitionMemory_Real		= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0xB6D61D02);
//		sceKernelTotalFreeMemSize_Real		= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0xF919F628);
		sceKernelGetModel_Real					= (void *)FindProc("sceSystemMemoryManager", "SysMemForKernel", Get_GetModelNid());
		sceKernelLibcTime_Real					= (void *)FindProc("sceSystemMemoryManager", "UtilsForUser", 0x27CC57F0);

		sceCtrlPeekBufferPositive_Real			= (void *)FindProc("sceController_Service", "sceCtrl_driver", 0x18654FC0);
		sceCtrlSetSamplingCycle_Real			= (void *)FindProc("sceController_Service", "sceCtrl_driver", 0x855C255D);
		sceCtrlSetSamplingMode_Real				= (void *)FindProc("sceController_Service", "sceCtrl_driver", 0x6CB49301);

		sceDisplayGetFrameBuf_Real				= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0x08A10838);
		sceDisplaySetFrameBuf_Real				= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0x37533141);
		sceDisplayWaitVblankStart_Real			= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0xC30D327D);
	}else{

		sceKernelAllocPartitionMemory_Real		= sceKernelAllocPartitionMemory;
		sceKernelGetBlockHeadAddr_Real			= sceKernelGetBlockHeadAddr;
		sceKernelFreePartitionMemory_Real		= sceKernelFreePartitionMemory;
//		sceKernelTotalFreeMemSize_Real		= sceKernelTotalFreeMemSize;
		sceKernelGetModel_Real					= sceKernelGetModel;
		sceKernelLibcTime_Real					= sceKernelLibcTime;

		sceCtrlPeekBufferPositive_Real			= sceCtrlPeekBufferPositive;
		sceCtrlSetSamplingCycle_Real			= sceCtrlSetSamplingCycle;
		sceCtrlSetSamplingMode_Real				= sceCtrlSetSamplingMode;

		sceDisplayGetFrameBuf_Real				= sceDisplayGetFrameBuf;
		sceDisplaySetFrameBuf_Real				= sceDisplaySetFrameBuf;
		sceDisplayWaitVblankStart_Real			= sceDisplayWaitVblankStart;
	}

}



/*
void nidResolve(void)
{
	sceKernelDevkitVersion_Real = (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x3FC9AE6A);




	sceKernelAllocPartitionMemory_Real	= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x237DBD4F);
	sceKernelGetBlockHeadAddr_Real		= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x9D9A5BA1);
	sceKernelFreePartitionMemory_Real	= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0xB6D61D02);
	sceKernelTotalFreeMemSize_Real		= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0xF919F628);
	sceKernelDevkitVersion_Real			= (void *)FindProc("sceSystemMemoryManager", "SysMemUserForUser", 0x3FC9AE6A);
	sceKernelGetModel_Real				= (void *)FindProc("sceSystemMemoryManager", "SysMemForKernel", Get_GetModelNid());
	sceKernelLibcTime_Real				= (void *)FindProc("sceSystemMemoryManager", "UtilsForUser", 0x27CC57F0);
	

	if( sceKernelDevkitVersion_Real() == PSP_FIRMWARE(0x635) ){
		sceCtrlPeekBufferPositive_Real	= (void *)FindProc("sceController_Service", "sceCtrl_driver", 0x18654FC0);
		sceCtrlSetSamplingCycle_Real	= (void *)FindProc("sceController_Service", "sceCtrl_driver", 0x855C255D);
		sceCtrlSetSamplingMode_Real		= (void *)FindProc("sceController_Service", "sceCtrl_driver", 0x6CB49301);

		sceDisplayGetFrameBuf_Real		= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0x08A10838);
		sceDisplaySetFrameBuf_Real		= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0x37533141);
		sceDisplayWaitVblankStart_Real	= (void *)FindProc("sceDisplay_Service", "sceDisplay_driver", 0xC30D327D);
	}else{
		sceCtrlPeekBufferPositive_Real	= sceCtrlPeekBufferPositive;
		sceCtrlSetSamplingCycle_Real	= sceCtrlSetSamplingCycle;
		sceCtrlSetSamplingMode_Real		= sceCtrlSetSamplingMode;

		sceDisplayGetFrameBuf_Real		= sceDisplayGetFrameBuf;
		sceDisplaySetFrameBuf_Real		= sceDisplaySetFrameBuf;
		sceDisplayWaitVblankStart_Real	= sceDisplayWaitVblankStart;

		
//		sceCtrlPeekBufferPositive_Real	= (void *)FindProc("sceController_Service", "sceCtrl", 0x3A622550);
//		sceCtrlSetSamplingCycle_Real	= (void *)FindProc("sceController_Service", "sceCtrl", 0x6A2774F3);
//		sceCtrlSetSamplingMode_Real		= (void *)FindProc("sceController_Service", "sceCtrl", 0x1F4011E6);
		
//		sceDisplayGetFrameBuf_Real		= (void *)FindProc("sceDisplay_Service", "sceDisplay", 0xEEDA2E54);
//		sceDisplaySetFrameBuf_Real		= (void *)FindProc("sceDisplay_Service", "sceDisplay", 0x289D82FE);
//		sceDisplayWaitVblankStart_Real	= (void *)FindProc("sceDisplay_Service", "sceDisplay", 0x984C27E7);
		
	}


}

*/


