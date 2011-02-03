//base is mp3play

// ヘッダー
#include <pspkernel.h>
#include <pspctrl.h>
#include <string.h>
#include <stdio.h>
#include "conf.h"
#include "button.h"
#include "file.h"

// プロトタイプ宣言
int Check_EOF(SceUID fd);
u32 Conv_Key(const char *buf);
char *ch_token(char *str, const char *delim);



int removeSpace(char *str);



extern char *lineFeedCode[3];

//わたされた文字列のスペースを取り除く
int removeSpace(char *str)
{
	int i,j;
	
	for( i = 0; str[i] != '\0'; i++ ){
		if( str[i] == ' ' || str[i] == '\r' ){
			for( j = i; str[j] != '\0'; j++ ) str[j] = str[j+1]; //1文字詰める
		}
	}
	
	return i;
}


// 関数
int Read_Line(SceUID fd, char *buf, int n)
{
	int res, count = 0;
	char c;

	if(fd < 0)
		return 0;

	do
	{
		res = sceIoRead(fd, &c, 1);

		if(c == '\r' || c == ' ')	continue;
		else if(c == '\n')			break;

		else
		{
			buf[count++] = c;
		}
	}
	while (res > 0 && count < n);

	buf[count] = '\0';
	return count;
}

int Check_EOF(SceUID fd)
{
	char c;

	if(sceIoRead(fd, &c, 1) == 1)
	{
		sceIoLseek(fd, -1, PSP_SEEK_CUR);
		return 0;
	}

	return 1;
}

u32 Conv_Key(const char *buf)
{
	if(strcasecmp(buf, "SELECT") == 0)			return PSP_CTRL_SELECT;
	else if(strcasecmp(buf, "START") == 0)		return PSP_CTRL_START;
	else if(strcasecmp(buf, "UP") == 0)			return PSP_CTRL_UP;
	else if(strcasecmp(buf, "RIGHT") == 0)		return PSP_CTRL_RIGHT;
	else if(strcasecmp(buf, "DOWN") == 0)		return PSP_CTRL_DOWN;
	else if(strcasecmp(buf, "LEFT") == 0)		return PSP_CTRL_LEFT;
	else if(strcasecmp(buf, "L_TRI") == 0)		return PSP_CTRL_LTRIGGER;
	else if(strcasecmp(buf, "R_TRI") == 0)		return PSP_CTRL_RTRIGGER;
	else if(strcasecmp(buf, "LTrigger") == 0)		return PSP_CTRL_LTRIGGER;
	else if(strcasecmp(buf, "RTrigger") == 0)		return PSP_CTRL_RTRIGGER;
	else if(strcasecmp(buf, "TRIANGLE") == 0)	return PSP_CTRL_TRIANGLE;
	else if(strcasecmp(buf, "CIRCLE") == 0)		return PSP_CTRL_CIRCLE;
	else if(strcasecmp(buf, "CROSS") == 0)		return PSP_CTRL_CROSS;
	else if(strcasecmp(buf, "SQUARE") == 0)		return PSP_CTRL_SQUARE;
	else if(strcasecmp(buf, "HOME") == 0)		return PSP_CTRL_HOME;
//	else if(strcasecmp(buf, "HOLD") == 0)		return PSP_CTRL_HOLD;
	else if(strcasecmp(buf, "NOTE") == 0)		return PSP_CTRL_NOTE;
	else if(strcasecmp(buf, "SCREEN") == 0)		return PSP_CTRL_SCREEN;
	else if(strcasecmp(buf, "VOLUP") == 0)		return PSP_CTRL_VOLUP;
	else if(strcasecmp(buf, "VOLDOWN") == 0)	return PSP_CTRL_VOLDOWN;
	else return 0;
}

// strtok関数の代わり(NULL指定不可)
// delimで指定した文字列をstrから探して分断する
char *ch_token(char *str, const char *delim)
{
	char *ptr = strstr(str, delim);
	if(ptr == NULL) return NULL;

	*ptr = '\0';
	return ptr + strlen(delim);
}


void INI_Get_Key(const char *str, u32 *rtn ,u32 defaultValue )
{
	char buf[256];
	char *ptr;

	//初期化
	*rtn = 0;
	// コピー
	strcpy(buf, str);

	// 最後まで
	do
	{
		ptr = ch_token(buf, "+");
		*rtn |= Conv_Key(buf);
		
		if(ptr != NULL)
			strcpy(buf, ptr);
	}
	while (ptr != NULL);

	if( *rtn == 0 ){
		*rtn = defaultValue;
	}
	
	return;
}



