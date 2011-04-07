
#include "sortgame.h"
#include "pprefsmenu.h"
#include "configmenu.h"
#include "dxlibp/dxppng.h"
#include "common.h"

#define MAX_PATH_LEN (256)
#define MAX_DISPLAY_NUM 7



/*-----------------------------------------------------------------------*/


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


#define SORTGAME_FLAG_NUM 6//not used?
enum {
  SORTGAME_FLAG_ISO = 1,
  SORTGAME_FLAG_CSO = 2,
  SORTGAME_FLAG_EBOOTDIR = 4,
  SORTGAME_FLAG_CATDIR = 8 ,//gamecategories
  SORTGAME_FLAG_CATDIR_2 = 16 ,//gamecategories lite
  SORTGAME_FLAG_OTHERDIR = 32,
};

//48x27
#define SORTGAME_IMG_BUF_SIZE (1296)
typedef struct sortgame_img_{
	u32 raw[SORTGAME_IMG_BUF_SIZE];
	int width,height;
} sortgame_img;

typedef struct {
  ScePspDateTime time;//並び順の判定基準になる時間
  char name[MAX_PATH_LEN];
  int sort_type;
  SceIoStat stat;
  sortgame_img img;
} sortgame_dir_t;


#define DIR_BUF_NUM 150



typedef struct EBOOT_PBP_HEADER_{
	char signature[4];// 0x00 0x50 0x42 0x50
	u32 version;//0x00 0x00 0x01 0x00
	u32 offsetOfParamSfo;
	u32 offsetOfIcon0Png;
	u32 offsetOfIcon1Pmf;
	u32 offsetOfIcon1Png;
	u32 offsetPic1Png;
	u32 offsetOfSnd0At3;
	u32 offsetOfDataPsp;
	u32 offsetOfDataPsar;
}EBOOT_PBP_HEADER;

/*-----------------------------------------------------------------------*/

//static int loadIconThreadState = 0;// == 0 not created / == 1 working / == 2 waiting / == 3 stopping
//static int loadIconSemaId = 0;
static sortgame_dir_t *sortgameBuf = NULL;
static int sortgameBufNum = 0;

/*-----------------------------------------------------------------------*/



int loadIcon0(const char *dirname, DXPPNG *png)
{
	EBOOT_PBP_HEADER header;
	DXPPNG_PARAMS param;
	SceUID fd;
	char path[256];
	
	strcpy(path, dirname);
	//最後が'/'で終わるパスじゃないなら
	if( path[strlen(path) - 1] != '/' ){
		strcat(path ,"/EBOOT.PBP");
	}else{
		strcat(path ,"EBOOT.PBP");
	}
	
	if( (fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) < 0 ) return fd;
	sceIoLseek(fd, 0, SEEK_SET);
	sceIoRead(fd , &header, sizeof(header));
	
	param.srcLength = header.offsetOfIcon1Pmf - header.offsetOfIcon0Png;

	if( param.srcLength == 0 ){
		sceIoClose(fd);
		return 1;
	}
	
	param.src = malloc(param.srcLength);
	if( param.src == NULL ){
		sceIoClose(fd);
		return 2;
	}
	
	sceIoLseek(fd, header.offsetOfIcon0Png, SEEK_SET);
	sceIoRead(fd, param.src, param.srcLength);
	
	sceIoClose(fd);
	
	param.mode = DXPPNG_MODE_RAW;
	
	param.funcs.pfree = 0;
	int rtn = dxppng_decode(&param, png);
	
	free(param.src);
	
	return rtn;
	
}

//go only
void sortgame_selectStrage(char *path)
{
	
	int num = 0;
	char menuStr[6] = "**0:/";
	char *menu[] = { 
		menuStr,
		"ef0:/",
		NULL
	};
	
	/*
	6.20TNのパスのエイリアス機能対策(?)
	goで(vshのときだけ?)ef0:/からプラグインを読み込ませるとms0:/という文字列をef0:/と書き換えるっぽいので、
	更に上書きしてやる
	*/
	menuStr[0] = 'm';
	menuStr[1] = 's';
	
	num = pprefsMakeSelectBox(20, 50, "SELECT STORAGE",menu, buttonData[buttonNum[0]].flag, 1 );
	
	if( num >= 0 ){
		strcpy(path,menu[num]);
	}
}

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

