
#ifndef __PPREFS_H_
#define __PPREFS_H_



double gettimeofday_sec();
void pprefsSleep(double sleepTime);
void makeWindow(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);
void makeWindowWithButtonGetting(int sx, int sy, int ex, int ey, u32 fgcolor ,u32 bgcolor);

#endif