#include <string.h>
#include <stdbool.h>
#include <pspctrl.h>

#include "libini.h"
#include "iniconfig.h"


// Libiniのサンプルより
void strutilRemoveChar( char *__restrict__ str, const char *__restrict__ search )
{
	int offset = 0,ins_offset = 0;
	
	while( str[offset] != '\0' )
	{
		if( ! strchr( search, str[offset] )  )
		{
			str[ins_offset++] = str[offset];
		}
		
		offset++;
	}
	
	str[ins_offset] = str[offset];
}


/*
	iniSplitのコールバック関数
	指定した区切り文字で分解された値が送られてくる
*/
void callbackToSplit(const char *val,void *opt)
{
	int i;
	int *pad = (int*)opt;
	
	//Pad Codeへ変換可能な文字列
	
	struct pad_data
	{
		char *name;
		unsigned int code;
	} pad_list[] =
	{
		{ "SELECT"		,PSP_CTRL_SELECT	},
		{ "START"		,PSP_CTRL_START		},
		{ "Up"			,PSP_CTRL_UP		},
		{ "Right"		,PSP_CTRL_RIGHT		},
		{ "Down"		,PSP_CTRL_DOWN		},
		{ "Left"		,PSP_CTRL_LEFT		},
		{ "LTrigger"	,PSP_CTRL_LTRIGGER	},
		{ "RTrigger"	,PSP_CTRL_RTRIGGER	},
		{ "Triangle"	,PSP_CTRL_TRIANGLE	},
		{ "Circle"		,PSP_CTRL_CIRCLE	},
		{ "Cross"		,PSP_CTRL_CROSS		},
		{ "Square"		,PSP_CTRL_SQUARE	},
		{ "HOME"		,PSP_CTRL_HOME		},
//		{ "HOLD"		,PSP_CTRL_HOLD		},
		{ "NOTE"		,PSP_CTRL_NOTE		},
		{ "SCREEN"		,PSP_CTRL_SCREEN	},
		{ "VolUp"		,PSP_CTRL_VOLUP		},
		{ "VolDown"		,PSP_CTRL_VOLDOWN	},
//		{ "WLAN"		,PSP_CTRL_WLAN_UP	},
//		{ "REMOTE"		,PSP_CTRL_REMOTE	},
//		{ "DISC"		,PSP_CTRL_DISC		},
//		{ "MS"			,PSP_CTRL_MS		},
	};
	
	
	const int pad_list_size = sizeof(pad_list) / sizeof(struct pad_data);
	
	
	//pad_listの各Pad Nameと比較
	for( i = 0 ; i < pad_list_size ; i++ )
	{
		//大文字小文字区別せず比較する
		//一致する物があればPad Codeへ変換し、ループを抜ける(=関数終了)
		if( !strcasecmp(val,pad_list[i].name) )
		{
			*pad |= pad_list[i].code;
			break;
		}
	}
}

bool callbackForIni(const char *key, char *val,void *opt){
	if( strcasecmp(key,"BootKey") == 0 ){
		int *bootKey = (int *)opt;
		
		//iniSplitで処理するために
		//不要な文字(タブ、半角/全角スペース）を値から除去
		strutilRemoveChar( val,"\t\x20　" );
		
		//iniSplitValでさらに値を'+'区切りで分解
		//コールバック関数へ 'pad'変数をオプションとして渡して結果を受け取る
		iniSplitVal(val,"+",callbackToSplit,bootKey);
		
		return true;
	}


	return false;
}


int readConfig( const char *file_name , int *buttonNum )
{

	ini_pair list[2];
	ini_data data;
	int bootKey = 0;
	char buf[256];
	bool swapButtonFlag;

	memset(&data,0,sizeof(ini_data));
	memset(list,0,sizeof(list));
	
	data.pair	= list;
	data.size	= sizeof(list);
	data.buf	= buf;
	data.bufLen	= 256;
	
	iniAddKey( &data ,"SwapButton"	,INI_TYPE_BOOL	,&swapButtonFlag	,false	);
	iniAddKey( &data ,"BootKey"		,INI_TYPE_STR ,NULL ,0);
	
	iniLoad(file_name,&data,callbackForIni,&bootKey);
	
	if( swapButtonFlag ){
		buttonNum[0] = 1;
		buttonNum[1] = 0;
	}else{
		buttonNum[0] = 0;
		buttonNum[1] = 1;
	}
	
	if(bootKey == 0){
		bootKey = PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER | PSP_CTRL_NOTE;
	}
	return bootKey;
}