void sortgame_run_sort( sortgame_dir_t *dir, int dirNum, char *rootPath ,int mode , bool isRemoveIsocache )
{
	int i = (mode == 0)?1:0;
	ScePspDateTime time = dir[i].time;

	if( isRemoveIsocache ){
		char removedFilePath[128];
		strcpy(removedFilePath, rootPath);
		strcat(removedFilePath, "PSP/SYSTEM/ISOCACHE.BIN");
		sceIoRemove(removedFilePath);
	}

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
//てきとう実装、もっと高速なソートアルゴリズムがあると思う
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
	
	for( i = 0; i < file_num; i ++){
		memset(dirBuf[i].img.raw, 0, sizeof(u32) * SORTGAME_IMG_BUF_SIZE);
		dirBuf[i].img.width = 0;
		dirBuf[i].img.height = 0;
	}
	
	strcpy(dirBuf[0].name,"[Uncategorized]");
	
	return file_num;

}

/*
void reducationRaw(u32 *srcData, int srcWidth, int srcHeight, u32 *dstData, int degree)
{
	int dstWidth = srcWidth / degree;
	int dstHeight = srcHeight / degree;
	u32 red, green, blue;
	int degreeX2 = degree * 2;
	
	int dstX,dstY,srcX,srcY,i,j;
	
	srcX = 0; srcY = 0; i = 0;
	for( dstY = 0; dstY < dstHeight; dstY++ ){
		for( dstX = 0; dstX < dstWidth; dstX++ ){

			blue = 0; green = 0; red = 0;
			for( i = 0; i < degree; i++ ){
				for( j = 0; j < degree; j++ ){
					blue  += (srcData[(srcX + j) + (srcY + i) * srcWidth] >> 16) & 0xff;
					green += (srcData[(srcX + j) + (srcY + i) * srcWidth] >> 8) & 0xff;
					red   += (srcData[(srcX + j) + (srcY + i) * srcWidth]) & 0xff;
				}
			}
			srcX += degree;
			if( srcX >= srcWidth ){
				srcX = 0;
				srcY += degree;
			}
			
			dstData[dstX + dstY * dstWidth] = 0xff000000 | ((blue/degreeX2)<<16) | ((green/degreeX2)<<8) | (red/degreeX2);
		}
	}
	
}
*/


/// http://thorshammer.blog95.fc2.com/blog-entry-169.html
/// <summary>
/// 単純に補間間引きによる拡大縮小
/// </summary>
void reducationRaw2(u32 *data, int xSize, int ySize, u32 *rescaledata, int hxSize, int hySize)
{
    // 拡大縮小用
    double xpos, ypos;
    double hokanX = (double)xSize / hxSize;
    double hokanY = (double)ySize / hySize;
	int i,j;
	int offset;
    ypos = 0.0;
    for (i = 0; i < hySize; i++)
    {
        xpos = 0.0;
        for (j = 0; j < hxSize; j++)
        {
            // 単純補間・間引き
            offset = j + i * hxSize;
            rescaledata[j + i * hxSize] = data[(int)xpos + ((int)ypos) * xSize];
            xpos += hokanX;
        }
        ypos += hokanY;
    }

    return;
}

