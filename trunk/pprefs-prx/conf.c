//from mp3play

// キーコンフィグ

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
u32 Get_Key(const char *str);
bool Get_Bool(const char *str,bool defaultValue);
int removeSpace(char *str);




//わたされた文字列からスペースを取り除く
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

u32 Get_Key(const char *str)
{
	u32 key = 0;
	char buf[256];
	char *ptr;

	// コピー
	strcpy(buf, str);

	// 最後まで
	do
	{
		ptr = ch_token(buf, "+");
		key |= Conv_Key(buf);

		if(ptr != NULL)
			strcpy(buf, ptr);
	}
	while (ptr != NULL);

	return key;
}

bool Get_Bool(const char *str,bool defaultValue)
{
	bool rtn = defaultValue;
	
	if( strcasecmp(str,"true") == 0 ){
		rtn = true;
	}else if(  strcasecmp(str,"false") == 0 ){
		rtn = false;
	}
	
	return rtn;
}

int Read_Conf(const char *path, Conf_Key *key)
{
	SceUID fd;
	char buf[256];
	char ms_path[128];
	char *ptr;

	// コピー
	strcpy(ms_path, path);

	// パス取得
	ptr = strrchr(ms_path, '/');
	ptr++;
	*ptr++	= 'p';
	*ptr++	= 'p';
	*ptr++	= 'r';
	*ptr++	= 'e';
	*ptr++	= 'f';
	*ptr++	= 's';
	*ptr++	= '.';
	*ptr++	= 'i';
	*ptr++	= 'n';
	*ptr++	= 'i';
	*ptr	= '\0';

	// 読み取れなっかった場合はデフォルトの設定
	Set_Default_Conf(key);
	
	
	// 設定ファイル・オープン
	fd = sceIoOpen(ms_path, PSP_O_RDONLY, 0777);
	if(fd < 0)
	{
		return -1;
	}

	// ファイル終端までループ
	while (Check_EOF(fd) == 0)
	{
		// 一行読み込む
		Read_Line(fd, buf, 255);

		// コメントと改行は除外
		if(buf[0] != '#' && buf[0] != '\n' && buf[0] != '\0')
		{
			ptr = ch_token(buf, "=");
			if(ptr == NULL) continue;

			if(strcasecmp(buf, "BOOTKEY") == 0)
			{
				key->bootKey = Get_Key(ptr);
			}
			else if(strcasecmp(buf, "SWAPBUTTON") == 0)
			{
				key->swapButton = Get_Bool(ptr,false);
			}
			else if(strcasecmp(buf, "BOOTMESSAGE") == 0)
			{
				key->bootMessage = Get_Bool(ptr,true);
			}
			else if(strcasecmp(buf, "ONEPUSHRESTART") == 0)
			{
				key->onePushRestart = Get_Bool(ptr,false);
			}
			/*
			else if(strcasecmp(buf, "???") == 0)
			{
				key->??? = ???
			}
			*/
		}
	}

	sceIoClose(fd);
	return 0;
}