void INI_Get_String(const char *str, char *rtn, const char *defaultValue)
{
	if( str[0] == '"' ){
		int i;
		for( i = 1; str[i] != '\0' && str[i] != '"' ; i++ ){
			rtn[i-1] = str[i];
		}
		if( str[i] == '"' ){
			rtn[i-1] = '\0';
			return;
		}
	}
	
	strcpy(rtn,defaultValue);
	return;
}

void INI_Get_Number_From_List(const char *str, int *rtn ,int defaultNum, char **list )
{
	int i;
	
	for( i = 0; list[i] != NULL; i++ ){
		if( strcasecmp(str,list[i]) == 0 ){
			*rtn = i;
			return;
		}
	}
	
	*rtn = defaultNum;
	return;
}

void INI_Get_Hex(const char *str, u32 *rtn ,u32 defaultNum)
{
	int i;

	*rtn = 0;
	for( i = 0; str[i] != '\0'; i++ ){
		*rtn *= 16;
		if( str[i] == '0' ){ *rtn += 0;
		}else if( str[i] == '1' ){ *rtn += 1;
		}else if( str[i] == '2' ){ *rtn += 2;
		}else if( str[i] == '3' ){ *rtn += 3;
		}else if( str[i] == '4' ){ *rtn += 4;
		}else if( str[i] == '5' ){ *rtn += 5;
		}else if( str[i] == '6' ){ *rtn += 6;
		}else if( str[i] == '7' ){ *rtn += 7;
		}else if( str[i] == '8' ){ *rtn += 8;
		}else if( str[i] == '9' ){ *rtn += 9;
		}else if( str[i] == 'A' || str[i] == 'a' ){ *rtn += 10;
		}else if( str[i] == 'B' || str[i] == 'b' ){ *rtn += 11;
		}else if( str[i] == 'C' || str[i] == 'c' ){ *rtn += 12;
		}else if( str[i] == 'D' || str[i] == 'd' ){ *rtn += 13;
		}else if( str[i] == 'E' || str[i] == 'e' ){ *rtn += 14;
		}else if( str[i] == 'F' || str[i] == 'f' ){ *rtn += 15;
		}else{
			*rtn = defaultNum;
			return;
		}
	}
	return;
}

void INI_Get_Bool(const char *str, bool *rtn, bool defaultValue)
{
	*rtn = defaultValue;
	
	if( strcasecmp(str,"true") == 0 ){
		*rtn = true;
	}else if(  strcasecmp(str,"false") == 0 ){
		*rtn = false;
	}
	
}

void INI_Set_Default(INI_Key *key)
{
	int i;
	for( i = 0; i < key[0].keyNum; i++ ){
		if( key[i].type & INI_TYPE_LIST )
		{
			*key[i].value.i = key[i].defaultValue.i;
		}
		else if( key[i].type & INI_TYPE_HEX )
		{
			*key[i].value.u = key[i].defaultValue.u;
		}
		else if( key[i].type & INI_TYPE_BUTTON )
		{
			*key[i].value.u = key[i].defaultValue.u;
		}
		else if( key[i].type & INI_TYPE_STRING )
		{
			strcpy(key[i].value.s,key[i].defaultValue.s);
		}
		else if( key[i].type & INI_TYPE_BOOL )
		{
			*key[i].value.b = key[i].defaultValue.b;
		}
	}
}

int INI_Read_Conf(const char *path, INI_Key *key)
{
	SceUID fd;
	char buf[256];
	char *ptr;
	int i;

	
	INI_Set_Default(key);

	
	// 設定ファイル・オープン
	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if(fd < 0)
	{
		return -1;
	}

	// ファイル終端までループ
	while (Check_EOF(fd) == 0)
	{
		// 一行読み込む
		Read_Line(fd, buf, 255);

		// コメントと改行ならスキップ
		if(buf[0] == '#' || buf[0] == '\n' || buf[0] == '\0') continue;
		
		//=でsplit
		ptr = ch_token(buf, "=");
		if(ptr == NULL) continue;
		
		for( i = 0; i < key[0].keyNum; i++ ){
			//左辺を探す
			if(strcasecmp( buf, key[i].key ) == 0){
				//見つかったらTYPE別に値をget
				if( key[i].type & INI_TYPE_HEX )
				{
					INI_Get_Hex(ptr, key[i].value.u, key[i].defaultValue.u);
					if( key[i].ex != NULL ){
						if( *key[i].value.u > *(int *)key[i].ex ) *key[i].value.u = key[i].defaultValue.u;
					}
				}
				else if( key[i].type & INI_TYPE_BUTTON )
				{
					INI_Get_Key(ptr, key[i].value.u, key[i].defaultValue.u);
				}
				else if( key[i].type & INI_TYPE_STRING )
				{
					INI_Get_String(ptr, key[i].value.s, key[i].defaultValue.s );
				}
				else if( key[i].type & INI_TYPE_LIST )
				{
					INI_Get_Number_From_List(ptr, key[i].value.i, key[i].defaultValue.i, (char **)key[i].ex);
				}
				else if( key[i].type & INI_TYPE_BOOL )
				{
					INI_Get_Bool(ptr, key[i].value.b, key[i].defaultValue.b);
				}
			}
		}

	}

	sceIoClose(fd);
	return 0;
}

