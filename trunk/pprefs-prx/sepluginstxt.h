
#ifndef __SEPLUGINSTXT_H_
#define __SEPLUGINSTXT_H_

#include <pspdebug.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <psprtc.h>
#include <psputility.h>
#include <psputility_sysparam.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspctrl_kernel.h>


#define LEN_PER_LINE 128
#define MAX_LINE 32
#define PRINT_PATH_LEN 52

struct pdatas{
	int num;//number of line
	bool exist;//ファイルが存在するかフラグ
	bool edit;//編集フラグ
	SceIoStat stat;
	struct pdataLine{
		char path[LEN_PER_LINE];
		bool toggle;//= ture ON / = false OFF
		int pathLen;
		char *print;
	}line[MAX_LINE];
} pdata[3];

int removeAnItem(int type,int num);
int addNewItem(int type,struct pdataLine *lineData);

char *getSepluginsTextName(char *str,char *basePath, int type);
int readSepluginsText( int ptype ,bool checkFlag ,char *basePath);
int writeSepluginsText(int ptype, char *basePath);

#endif