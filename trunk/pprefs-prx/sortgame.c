
#include "sortgame.h"
#include "pprefsmenu.h"
#include "configmenu.h"
#include "common.h"

#define MAX_PATH_LEN (256)
#define MAX_DISPLAY_NUM 21


/** sceIoChstat bitmasks */
enum IOFileStatBitmask {
    FIO_CST_MODE = 0x0001,
    FIO_CST_ATTR = 0x0002,
    FIO_CST_SIZE = 0x0004,
    FIO_CST_CT = 0x0008,
    FIO_CST_AT = 0x0010,
    FIO_CST_MT = 0x0020,
    FIO_CST_PRVT = 0x0040
}; 


#define SORTGAME_FLAG_NUM 6//not use?
enum {
  SORTGAME_FLAG_ISO = 1,
  SORTGAME_FLAG_CSO = 2,
  SORTGAME_FLAG_EBOOTDIR = 4,
  SORTGAME_FLAG_CATDIR = 8 ,//gamecategories
  SORTGAME_FLAG_CATDIR_2 = 16 ,//gamecategories lite
  SORTGAME_FLAG_OTHERDIR = 32,
};


typedef struct {
  ScePspDateTime time;//並び順の判定基準になる時間
  char name[MAX_PATH_LEN];
  int sort_type;
  SceIoStat stat;
} sortgame_dir_t;



#define DIR_BUF_NUM 150




bool sortgame_confirm_save(bool editFlag)
{
	wait_button_up(&padData);

	if( editFlag ){
		char *menu[] = { PPREFSMSG_YESORNO_LIST };
		if( pprefsMakeSelectBox(24,  40, PPREFSMSG_SORTGAME_SAVE, menu, buttonData[buttonNum[0]].flag, 1 ) == 0 ){
			return true;
		}
	}
	
	return false;
}


/*
pathで指定されたフォルダにEBOOT.PBPが存在するか?
@return : 
          == 0 存在する
          != 0 存在しない
*/
int exist_ebootpbp(char *path)
{
	char str[256];
	int i;
	SceIoStat stat;

	/*
	strcpy(str, dirName);
	i = strlen(str);
	if( str[i-1] != '/' ){
		str[i] = '/';
		str[i+1] = '\0';
	}
	*/
	
	strcpy(str, path);
	i = strlen(str);
	if( str[i-1] != '/' ){
		str[i] = '/';
		str[i+1] = '\0';
	}

	strcat(str, "EBOOT.PBP");
	

	
	return sceIoGetstat(str, &stat);
}

char *getFullpath(char *rtn, const char *dirname,const char *basename)
{
	int len;
	
	strcpy(rtn, dirname);
	len = strlen(rtn);
	if( rtn[len-1] != '/' ){
		rtn[len] = '/';
		rtn[len+1] = '\0';
	}
	
	strcat(rtn, basename);

	return rtn;
}

