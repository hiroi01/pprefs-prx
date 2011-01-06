
#include "button.h"

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