int sortgame_listup(sortgame_dir_t *dirBuf,int dirBufNum, char *rootPath, char *exPath, u32 type)
{

	int file_num = 0,i = 0,len;
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
		){//if iso folder
			file_num = sortgame_read_dir(exPath, dirBuf, file_num, dirBufNum, (SORTGAME_FLAG_ISO|SORTGAME_FLAG_CSO) );
		}else{
			file_num = sortgame_read_dir(exPath, dirBuf, file_num, dirBufNum, SORTGAME_FLAG_EBOOTDIR );
		}
	}

	
	
	
	if( config.sortType & SORT_TYPE_NOTDISPLAY_ICON0 ){
		for( i = 0; i < file_num; i ++){
			memset(dirBuf[i].img.raw, 0, sizeof(u32) * SORTGAME_IMG_BUF_SIZE);
			dirBuf[i].img.width = 0;
			dirBuf[i].img.height = 0;
		}
	}else{
		DXPPNG pngtmp;
		char *gauge[] = {
			"-",
			"\\",
			"|",
			"/"
		};
		libmPrint(20,22,BG_COLOR,FG_COLOR,"Loading...");
		//clear & set img
		for( i = 0; i < file_num; i ++){
			memset(dirBuf[i].img.raw, 0, sizeof(u32) * SORTGAME_IMG_BUF_SIZE);
			dirBuf[i].img.width = 0;
			dirBuf[i].img.height = 0;
			
			if( loadIcon0(dirBuf[i].name, &pngtmp) == 0 ){//iconの読み込みに成功したら
				dirBuf[i].img.width = pngtmp.widthN2 / 3;
				dirBuf[i].img.height = pngtmp.heightN2 / 3;

				if( dirBuf[i].img.width > 48 ) dirBuf[i].img.width = 48;
				if( dirBuf[i].img.height > 27 ) dirBuf[i].img.height = 27;
				reducationRaw2((u32 *)pngtmp.raw, pngtmp.widthN2, pngtmp.heightN2, dirBuf[i].img.raw, dirBuf[i].img.width, dirBuf[i].img.height);

				free(pngtmp.raw);
			}
			libmPrint(100,22,BG_COLOR,FG_COLOR,gauge[i%4]);

		}
	}
	
	
	//sort
	qsort(dirBuf, file_num, sizeof(sortgame_dir_t), gamesort_compare);

	return file_num;

}


/*
 矢印(カーソル)を↑↓長押しでもうまくスクロールするようにというマクロ
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


/*
void sortgame_move_to_top()
{
	for( i = tmp; i > *now_arrow; i-- ){
		swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i-1]);
	}
}

void sortgame_move_to_bottom()
{
	for( i = tmp; i < *now_arrow; i++ ){
		swap_pdataLine(pdata[now_type].line[i],pdata[now_type].line[i+1]);
	}

}
*/




void sortgame_display_img(sortgame_img *img, u32 startX, u32 startY)
{
	u32 x,y,count = 0;
	for(y = 0; y < img->height; y++){
		for(x = 0; x < img->width; x++){
			libmPoint(libmMakeDrawAddr(startX + x, startY + y),img->raw[count++]);
		}
	}
}


/*
void stopLoadIconThreadAndWait()
{
	if( loadIconThreadState == 1 ){//loadIconThread is working
		loadIconThreadState = 3;//stop
	}
	while( loadIconThreadState != 2 );
}

void startLoadIconThread()
{
	if( loadIconThreadState == 2 ){//loadIconThread is waiting
		loadIconThreadState = 1;//work
		sceKernelSignalSema(loadIconSemaId, 1);
	}
}


int loadIconThread( SceSize arglen, void *argp )
{
	DXPPNG pngtmp;
	int i;
	
	
	while(1){
		loadIconThreadState = 2;//wait
		sceKernelWaitSema(loadIconSemaId,1,0);

		loadIconThreadState = 1;//work
		sceKernelSignalSema(loadIconSemaId, 0);
		
		for( i = 0; i < sortgameBufNum; i ++){
			if( loadIcon0(sortgameBuf[i].name, &pngtmp) == 0 ){//iconの読み込みに成功したら
				sortgameBuf[i].img.width = pngtmp.widthN2 / 3;
				sortgameBuf[i].img.height = pngtmp.heightN2 / 3;
				if( sortgameBuf[i].img.width > 48 ) sortgameBuf[i].img.width = 48;
				if( sortgameBuf[i].img.height > 27 ) sortgameBuf[i].img.height = 27;
				reducationRaw2((u32 *)pngtmp.raw, pngtmp.widthN2, pngtmp.heightN2, sortgameBuf[i].img.raw, sortgameBuf[i].img.width, sortgameBuf[i].img.height);

				free(pngtmp.raw);
			}
			if( loadIconThreadState == 3 ) break;
			sceKernelDelayThread(1000 * 1000);
		}
	}
	
	return 0;

}

*/