/*
@param : path
         走査するpath
@param : dir
@param : file_num
@param : maxDirNum
@param : type

@return : 
          now file_num
*/
int sortgame_read_dir(char *path, sortgame_dir_t *dir, int file_num, int maxDirNum, u32 type)
{
	SceUID dp;
	SceIoDirent entry;
	int num;

	dp = sceIoDopen(path);
	if(dp < 0) return file_num;


	//これをやらないとフリーズするらしい
	memset(&entry, 0, sizeof(entry));

	while((sceIoDread(dp, &entry) > 0) && (file_num < maxDirNum))
	{
		getFullpath(dir[file_num].name, path, entry.d_name);
		sceIoGetstat(dir[file_num].name, &dir[file_num].stat);

		num = strlen(entry.d_name);

		switch(entry.d_stat.st_mode & FIO_S_IFMT)
		{
			case FIO_S_IFREG://ファイルなら
				if( type & SORTGAME_FLAG_ISO )
				{
					if( strncasecmp(&entry.d_name[num - 4], ".iso", 4) == 0 ){
						dir[file_num].time = dir[file_num].stat.st_ctime;
						dir[file_num].sort_type = 1;
						file_num++;
						continue;
					}
				}
				if( type & SORTGAME_FLAG_CSO )
				{
					if( strncasecmp(&entry.d_name[num - 4], ".cso", 4) == 0 ){
						dir[file_num].time = dir[file_num].stat.st_ctime;
						dir[file_num].sort_type = 1;
						file_num++;
						continue;
					}
				}
			break;

			case FIO_S_IFDIR://ディレクトリなら
				if( (strcmp(&entry.d_name[0], ".") != 0) && (strcmp(&entry.d_name[0], "..") != 0) )
				{
					if( type & SORTGAME_FLAG_EBOOTDIR )
					{
						if( type & SORTGAME_FLAG_CATDIR && strncasecmp(entry.d_name, "CAT_", 4) == 0 ) continue;
						
						if( exist_ebootpbp(dir[file_num].name) == 0 ){
							dir[file_num].time = dir[file_num].stat.st_mtime;
							dir[file_num].sort_type = 0;
							file_num++;
							continue;
						}
					}

					if( type & SORTGAME_FLAG_CATDIR )
					{
						if( strncasecmp(entry.d_name, "CAT_", 4) == 0 ){
							dir[file_num].time = dir[file_num].stat.st_mtime;
							dir[file_num].sort_type = 0;
							file_num++;
							continue;
						}
					}
					
					if( type & SORTGAME_FLAG_CATDIR_2 )
					{
						if( exist_ebootpbp(dir[file_num].name) != 0 ){
							dir[file_num].time = dir[file_num].stat.st_mtime;
							dir[file_num].sort_type = 0;
							file_num++;
							continue;
						}
					}
				}
			break;
		}
	}

	sceIoDclose(dp);


	return file_num;
}

void sortgame_run_sort( sortgame_dir_t *dir, int dirNum ,int mode)
{
	int i = (mode == 0)?1:0;
	ScePspDateTime time = dir[i].time;

	for( i++; i < dirNum; i++ )
	{

		//1分前へ
//		if( time.second > 9 ){
//			time.second -= 10;
//		}else{
//			time.second = time.second - 10 + 60;
			if( time.minute > 0 ){
				time.minute -= 1;
			}else{
				time.minute = 59;
				if( time.hour > 0 ){
					time.hour -= 1;
				}else{
					time.hour = 23;
					if( time.day > 1 ){
						time.day -= 1;
					}else{
						time.day = 28;
						if( time.month > 1 ){
							time.month -= 1;
						}else{
							time.month = 12;
							time.year -= 1;
						}
					}
				}
			}
//		}



		if( dir[i].sort_type == 0 ) dir[i].stat.st_mtime = time;
		else dir[i].stat.st_ctime = time;
		
		sceIoChstat(dir[i].name, &dir[i].stat, (FIO_CST_CT | FIO_CST_MT));
		
	}

}



//降順と昇順を間違えてしまったので、引数をキャストするときに逆にしてごまかした(ぇ
//callback for qsort
int gamesort_compare(const void *arg1, const void *arg2)
{
	sortgame_dir_t *dir1 = (sortgame_dir_t *)arg2;
	sortgame_dir_t *dir2 = (sortgame_dir_t *)arg1;

	if( dir1->time.year        != dir2->time.year )        return dir1->time.year        - dir2->time.year;
	if( dir1->time.month       != dir2->time.month )       return dir1->time.month       - dir2->time.month;
	if( dir1->time.day         != dir2->time.day )         return dir1->time.day         - dir2->time.day;
	if( dir1->time.hour        != dir2->time.hour )        return dir1->time.hour        - dir2->time.hour;
	if( dir1->time.minute      != dir2->time.minute )      return dir1->time.minute      - dir2->time.minute;
	if( dir1->time.second      != dir2->time.second )      return dir1->time.second      - dir2->time.second;
	if( dir1->time.microsecond != dir2->time.microsecond ) return dir1->time.microsecond - dir2->time.microsecond;


	return 0;
}

