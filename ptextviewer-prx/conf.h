//from mp3play


// L[RtBO


#ifndef __CONF_H__
#define	__CONF_H__

#include <stdbool.h>


typedef struct
{
	u32 bootKey;
	char startPath[128];
} Conf_Key;


int Read_Line(SceUID fd, char *buf, int n);
int Read_Conf(const char *path, Conf_Key *key);
int Write_Conf(const char *path, Conf_Key *key);
void Set_Default_Conf(Conf_Key *key);
void Set_Default_Path(char path[3][64], int num);

#endif