#define SPACE_BETWEEN_THE_LINES (31)


#define printOneGame(i, offset) \
libmFillRect(5, 35 + i*(SPACE_BETWEEN_THE_LINES) + 3, 475, 35 + i*(SPACE_BETWEEN_THE_LINES) + 3 + 27 ,BG_COLOR); \
sortgame_display_img(&dir[i+offset].img, 7, 35 + i*(SPACE_BETWEEN_THE_LINES) + 3 ); \
libmPrint(7 + 48 + 5, 35 + i*(SPACE_BETWEEN_THE_LINES) + 9, FG_COLOR, BG_COLOR,  dir[i+offset].name); \




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
	if( dir == NULL ) return -2;
	sortgameBuf = dir;
		
	/*
	if( loadIconThreadState == 0 ){
		loadIconSemaId = sceKernelCreateSema("loadicon_sema", 0, 0, 1, 0);
		SceUID thid = sceKernelCreateThread( "PPREFS_LOADICON", loadIconThread, 10, 0x1000, PSP_THREAD_ATTR_CLEAR_STACK, 0 );
		if( thid ){
			sceKernelStartThread( thid, 0, NULL );
		}else{
			return -1;
		}
	}
	*/
	

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
	
//	stopLoadIconThreadAndWait();
	
	if( mode == 0 ){
		file_num = sortgame_listup_category(dir, DIR_BUF_NUM, rootName, config.sortType );
	}else{
		file_num = sortgame_listup(dir, DIR_BUF_NUM, rootName, dirPath, config.sortType );
	}
	sortgameBufNum = file_num;

//	startLoadIconThread();

	//init
	now_arrow = 0;
	offset = 0;
	beforeButtons = 0;
	time = 0;
	firstFlag = true;
	editFlag = false;

	libmFillRect( 0 , 35 , 480 , 35 + MAX_DISPLAY_NUM*(SPACE_BETWEEN_THE_LINES),BG_COLOR );
	while(1){
		for( i = 0; i < file_num && i < MAX_DISPLAY_NUM; i++ ){
//			libmPrintf(15,38 + i*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dir[i+offset].name);
//			libmFillRect(6, 38 + i*(SPACE_BETWEEN_THE_LINES) + 3, 5 + 48, 38 + i*(SPACE_BETWEEN_THE_LINES) + 3 + 27 ,BG_COLOR);
			printOneGame(i, offset);
		}
//		libmPrintf(5 ,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
		libmFrame(5 , 35 + now_arrow*(SPACE_BETWEEN_THE_LINES), 475, 35 + (now_arrow+1)*(SPACE_BETWEEN_THE_LINES) - 1, FG_COLOR );

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
//						libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
						libmFrame(5 , 35 + now_arrow*(SPACE_BETWEEN_THE_LINES), 475, 35 + (now_arrow+1)*(SPACE_BETWEEN_THE_LINES) - 1, FG_COLOR );

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

//						fillLine(38 +       tmp*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
//						fillLine(38 + now_arrow*(LIBM_CHAR_HEIGHT+2),BG_COLOR);
//						libmPrintf(15,38 +       tmp*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dir[      tmp+offset].name);
//						libmPrintf(15,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,"%s",dir[now_arrow+offset].name);

						printOneGame(tmp, offset);
						printOneGame(now_arrow, offset);
//						libmFillRect(5, 38 + tmp*(SPACE_BETWEEN_THE_LINES) + 3, 475, 38 + tmp*(SPACE_BETWEEN_THE_LINES) + 3 + 27 ,BG_COLOR);
//						libmFillRect(5, 38 + now_arrow*(SPACE_BETWEEN_THE_LINES) + 3, 475, 38 + now_arrow*(SPACE_BETWEEN_THE_LINES) + 3 + 27 ,BG_COLOR);
//						sortgame_display_img(&dir[tmp+offset].img, 5, 38 + tmp*(SPACE_BETWEEN_THE_LINES) + 3 );
//						libmPrint(5 + 74 + 5, 38 + tmp*(SPACE_BETWEEN_THE_LINES) + 9, FG_COLOR, BG_COLOR, dir[tmp+offset].name);
//						sortgame_display_img(&dir[now_arrow+offset].img, 5, 38 + now_arrow*(SPACE_BETWEEN_THE_LINES) + 3 );
//						libmPrint(5 + 74 + 5, 38 + now_arrow*(SPACE_BETWEEN_THE_LINES) + 9, FG_COLOR, BG_COLOR, dir[now_arrow+offset].name);

					}
				}
				
				if( flag ) break;
				