int Write_Conf(const char *path, Conf_Key *key)
{
	SceUID fd,fdw;
	int readSize;
	char buf[256],tmp[256];
	char ms_path[128],ms_write_path[128];
	char *ptr;
	u32 flag = 0;
	

	// コピー
	strcpy(ms_path, path);

	// パス取得
	ptr = strrchr(ms_path, '/');
	ptr++;
	*ptr++	= 'p';
	*ptr++	= 'p';
	*ptr++	= 'r';
	*ptr++	= 'e';
	*ptr++	= 'f';
	*ptr++	= 's';
	*ptr++	= '.';
	*ptr++	= 'i';
	*ptr++	= 'n';
	*ptr++	= 'i';
	*ptr	= '\0';

	strcpy(ms_write_path, path);

	ptr = strrchr(ms_write_path, '/');
	ptr++;
	*ptr++	= 'p';
	*ptr++	= 'p';
	*ptr++	= 'r';
	*ptr++	= 'e';
	*ptr++	= 'f';
	*ptr++	= 's';
	*ptr++	= '.';
	*ptr++	= 'i';
	*ptr++	= 'n';
	*ptr++	= 'i';
	*ptr++	= '.';
	*ptr++	= 't';
	*ptr++	= 'm';
	*ptr++	= 'p';
	*ptr	= '\0';

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
		
		if(strcasecmp(buf, "BOOTKEY") == 0 && (!(flag & 1)) )
		{
			tmp[0] = '\0';
			GET_KEY_NAME_FOR_CONF(key->bootKey,tmp);
			ptr = strrchr(tmp, '+');
			if( ptr != NULL ) ptr[-1] = '\0';
			sprintf(buf,"BootKey = %s\n",tmp);
			flag |= 1;
		}
		else if(strcasecmp(buf, "SWAPBUTTON") == 0 && (!(flag & 2)) )
		{
			sprintf(buf,"SwapButton = %s\n",key->swapButton?"true":"false");
			flag |= 2;
		}
		else if(strcasecmp(buf, "BOOTMESSAGE") == 0 && (!(flag & 4)) )
		{
			sprintf(buf,"BootMessage = %s\n",key->bootMessage?"true":"false");
			flag |= 4;
		}
		else if(strcasecmp(buf, "ONEPUSHRESTART") == 0 && (!(flag & 8)) )
		{
			sprintf(buf,"OnePushRestart = %s\n",key->onePushRestart?"true":"false");
			flag |= 8;
		}
		/*
			else if(strcasecmp(buf, "???") == 0 && (!(flag & ?)) )
			{
				sprintf(buf,"??? = %s\n", ???);
				flag |= ?;
			}
		*/
		else
		{
			//ダメな行
			continue;
		}
		sceIoWrite(fdw,buf,strlen(buf));
	}
	
	
	if( (!(flag & 1)) )
	{
		tmp[0] = '\0';
		GET_KEY_NAME_FOR_CONF(key->bootKey,tmp);
		ptr = strrchr(tmp, '+');
		if( ptr != NULL ) ptr[-1] = '\0';
		sprintf(buf,"BootKey = %s\n",tmp);
		sceIoWrite(fdw,buf,strlen(buf));
		flag |= 1;
	}
	if( (!(flag & 2)) )
	{
		sprintf(buf,"SwapButton = %s\n",key->swapButton?"true":"false");
		sceIoWrite(fdw,buf,strlen(buf));
		flag |= 2;
	}
	if( (!(flag & 4)) )
	{
		sprintf(buf,"BootMessage = %s\n",key->bootMessage?"true":"false");
		sceIoWrite(fdw,buf,strlen(buf));
		flag |= 4;
	}
	if( (!(flag & 4)) )
	{
		sprintf(buf,"BootMessage = %s\n",key->bootMessage?"true":"false");
		sceIoWrite(fdw,buf,strlen(buf));
		flag |= 4;
	}
	if( (!(flag & 8)) )
	{
		sprintf(buf,"OnePushRestart = %s\n",key->onePushRestart?"true":"false");
		sceIoWrite(fdw,buf,strlen(buf));
		flag |= 8;
	}
	/*
	if( (!(flag & ?)) )
	{
		sprintf(buf,"??? = %s\n", ???);
		sceIoWrite(fdw,buf,strlen(buf));
		flag |= ?;
	}
	*/

	if( fd >= 0 ) sceIoClose(fd);
	sceIoClose(fdw);
	
	sceIoRemove(ms_path);
	sceIoRename(ms_write_path,ms_path);
	sceIoRemove(ms_write_path);
	
	return 0;
}

void Set_Default_Conf(Conf_Key *key)
{
	key->bootKey = PSP_CTRL_HOME;
	key->swapButton = false;
	key->bootMessage = true;
	key->onePushRestart = false;
	/*
	key->??? = ???;
	*/
}

//4箇所



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
