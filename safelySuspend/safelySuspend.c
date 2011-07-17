/*
sefelySuspend by hiroi01


Thanks for plum

and, refered to the code of VshCtrl in procfw





 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .

*/


// ヘッダー
#include <pspthreadman.h>
#include "safelySuspend.h"
#include <string.h>

#define arraynumberof(array) (sizeof(array) / sizeof(array[0]))

// グローバル変数
static int first_th[MAX_THREAD];
static int first_count;

static int current_th[MAX_THREAD];
static int current_count;

static int threadsState = 0;// != 0 : suspend / == 0 : resume

static clock_t safelySuspendTime;


int isExistScePafJob(void)
{
	int i;
	//get threads list
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, current_th, MAX_THREAD, &current_count);
	
	for(i = 0; i < current_count; i++){
		SceKernelThreadInfo info;
		
		info.size = sizeof(info);
		
		if( sceKernelReferThreadStatus(current_th[i], &info) < 0 ){
			continue;
		}
		
		if( ! strcmp(info.name, "ScePafJob") ) {
				return 1;
		}
	}
	
	return 0;
}


#if 0
static char *checkThreadName[] = {
	"SceNpSignupEvent",
	"VshCacheIoPrefetchThread",
	"VideoDecoder",
	"AudioDecoder",
	"ScePafJob",
	"ScePSStoreBrowser2",
	"SceNetDhcpClient",
};


int safelySuspendForVSH()
{
	int i, n;
	SceUID my_thid;
	
	
	if( sceKernelFindModuleByName("sceUSB_Stor_Driver") ){
		return 1;
	}
	
	//get threads list
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, current_th, MAX_THREAD, &current_count);
	
	for(i = 0; i < current_count; i++){
		SceKernelThreadInfo info;
		
		info.size = sizeof(info);

		if( sceKernelReferThreadStatus(current_th[i], &info) < 0 ){
			continue;
		}
		
		for( n = 0; n < arraynumberof(checkThreadName); n++ ){
			if( ! strcmp(info.name, checkThreadName[n]) ) {
				return 2;
			}
		}
	}
	
	my_thid = sceKernelGetThreadId();
	
	//suspend threads
	for(i = 0; i < current_count; i++){

		for(n = 0; n < first_count; n++){
			if(current_th[i] == first_th[n])
			{
				current_th[i] = 0;
				break;
			}
		}
		
		if(current_th[i] != my_thid)
			sceKernelSuspendThread(current_th[i]);
	}
	
	threadsState = 1;
	return 0;
}
#endif


void safelySuspendThreadsInit()
{
	safelySuspendTime = sceKernelLibcClock();
}

int safelySuspendThreads( clock_t waitTime )
{
	if( ! threadsState ){
		if( (sceKernelLibcClock() - safelySuspendTime) >= waitTime ){
//			Suspend_Resume_Threads(SUSPEND_MODE);
			suspendThreads();
			return 1;
		}
		return -1;
	}
	
	return 0;
}

void suspendThreads()
{
	if( ! threadsState ){
		Suspend_Resume_Threads(SUSPEND_MODE);
		threadsState = 1;
	}	
}

void resumeThreads()
{
	if( threadsState ){
		Suspend_Resume_Threads(RESUME_MODE);
		threadsState = 0;
	}
}

// 関数
void Get_FirstThreads()
{
	// スレッド一覧を取得
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, first_th, MAX_THREAD, &first_count);
}

void Suspend_Resume_Threads(int mode)
{
	int i, n;
	SceUID my_thid;
	SceUID (*Thread_Func)(SceUID) = NULL;

	my_thid = sceKernelGetThreadId();
	Thread_Func = (mode == RESUME_MODE ? sceKernelResumeThread : sceKernelSuspendThread);

	// スレッド一覧を取得
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, current_th, MAX_THREAD, &current_count);

	for(i = 0; i < current_count; i++)
	{
		for(n = 0; n < first_count; n++)
		{
			if(current_th[i] == first_th[n])
			{
				current_th[i] = 0;
				n = first_count;
			}
		}

		if(current_th[i] != my_thid)
			Thread_Func(current_th[i]);
	}

	return;
}