//				libmPrintf(5,38 +       tmp*(LIBM_CHAR_HEIGHT+2),BG_COLOR,BG_COLOR," ");
//				libmPrintf(5,38 + now_arrow*(LIBM_CHAR_HEIGHT+2),FG_COLOR,BG_COLOR,">");
				libmFrame(5 , 35 + tmp*(SPACE_BETWEEN_THE_LINES), 475, 35 + (tmp+1)*(SPACE_BETWEEN_THE_LINES) - 1, BG_COLOR );
				libmFrame(5 , 35 + now_arrow*(SPACE_BETWEEN_THE_LINES), 475, 35 + (now_arrow+1)*(SPACE_BETWEEN_THE_LINES) - 1, FG_COLOR );

			}
			else if( (!(config.sortType & SORT_TYPE_NORMAL_LIST)) && mode == 0 && padData.Buttons & (buttonData[buttonNum[0]].flag|PSP_CTRL_RTRIGGER) )
			{

				if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir, file_num, rootName, mode, (config.sortType & SORT_TYPE_NOTREMOVE_ISOCACHE)?false:true);


				//if select [Uncategorize]
				if( now_arrow+offset == 0 ) dirPath[0] = '\0';
				else strcpy(dirPath, dir[now_arrow+offset].name);
				mode = 1;
				
				wait_button_up(&padData);
				goto LIST_UP;
			}
			else if( padData.Buttons & PSP_CTRL_LTRIGGER )
			{
				if( beforeButtons & PSP_CTRL_LTRIGGER ) continue;
				beforeButtons = PSP_CTRL_LTRIGGER;

				if( (!(config.sortType & SORT_TYPE_NORMAL_LIST)) && mode == 1 ){
					if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir, file_num, rootName, mode, (config.sortType & SORT_TYPE_NOTREMOVE_ISOCACHE)?false:true);
					mode = 0;
				}else if( deviceModel == 4 ){//if device is 'go'
					if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir, file_num, rootName, mode, (config.sortType & SORT_TYPE_NOTREMOVE_ISOCACHE)?false:true);
					sortgame_selectStrage(rootName);
				}else{
					continue;
				}

				wait_button_up(&padData);
				goto LIST_UP;
			}
			else if( padData.Buttons & PSP_CTRL_START )
			{
				if( beforeButtons & PSP_CTRL_START ) continue;
				beforeButtons = PSP_CTRL_START;
				
				if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir, file_num, rootName, mode, (config.sortType & SORT_TYPE_NOTREMOVE_ISOCACHE)?false:true);
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

				wait_button_up(&padData);
				break;
			}
			else if( padData.Buttons & PSP_CTRL_HOME )
			{
				if( beforeButtons & PSP_CTRL_HOME ) continue;
				beforeButtons = PSP_CTRL_HOME;
				
				if( sortgame_confirm_save(editFlag) ) sortgame_run_sort(dir, file_num, rootName, mode, (config.sortType & SORT_TYPE_NOTREMOVE_ISOCACHE)?false:true);
				
				free(dir);
				sortgameBuf = NULL;
				sortgameBufNum = 0;
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