/*------------------------------------------------------------------
-----if INI_TYPE_HEX-----
@params : ex
          最大値が必要ならexにその最大値の値の入った変数のポインタ(exの中身の値が変更されることはない(=const))
          必要ないならNULL
-----if INI_TYPE_LIST-----
@params : ex
          const char *list[] = {
               "1st",
               "2nd",
               "3rd",
               NULL
          }
          のような変数のポインタ
          (exの中身の値が変更されることはない(=const))
-----the others-----
@params : ex
          set NULL
          


-----common-----
@params : keyName
@params : value
@params : defaultValue
@params : type
@returns : 
             < 0 on error
            == 0 no problem
--------------------------------------------------------------------*/

void INI_Add_Hex(INI_Key *key, char *keyName,  u32 *value, u32 defaultValue, int *ex)
{
	key[key[0].keyNum].value.u = value;
	key[key[0].keyNum].defaultValue.u = defaultValue;

	key[key[0].keyNum].type = INI_TYPE_STATIC;
	key[key[0].keyNum].type |= INI_TYPE_HEX;
	
	key[key[0].keyNum].ex = ex;

	//左辺をセット
	strcpy(key[key[0].keyNum].key,keyName);

	key[0].keyNum++;
}

void INI_Add_Button(INI_Key *key, char *keyName,  u32 *value, u32 defaultValue)
{
	key[key[0].keyNum].value.u = value;
	key[key[0].keyNum].defaultValue.u = defaultValue;

	key[key[0].keyNum].type = INI_TYPE_STATIC;
	key[key[0].keyNum].type |= INI_TYPE_BUTTON;
	
//	key[key[0].keyNum].ex = ex;
	key[key[0].keyNum].ex = NULL;

	//左辺をセット
	strcpy(key[key[0].keyNum].key,keyName);
	
	key[0].keyNum++;
}

void INI_Add_String(INI_Key *key, char *keyName, char *value, char *defaultValue)
{
	key[key[0].keyNum].value.s = value;
	key[key[0].keyNum].defaultValue.s = defaultValue;

	key[key[0].keyNum].type = INI_TYPE_STATIC;
	key[key[0].keyNum].type |= INI_TYPE_STRING;
	
//	key[key[0].keyNum].ex = ex;
	key[key[0].keyNum].ex = NULL;

	//左辺をセット
	strcpy(key[key[0].keyNum].key,keyName);
	
	key[0].keyNum++;
}

void INI_Add_List(INI_Key *key, char *keyName, int *value, int defaultValue,  const char *list[] )
{
	key[key[0].keyNum].value.i = value;
	key[key[0].keyNum].defaultValue.i = defaultValue;

	key[key[0].keyNum].type = INI_TYPE_STATIC;
	key[key[0].keyNum].type |= INI_TYPE_LIST;
	
	key[key[0].keyNum].ex = list;

	//左辺をセット
	strcpy(key[key[0].keyNum].key,keyName);

	key[0].keyNum++;
}

void INI_Add_Bool(INI_Key *key, char *keyName, bool *value, bool defaultValue)
{
	key[key[0].keyNum].value.b = value;
	key[key[0].keyNum].defaultValue.b = defaultValue;

	key[key[0].keyNum].type = INI_TYPE_STATIC;
	key[key[0].keyNum].type |= INI_TYPE_BOOL;
	
	key[key[0].keyNum].ex = NULL;

	//左辺をセット
	strcpy(key[key[0].keyNum].key,keyName);

	key[0].keyNum++;
}

void INI_Init_Key(INI_Key *key)
{
	key[0].keyNum = 0;
}






