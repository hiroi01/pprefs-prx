// キーコンフィグ

#ifndef __CONF_H__
#define	__CONF_H__

#include <stdbool.h>


typedef struct
{
	u32 bootKey;
	bool swapButton;
	bool bootMessage;
} Conf_Key;


int Read_Line(SceUID fd, char *buf, int n);
int Read_Conf(const char *path, Conf_Key *key);


#endif