int sortgame_listup_category(sortgame_dir_t *dirBuf,int dirBufNum, char *rootPath, u32 type)
{
	int file_num = 0,i;
	char path[256];

	if( type & SORT_TYPE_GAME)
		file_num = sortgame_read_dir(getFullpath(path,rootPath,"PSP/GAME/"), dirBuf, file_num, dirBufNum, (type & SORT_TYPE_CATEGORIZES_LIGHT)?SORTGAME_FLAG_CATDIR_2:SORTGAME_FLAG_CATDIR);
	if( type & SORT_TYPE_ISOCSO)
		file_num = sortgame_read_dir(getFullpath(path,rootPath,"ISO/"), dirBuf, file_num, dirBufNum, (type & SORT_TYPE_CATEGORIZES_LIGHT)?SORTGAME_FLAG_CATDIR_2:SORTGAME_FLAG_CATDIR);
	if( type & SORT_TYPE_GAME150)
		file_num = sortgame_read_dir(getFullpath(path,rootPath,"PSP/GAME150/"), dirBuf, file_num, dirBufNum, (type & SORT_TYPE_CATEGORIZES_LIGHT)?SORTGAME_FLAG_CATDIR_2:SORTGAME_FLAG_CATDIR);
	if( type & SORT_TYPE_GAME500)
		file_num = sortgame_read_dir(getFullpath(path,rootPath,"PSP/GAME500/"), dirBuf, file_num, dirBufNum, (type & SORT_TYPE_CATEGORIZES_LIGHT)?SORTGAME_FLAG_CATDIR_2:SORTGAME_FLAG_CATDIR);

	qsort(dirBuf, file_num, sizeof(sortgame_dir_t), gamesort_compare);
	

	if( file_num + 1 > dirBufNum ) file_num = dirBufNum - 1;
	for( i = file_num; i > 0; i-- ) dirBuf[i] = dirBuf[i-1];
	file_num++;
	
		
	strcpy(dirBuf[0].name,"[Uncategorized]");
	
	return file_num;

}

int sortgame_listup(sortgame_dir_t *dirBuf,int dirBufNum, char *rootPath, char *exPath, u32 type)
{

	int file_num = 0,i,len;
	char path[256];
	u32 flag = 0;
	
	if( type & SORT_TYPE_CATEGORIZES ) flag |= SORTGAME_FLAG_CATDIR;
	
	if( exPath[0] == '\0' ){//Uncategorized
		if( type & SORT_TYPE_GAME )
			file_num = sortgame_read_dir(getFullpath(path,rootPath,"PSP/GAME/"), dirBuf, file_num, dirBufNum, SORTGAME_FLAG_EBOOTDIR | flag );
		if( type & SORT_TYPE_ISOCSO )
			file_num = sortgame_read_dir(getFullpath(path,rootPath,"ISO/"), dirBuf, file_num, dirBufNum, (SORTGAME_FLAG_ISO|SORTGAME_FLAG_CSO) );
		if( type & SORT_TYPE_GAME150 )
			file_num = sortgame_read_dir(getFullpath(path,rootPath,"PSP/GAME150/"), dirBuf, file_num, dirBufNum, SORTGAME_FLAG_EBOOTDIR | flag );
		if( type & SORT_TYPE_GAME500 )
			file_num = sortgame_read_dir(getFullpath(path,rootPath,"PSP/GAME500/"), dirBuf, file_num, dirBufNum, SORTGAME_FLAG_EBOOTDIR | flag);
	}else{
		strcpy(path,exPath);
		len = strlen(path);

		for( i = 0; i + 1 < len ; i++ ){
			if( path[i] == ':' &&  path[i+1] == '/' ) break;
		}
		i += 2;
		
		if( 
		    (path[i] == 'i' || path[i] == 'I') &&
		    (path[i+1] == 's' || path[i+1] == 'S') &&
		    (path[i+2] == 'o' || path[i+2] == 'O')
		){
			file_num = sortgame_read_dir(exPath, dirBuf, file_num, dirBufNum, (SORTGAME_FLAG_ISO|SORTGAME_FLAG_CSO) );
		}else{
			file_num = sortgame_read_dir(exPath, dirBuf, file_num, dirBufNum, SORTGAME_FLAG_EBOOTDIR );
		}
	}
	
	
	qsort(dirBuf, file_num, sizeof(sortgame_dir_t), gamesort_compare);

	return file_num;

}


