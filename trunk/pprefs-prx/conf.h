//from mp3play


// L[RtBO


#ifndef __CONF_H__
#define	__CONF_H__

#include <stdbool.h>


typedef struct
{
	u32 bootKey;
	bool swapButton;
	bool bootMessage;
	bool onePushRestart;
	int lineFeedCode;// = 0:CR+LF  =1:CR  =2:LF
} Conf_Key;


int Read_Line(SceUID fd, char *buf, int n);
int Read_Conf(const char *path, Conf_Key *key);
int Write_Conf(const char *path, Conf_Key *key);
void Set_Default_Conf(Conf_Key *key);

#endif

