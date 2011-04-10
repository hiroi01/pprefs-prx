#ifndef __FILESELECTER_H_
#define __FILESELECTER_H_
#include "file.h"



void selectBasePath();
int fileSelecter(const char *startPath, dir_t *rtn, char* titleLabel,int selectType, char *dir_type_sort);

int fileManager(const char *startPath ,char* titleLabel,int selectType, char *dir_type_sort);

#endif