/*
 矢印(カーソル)を↑↓長押しでもうまくスクロールするようにというマクロ
 なんとなく関数でなくマクロで実装してしまった
 */
#define ALLORW_WAIT(button,firstWait,wait) \
if( (beforeButtons & (button) ) == (button) ){ \
	if( firstFlag ){ \
		if( (sceKernelLibcClock() - time) >= (firstWait) ){ \
			time = sceKernelLibcClock(); \
			firstFlag = false; \
		}else{ \
			continue; \
		} \
	}else{ \
		if( (sceKernelLibcClock() - time) >= (wait) ){ \
			time = sceKernelLibcClock(); \
		}else{ \
			continue; \
		} \
	} \
}else{ \
	firstFlag = true; \
	beforeButtons = button; \
	time = sceKernelLibcClock(); \
} \






int sortgame_menu(void)
{
	wait_button_up(&padData);
	
	int file_num,i,now_arrow,offset,tmp,flag,tmp2;
	sortgame_dir_t *dir,dirTmp;
	u32 beforeButtons;
	clock_t time;
	bool firstFlag,editFlag;
	char rootName[16];
	char dirPath[128];
	int mode = 0;// == 0 カテゴリリストを表示

	strcpy(rootName,rootPath);
	
	dir = malloc( sizeof(sortgame_dir_t) * DIR_BUF_NUM );
	if( dir == NULL ) return -1;


	PRINT_SCREEN();
	
	if( config.sortType == 0 ){
		char *menu[] = { "OK", NULL };
		pprefsMakeSelectBox(24,  40, PPREFSMSG_SORTGAME_PLEASECONFIG ,menu, buttonData[buttonNum[0]].flag, 0);
		config.sortType = selectSorttypeByuser();
		PRINT_SCREEN();
	}
	

	//init
	dirPath[0] = '\0';
	
	if( config.sortType & SORT_TYPE_NORMAL_LIST ) mode = 1;

LIST_UP:
	
	if( mode == 0 ){
		file_num = sortgame_listup_category(dir, DIR_BUF_NUM, rootName, config.sortType );
	}else{
		file_num = sortgame_listup(dir, DIR_BUF_NUM, rootName, dirPath, config.sortType );
	}



	//init
	now_arrow = 0;
	offset = 0;
	beforeButtons = 0;
	time = 0;
	firstFlag = true;
	editFlag = false;

	while(1){
		libmFillRect( 0 , 38 , 480 , 46 + MAX_DISPLAY_NUM*(LIBM_CHAR_HEIGHT+2),BG_COLOR );
		for( i = 0; i < file_num && i < MAX_DISPLAY_NUM; i++ ){
			libmPrintf(15,38 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dir[i+offset].name);
		}
		libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");

		libmPrint(20,22,BG_COLOR,FG_COLOR,PPREFSMSG_SORTGAME_TITLE);
		libmFillRect(0 , 254 , 480 , 272 ,BG_COLOR);
		libmPrintf(0,264,EX_COLOR ,BG_COLOR,PPREFSMSG_SORTGAME_HOWTOUSE,buttonData[buttonNum[0]].name);

		while(1){
			get_button(&padData);
			if( padData.Buttons & (PSP_CTRL_DOWN|PSP_CTRL_UP) )
			{
				ALLORW_WAIT((PSP_CTRL_DOWN|PSP_CTRL_UP),3 * 100 * 1000,1 * 100 * 1000);
				
				flag = 0;
				tmp = now_arrow;
				tmp2 = now_arrow + offset;
				if( padData.Buttons & PSP_CTRL_DOWN ){
					if( now_arrow + 1 < MAX_DISPLAY_NUM && now_arrow + 1 < file_num ){
						now_arrow++;
						libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
					}else{
						if( offset+MAX_DISPLAY_NUM < file_num ){
							offset++;
							flag = 1;
						}
					}
				}else{
					if( now_arrow - 1 >= 0 ){
						now_arrow--;
					}else{
						if( offset > 0 ){
							offset--;
							flag = 1;
						}
					}
				}

				if( padData.Buttons & PSP_CTRL_SQUARE && tmp2 != now_arrow + offset ){
					if( !(mode == 0 && ((now_arrow + offset) == 0 || tmp2 == 0)) ){
						editFlag = true;
						
						dirTmp					= dir[tmp2];
						dir[tmp2]				= dir[now_arrow + offset];
						dir[now_arrow + offset]	= dirTmp;

						fillLine(38 +       tmp*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
						fillLine(38 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
						libmPrintf(15,38 +       tmp*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dir[      tmp+offset].name);
						libmPrintf(15,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dir[now_arrow+offset].name);
					}
				}
				
				if( flag ) break;
				
				libmPrintf(5,38 +       tmp*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");

			}
			else if( (!(config.sortType & SORT_TYPE_NORMAL_LIST)) && mode == 0 && padData.Buttons & (buttonData[buttonNum[0]].flag|PSP_CTRL_RTRIGGER) )
			{

				if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir, file_num, mode);

				//if select [Uncategorize]
				if( now_arrow+offset == 0 ) dirPath[0] = '\0';
				else strcpy(dirPath, dir[now_arrow+offset].name);
				mode = 1;
				
				wait_button_up(&padData);
				goto LIST_UP;
			}
			else if( (!(config.sortType & SORT_TYPE_NORMAL_LIST)) && mode == 1 && padData.Buttons & PSP_CTRL_LTRIGGER )
			{
				if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir, file_num, mode);

				mode = 0;
				wait_button_up(&padData);
				goto LIST_UP;
			}
			else if( padData.Buttons & PSP_CTRL_START )
			{
				if( beforeButtons & PSP_CTRL_START ) continue;
				beforeButtons = PSP_CTRL_START;
				
				if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir,file_num, mode);
				goto LIST_UP;
			}
			else if( padData.Buttons & PSP_CTRL_SELECT )
			{
				
				if( beforeButtons & PSP_CTRL_SELECT ) continue;
				beforeButtons = PSP_CTRL_SELECT;
				
				char *menu[] = { PPREFSMSG_YESORNO_LIST };
				if( pprefsMakeSelectBox(24,  40, PPREFSMSG_MAINMENU_RELOAD,menu, buttonData[buttonNum[0]].flag, 1 ) == 0 ){
					wait_button_up(&padData);
					goto LIST_UP;
				}
				
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				if( beforeButtons & PSP_CTRL_HOME ) continue;
				beforeButtons = PSP_CTRL_HOME;
				
				/*
				tmp = 0;
				if( editFlag ){
					char *menu[] = { PPREFSMSG_YESORNO_LIST };
					if( pprefsMakeSelectBox(24,  40, PPREFSMSG_SORTGAME_SAVE, menu, buttonData[buttonNum[0]].flag, 1 ) == 0 ){
						tmp = 1;
					}
				}
				if( tmp ) sortgame_run_sort(dir, file_num, mode);
				*/
				
				if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir,file_num, mode);

				free(dir);
				wait_button_up(&padData);
				return 0;
			}
			else
			{
				beforeButtons = 0;
			}
		}
	}
	
	
	return 0;
}