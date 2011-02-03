// スレッド

// ヘッダー
#include <pspthreadman.h>
#include "thread.h"
#include <stdbool.h>
#include "nidresolve.h"

// グローバル変数
static int first_th[MAX_THREAD];
static int first_count;

static int current_th[MAX_THREAD];
static int current_count;

static bool threadsState = false;// == ture : suspend / == false : resume

static clock_t safelySuspendTime;





void safelySuspendThreadsInit()
{
	safelySuspendTime = sceKernelLibcClock();
}

void safelySuspendThreads( clock_t waitTime )
{
	if( ! threadsState ){
		if( (sceKernelLibcClock() - safelySuspendTime) >= waitTime ){
			Suspend_Resume_Threads(SUSPEND_MODE);
//			suspendThreads()
		}
	}
	
}

void suspendThreads()
{
	if( ! threadsState ){
		Suspend_Resume_Threads(SUSPEND_MODE);
		threadsState = true;
	}	
}

void resumeThreads()
{
	if( threadsState ){
		Suspend_Resume_Threads(RESUME_MODE);
		threadsState = false;
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

