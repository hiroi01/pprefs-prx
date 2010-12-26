#include <pspctrl.h>

#include "button.h"

void get_button(SceCtrlData *data)
{
  sceCtrlPeekBufferPositive( data, 1 );
  data->Buttons &= CHEACK_KEY;
}

void wait_button_up(SceCtrlData *data)
{
  while((data->Buttons & CHEACK_KEY) != 0)
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
