
#include "button.h"
#include "nidresolve.h"

static SceCtrlData data;

void waitButtonUp(void)
{
  do{
    sceCtrlPeekBufferPositive( &data, 1 );
  }while((data.Buttons & CHEACK_KEY_2) != 0);
}

void waitAnyButtonDown(unsigned int key)
{
	while(1){
		get_button(&data);
		if( data.Buttons & key ) break;
	}
}

void waitButtonDown(unsigned int key)
{
	while(1){
		get_button(&data);
		if((data.Buttons & key) == key) break;
	}
}


void waitButtonUpEx(unsigned int exception_key)
{
  do{
    sceCtrlPeekBufferPositive( &data, 1 );
  }while((data.Buttons & (CHEACK_KEY_2 & ~exception_key)) != 0);
}

bool isButtonDown(unsigned int key)
{
	get_button(&data);
	if((data.Buttons & key) == key) return true;
	return false;
}









void get_button(SceCtrlData *data)
{
  sceCtrlPeekBufferPositive( data, 1 );
  data->Buttons &= CHEACK_KEY_2;
}






void wait_button_up_multithread(SceCtrlData *data)
{
	while((data->Buttons & CHEACK_KEY_2) != 0)
	{
		sceCtrlPeekBufferPositive( data, 1 );
		sceKernelDelayThread( 50000 );
	}
}



void wait_button_up(SceCtrlData *data)
{
  while((data->Buttons & CHEACK_KEY_2) != 0)
  {
    sceCtrlPeekBufferPositive( data, 1 );
  }
}

void wait_button_down(SceCtrlData *data,unsigned int key)
{
	while(1){
		get_button(data);
		if((data->Buttons & key) == key) break;
	}
}


void wait_any_button_down(SceCtrlData *data,unsigned int key)
{
	while(1){
		get_button(data);
		if( data->Buttons & key ) break;
	}
}

void wait_button_up_ex(SceCtrlData *data,unsigned int exception_key)
{
  while((data->Buttons & (CHEACK_KEY_2 & ~exception_key)) != 0)
  {
    sceCtrlPeekBufferPositive( data, 1 );
  }
}

bool is_button_down(SceCtrlData *data,unsigned int key)
{
	get_button(data);
	if((data->Buttons & key) == key) return true;
	return false;
}

//made by plum
//thank you plum
// 関数
int Count_Buttons(u32 buttons, int count)
{
	SceCtrlData pad;
	clock_t time;

	// 指定するボタンをセット
	pad.Buttons = buttons;

	// 現在の時間 + 指定した時間
	time = sceKernelLibcClock() + count;

	// ボタンが離れるまでループ
	while((pad.Buttons & buttons) == buttons)
	{
		// ディレイ
		sceKernelDelayThread(50000);

		// パッド情報を取得する
//		sceCtrlReadBufferPositive(&pad, 1);
		sceCtrlPeekBufferPositive(&pad, 1);
		// 現在の時間が指定した時間を過ぎたら
		if(sceKernelLibcClock() > time)
			return 1;
	}

	return 0;
}
