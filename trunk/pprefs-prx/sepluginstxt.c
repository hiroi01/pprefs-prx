#include "common.h"
#include "sepluginstxt.h"
#include "file.h"


static const char *textFileName[] = {
	"vsh.txt",
	"game.txt",
	"pops.txt"
};

/*	remove an item from pdata

	@param : type
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	@param : num
	number of removing data

	@return : 
	= 0 no problem
	< 0 on error
*/


char *getSepluginsTextName(char *str,char *basePath, int type)
{
	if( !(0 <= type &&  type <= 2) ) return NULL;
	strcpy(str, basePath);
	strcat(str, textFileName[type]);
	
	return str;
}

int removeAnItem(int type, int num){
	if( !(0 <= type &&  type <= 2) ) return -1;
	if(  ! (pdata[type].num > 0) ) return 1;
	
	if( num == pdata[type].num -1  ){//last item
		if( pdata[type].num > 0) pdata[type].num--;
	}else{
		for( ; num + 1 < pdata[type].num; num++ ){
			strcpy(pdata[type].line[num].path,pdata[type].line[num+1].path);
			pdata[type].line[num].toggle = pdata[type].line[num+1].toggle;
			pdata[type].line[num].pathLen = pdata[type].line[num+1].pathLen;
		}
		if( pdata[type].num > 0) pdata[type].num--;
	}
	
	return 0;
}


/*
int removeAnItem(int type,int num){
	if( !(0 <= type &&  type <= 2) ) return -1;
	if(  ! (pdata[type].num > 0) ) return 1;
	int i;
	for(i = num; i + 1 < pdata[type].num; i++){
		strcpy(pdata[type].line[num].path,pdata[type].line[num+1].path);
		pdata[type].line[num].toggle = pdata[type].line[num+1].toggle;
		pdata[type].line[num].pathLen = pdata[type].line[num+1].pathLen;
	}
	
	pdata[type].num--;
	
	return 0;
}
*/
/*
	add a new item to pdata

	@param : type
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	@param : str
	adding text(plugin's path)

	@return : 
	= 0 no problem
	< 0 on error
*/
int addNewItem(int type,struct pdataLine *lineData)
{
	if( !(0 <= type &&  type <= 2) ) return -1;
	if( !(pdata[type].num < MAX_LINE) ) return -2;

	strcpy(pdata[type].line[pdata[type].num].path,lineData->path);
	pdata[type].line[pdata[type].num].toggle = lineData->toggle;
	pdata[type].line[pdata[type].num].pathLen = strlen(pdata[type].line[pdata[type].num].path);
	pdata[type].line[pdata[type].num].print = &pdata[type].line[pdata[type].num].path[(pdata[type].line[pdata[type].num].pathLen < PRINT_PATH_LEN)?0:(pdata[type].line[pdata[type].num].pathLen-PRINT_PATH_LEN)];

	pdata[type].num++;
	
	return 0;
	

}


/*
	@param : ptype 
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	
	@return : 
	= 0 no problem
	= 1,2,3 error about vsh.txt(=1) or game.txt(=2) or pops.txt(=3)
	< 0 on error

*/

int writeSepluginsText(int ptype, char *basePath)
{
	if( !(0 <= ptype &&  ptype <= 2) ){
		return -1;
	}


	int i,type = ptype;
	SceUID fp;
	char path[128];
	
//	checkMs();

	strcpy(path, basePath);
	strcat(path, textFileName[type]);


	fp = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if( fp < 0 ) return (type+1);

	for( i = 0; i < pdata[type].num; i++ ){
		sprintf(commonBuf,"%s %c%s",
				pdata[type].line[i].path,
				pdata[type].line[i].toggle?'1':'0',
				lineFeedCode[config.lineFeedCode]
		);
		sceIoWrite(fp,commonBuf,strlen(commonBuf));
	}
	sceIoClose(fp);
	
	//各種情報更新
	sceIoGetstat(path, &pdata[type].stat);
	pdata[type].edit = false;
	pdata[type].exist = true;

	return 0;
}




/*
	@param : ptype 
	= 0 vsh.txt
	= 1 game.txt
	= 2 pops.txt
	= 3 all
	@param : checkFlag
	= true ファイルを読む必要があるかチェックして、必要なら読み込む
	= false 必要かどうか関係なく、ファイルを読み込む
	@return : 
	 = 0 no problem
	!= 0 on error
*/

int readSepluginsText( int ptype ,bool checkFlag, char *basePath)
{

	SceUID fp;
	int type,i,readSize,loopend,ret = 0;
	char line[LEN_PER_LINE],*ptr;
	SceIoStat stat;
	char path[128];


	if( 0 <= ptype &&  ptype <= 2 ){
		type = ptype;
		loopend = type + 1;
	}else if( ptype == 3 ){
		type = 0;
		loopend = 3;
	}else{
		type = 0;
		loopend = 1;
	}
	
//	checkMs();
	
	for( ; type < loopend; type++){
		pdata[type].edit = false;

		
		if( ptype > 3 ){
			strcpy(path, basePath);
		}else{
			strcpy(path, basePath);
			strcat(path, textFileName[type]);
		}

		if( sceIoGetstat(path, &stat) < 0 ){//ファイルが存在しない
			pdata[type].exist = false;
			pdata[type].num = 0;
			continue;
		}
		
		//checkFlagが立っていて、これ以前にファイルが存在していて、sizeとmtimeが一緒なら -> ファイルを読む必要なし
		if( checkFlag &&
		    pdata[type].exist == true &&
		    stat.st_size == pdata[type].stat.st_size &&
		    compareScePspDateTime(stat.st_mtime, pdata[type].stat.st_mtime)
		){
			continue;
		}

		
		fp = sceIoOpen(path, PSP_O_RDONLY, 0777);
		if( fp < 0 ){//ファイルオープンエラー
			pdata[type].exist = false;
			pdata[type].num = 0;
			ret |= (1<<type);
			continue;
		}


		i = 0;
		while( i < MAX_LINE ){
			readSize = read_line_file(fp,line,LEN_PER_LINE - 1);
			if( readSize < 0 ){
				break;
			}
			if( line[0] == '\0' || line[0] == ' ' ){
				continue;
			}else if( (ptr = strchr(line,' ')) == NULL){// ' 'が見つからないなら /  if ' ' is not found
				line[readSize] = '\0';
				strcpy(pdata[type].line[i].path,line);
				pdata[type].line[i].toggle = false;
			}else{
				*ptr = '\0';
				strcpy(pdata[type].line[i].path,line);
				if( (ptr - line + 1) < LEN_PER_LINE ){
					if( *(ptr+1) == '1' )
						pdata[type].line[i].toggle = true;
					else
						pdata[type].line[i].toggle = false;
				}
			}
			pdata[type].line[i].pathLen = strlen(pdata[type].line[i].path);
			pdata[type].line[i].print = &pdata[type].line[i].path[(pdata[type].line[i].pathLen < PRINT_PATH_LEN)?0:(pdata[type].line[i].pathLen-PRINT_PATH_LEN)];
			i++;
		}
		sceIoClose(fp);
		
		//各種情報を更新
		pdata[type].num = i;
		pdata[type].exist = true;
		pdata[type].stat = stat;
	}
	return ret;
}

