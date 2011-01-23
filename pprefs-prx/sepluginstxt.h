
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


#define LEN_PER_LINE 256
#define MAX_LINE 64

struct pdatas{
	int num;//number of line
	bool exist;//ファイルが存在するかフラグ
	bool edit;//編集フラグ
	SceIoStat stat;
	struct pdataLine{
		char path[LEN_PER_LINE];
		bool toggle;//= ture ON / = false OFF
	}line[MAX_LINE];
} pdata[3];

int removeAnItem(int type,int num);
int addNewItem(int type,struct pdataLine *lineData);


int readSepluginsText( int ptype ,bool checkFlag );
int writeSepluginsText(int ptype);

#endif