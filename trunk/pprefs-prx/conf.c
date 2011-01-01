// キーコンフィグ

// ヘッダー
#include <pspkernel.h>
#include <pspctrl.h>
#include <string.h>
#include <stdio.h>
#include "conf.h"

// プロトタイプ宣言
int Check_EOF(SceUID fd);
u32 Conv_Key(const char *buf);
char *ch_token(char *str, const char *delim);
u32 Get_Key(const char *str);
bool Get_Bool(const char *str,bool defaultValue);

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
	else if(strcasecmp(buf, "HOLD") == 0)		return PSP_CTRL_HOLD;
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
	
	// 最後まで
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
	key->bootKey = PSP_CTRL_HOME;
	key->swapButton = false;
	
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
		}
	}

	sceIoClose(fd);
	return 0;
}