int INI_Write_Conf(const char *ms_path, INI_Key *key, const char *lineFeedCode)
{
	SceUID fd,fdw;
	int readSize,i;
	char buf[256],tmp[256];
	char ms_write_path[128];
	char *ptr,**listPtr;
	u32 flag = 0;
	

	strcpy(ms_write_path, ms_path);
	strcat(ms_write_path, ".tmp");

	fdw = sceIoOpen(ms_write_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if( fdw < 0 ) return -1;
	
	fd = sceIoOpen(ms_path, PSP_O_RDONLY, 0777);
	

	while( fd >= 0 ){
		readSize = read_line_file_keepn(fd,buf,256);
		if( readSize <= 0 ) break;
		
		
		
		strcpy(tmp,buf);
		//余計なスペースを削除する
		removeSpace(buf);
		//コメントや改行だけの行など飛ばしてもいい行なら
		if( buf[0] == '#' || buf[0] == '\n' || buf[0] == '\0' ){
			sceIoWrite(fdw,tmp,readSize);
			continue;
		}
		ptr = ch_token(buf, "=");
		if(ptr == NULL) continue; //ダメな行
		
		
		for( i = 0; i < key[0].keyNum; i++ ){
			if(strcasecmp( buf, key[i].key ) == 0  && (!(flag & (1<<i))) ){
				flag |= (1<<i);
				if( key[i].type & INI_TYPE_BUTTON )
				{
					tmp[0] = '\0';
					GET_KEY_NAME_FOR_CONF(*key[i].value.u,tmp);
					ptr = strrchr(tmp, '+');
					if( ptr != NULL ) ptr[-1] = '\0';
					sprintf(buf,"%s = %s%s",key[i].key,tmp,lineFeedCode);
				}
				else if( key[i].type & INI_TYPE_BOOL )
				{
					sprintf(buf,"%s = %s%s",key[i].key, *key[i].value.b?"true":"false", lineFeedCode);
				}
				else if( key[i].type & INI_TYPE_LIST )
				{
					listPtr = (char **)key[i].ex;
					sprintf(buf,"%s = %s%s", key[i].key, listPtr[*key[i].value.i], lineFeedCode);
				}
				else if( key[i].type & INI_TYPE_HEX )
				{
					sprintf(buf,"%s = %x%s", key[i].key, *key[i].value.u, lineFeedCode);
				}
				else if( key[i].type & INI_TYPE_STRING )
				{
					sprintf(buf,"%s = \"%s\"%s", key[i].key, key[i].value.s, lineFeedCode);
				}
				sceIoWrite(fdw,buf,strlen(buf));
			}
		}
	}


	for( i = 0; i < key[0].keyNum; i++ ){
		if( (!(flag & (1<<i))) ){
			flag |= (1<<i);
			if( key[i].type & INI_TYPE_BUTTON )
			{
				tmp[0] = '\0';
				GET_KEY_NAME_FOR_CONF(*key[i].value.u,tmp);
				ptr = strrchr(tmp, '+');
				if( ptr != NULL ) ptr[-1] = '\0';
				sprintf(buf,"%s = %s%s",key[i].key,tmp,lineFeedCode);
			}
			else if( key[i].type & INI_TYPE_BOOL )
			{
				sprintf(buf,"%s = %s%s",key[i].key, *key[i].value.b?"true":"false", lineFeedCode);
			}
			else if( key[i].type & INI_TYPE_LIST )
			{
				listPtr = (char **)key[i].ex;
				sprintf(buf,"%s = %s%s", key[i].key, listPtr[*key[i].value.i], lineFeedCode);
			}
			else if( key[i].type & INI_TYPE_HEX )
			{
				sprintf(buf,"%s = %x%s", key[i].key, *key[i].value.u, lineFeedCode);
			}
			else if( key[i].type & INI_TYPE_STRING )
			{
				sprintf(buf,"%s = \"%s\"%s", key[i].key, key[i].value.s, lineFeedCode);
			}
			sceIoWrite(fdw,buf,strlen(buf));
		}
	}


	if( fd >= 0 ) sceIoClose(fd);
	sceIoClose(fdw);
	
	sceIoRemove(ms_path);
	sceIoRename(ms_write_path,ms_path);
	sceIoRemove(ms_write_path);
	
	return 0;
}




/*

// @return:
// true  とばしていい行
// false とばしちゃダメ
bool is_skiping_line(char *str)
{
	int i;
	for( i = 0; str[i] != '\0'; i++ ){
		if( str[i] == ' ' || str[i] == '\r' ){
			continue;
		}else if( str[i] == '#' || str[i] == '\n' || str[i] == '\0' ){
			return true;
		}
	}
	return false;
}

*/
