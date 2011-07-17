#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <pspusbdevice.h>
#include <pspmodulemgr.h>
#include <pspsdk.h>

#define NELEMS(a) (sizeof(a) / sizeof(a[0]))


struct UsbModule {
	char *name;
	char *path;
	int moid;
};

static struct UsbModule modules[] = {
	{"sceSemawm", "flash0:/kd/semawm.prx", -1},
	{"sceUSB_Stor_Driver", "flash0:/kd/usbstor.prx", -1},
	{"sceUSB_Stor_Mgr_Driver", "flash0:/kd/usbstormgr.prx", -1},
	{"sceUSB_Stor_Ms_Driver", "flash0:/kd/usbstorms.prx", -1},
	{"sceUSB_Stor_Boot_Driver", "flash0:/kd/usbstorboot.prx", -1},
	{"sceUSB_Stor_Boot_Driver", "flash0:/kd/usbstorboot.prx", -1},
};

static char usbStatus = 0;

int StopUnloadModule(SceUID modID){
    int status;
    sceKernelStopModule(modID, 0, NULL, &status, NULL);
    sceKernelUnloadModule(modID);
    return 0;
}

int disableUsb(void)
{
	if(usbStatus)
	{
		
		sceUsbDeactivate(0);
		pspUsbDeviceFinishDevice();
		sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
		sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		
		/*
		int i;
		for(i = 0; i < NELEMS(modules); i++){
			if( modules[i].moid < 0 ) continue;
			StopUnloadModule(modules[i].moid);
		}
		*/
			
		
		sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0 );
		usbStatus = 0;
		sceKernelDelayThread(300000);

	}
	return 0;
}

int enableUsb(void)
{
	if (usbStatus == 1)
	{
		disableUsb();
		return 0;
	}
	
	
	if (!usbStatus)
	{
		int i;
		for(i = 0; i < NELEMS(modules); i++){
			if( sceKernelFindModuleByName(modules[i].name) == NULL ){
				modules[i].moid = pspSdkLoadStartModule(modules[i].path, PSP_MEMORY_PARTITION_KERNEL);
			}
		}
		
		sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
		sceUsbstorBootSetCapacity(0x800000);
		sceUsbActivate(0x1c8);
		usbStatus = 1;
		sceKernelDelayThread(300000);
	}
	return 1;
}


