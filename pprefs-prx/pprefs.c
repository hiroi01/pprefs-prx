
#include "common.h"
#include "libmenu.h"
#include "button.h"

double gettimeofday_sec()
{
	struct timeval tv;
	//gettimeofday(&tv, NULL);
	sceKernelLibcGettimeofday(&tv, NULL);
	return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void pprefsSleep(double sleepTime)
{
	double timesec= gettimeofday_sec();
	
	while( (gettimeofday_sec() - timesec) < sleepTime );
}
//にゅーってでる窓を作る
void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor)
{
	int nowx = sx,nowy = sy;
	while(1){
		nowx += 8;
		nowy += 8;
		if( nowx > ex ) nowx = ex;
		if( nowy > ey ) nowy = ey;
		libmFillRect(sx , sy , nowx , nowy , bgcolor );
		libmFrame(sx , sy , nowx ,nowy , fgcolor );
		if( nowx == ex && nowy == ey ) break;
		pprefsSleep(0.008);
	}

}

//にゅーってしている最中にもボタンをgetする
void makeWindowWithButtonGetting(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor)
{
	int nowx = sx,nowy = sy;
	while(1){
		get_button(&padData);
		nowx += 8;
		nowy += 8;
		if( nowx > ex ) nowx = ex;
		if( nowy > ey ) nowy = ey;
		libmFillRect(sx , sy , nowx , nowy , bgcolor );
		libmFrame(sx , sy , nowx ,nowy , fgcolor );
		if( nowx == ex && nowy == ey ) break;
		pprefsSleep(0.008);
	}

}


