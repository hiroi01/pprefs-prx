/*
 * file.c
 *
 *  Created on: 2009/12/26
 *      Author: takka
 */


 /*
 * iso toolのソースより
 *
 * mod by hiroi01
 */

#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef EMU
#include <pspkernel.h>
#include <pspmscm.h>
#endif




#include "button.h"
#include "file.h"
#include "pprefsmenu.h"
#include "common.h"

#define FIO_CST_SIZE    0x0004

//--------------------------------------------------------


typedef enum {
  ERR_OPEN          = -1,
  ERR_READ          = -2,
  ERR_WRITE         = -3,
  ERR_SEEK          = -4,
  ERR_CLOSE         = -5,

  ERR_DECRYPT       = -6,
  ERR_NOT_CRYPT     = -7,

  ERR_DEFLATE       = -8,
  ERR_DEFLATE_SIZE  = -9,
  ERR_INFLATE       = -10,

  ERR_INIT          = -11,

  ERR_PRX           = -12,

  ERR_NOT_FOUND     = -13,
  ERR_SIZE_OVER     = -14,

  ERR_CHG_STAT      = -15,

  ERR_NO_UMD        = -16,

  ERR_RENAME        = -17,

  ERR_NO_MEMORY     = -18,

} err_msg_num;

#define YES                 (1)
#define NO                  (0)

#define DONE                (0)
#define CANCEL              (-1)
//--------------------------------------------------------




int compare_dir_int(const void* c1, const void* c2);
int compare_dir_str(const void* c1, const void* c2);
int compare_dir_dir(const void* c1, const void* c2);

/*---------------------------------------------------------------------------
  ファイルサイズ変更 // 動作しません
  const char *path : パス
  int length       : サイズ

  return int       : 変更後のファイルサイズ, エラーの場合はERR_CHG_STATを返す
---------------------------------------------------------------------------*/
/*
int file_truncate(const char *path, int length)
{
    SceIoStat psp_stat;
    int ret;

    psp_stat.st_size = length;
    ret = sceIoChstat(path, &psp_stat, FIO_CST_SIZE);
    if(ret < 0)
      ret = ERR_CHG_STAT;

    return ret;
}
*/


// ソート時の優先順位
const char dir_type_sort_default[] = {
    'c', // TYPE_ISO
    'c', // TYPE_CSO
    'b', // TYPE_DIR
    'a', // TYPE_UMD
    'c', // TYPE_SYS
    'c', // TYPE_PBT
    'c', // TYPE_PRX
    'a', // TYPE_TXT
    'c', // TYPE_ETC
};

int compare_dir_str(const void* c1, const void* c2)
{
  return strcasecmp(&(((dir_t *)c1)->sort_type), &(((dir_t *)c2)->sort_type));
}

/*---------------------------------------------------------------------------
  ディレクトリ読取り
  dir_t dir[]      : dir_t配列のポインタ
  const char *path : パス

  return int       : ファイル数, dir[0].numにも保存される
---------------------------------------------------------------------------*/
int read_dir(dir_t dir[], const char *path, int dir_only,char *dir_type_sort)
{
  SceUID dp;
  SceIoDirent entry;
  int num;
  int file_num = 0;
  if( dir_type_sort == NULL ) dir_type_sort = (char *)dir_type_sort_default;
  checkMs();
//  int ret  = check_ms();
/*
  if(dir_only == 0)
  {
    strcpy(dir[0].name, "[UMD DRIVE]");
    dir[0].type = TYPE_UMD;
    dir[file_num].sort_type = dir_type_sort[TYPE_UMD];
    file_num++;
  }
*/
  dp = sceIoDopen(path);
  if(dp >= 0)
  {
    memset(&entry, 0, sizeof(entry));

    while((sceIoDread(dp, &entry) > 0))
    {
      num = strlen(entry.d_name);

      strcpy(dir[file_num].name, entry.d_name);
      switch(entry.d_stat.st_mode & FIO_S_IFMT)
      {
        case FIO_S_IFREG:
          if(dir_only == 0)
          {
            if(strncasecmp(&entry.d_name[num - 4], ".iso", 4) == 0)
            {
              dir[file_num].type = TYPE_ISO;
              dir[file_num].sort_type = dir_type_sort[TYPE_ISO];
              file_num++;
            }
            else if(strncasecmp(&entry.d_name[num - 4], ".cso", 4) == 0)
            {
              dir[file_num].type = TYPE_CSO;
              dir[file_num].sort_type = dir_type_sort[TYPE_CSO];
              file_num++;
            }
            else if(strncasecmp(entry.d_name, "PBOOT.PBP", 4) == 0)
            {
              dir[file_num].type = TYPE_PBT;
              dir[file_num].sort_type = dir_type_sort[TYPE_PBT];
              file_num++;
            }
            else if(strncasecmp(&entry.d_name[num - 4], ".prx", 4) == 0)
            {
              dir[file_num].type = TYPE_PRX;
              dir[file_num].sort_type = dir_type_sort[TYPE_PRX];
              file_num++;
            }
            else if(strncasecmp(&entry.d_name[num - 4], ".txt", 4) == 0)
            {
              dir[file_num].type = TYPE_TXT;
              dir[file_num].sort_type = dir_type_sort[TYPE_TXT];
              file_num++;
            }
            else
            {
              dir[file_num].type = TYPE_ETC;
              dir[file_num].sort_type = dir_type_sort[TYPE_ETC];
              file_num++;
            }
          }
          break;

        case FIO_S_IFDIR:
          if((strcmp(&entry.d_name[0], ".") != 0) && (strcmp(&entry.d_name[0], "..") != 0))
          {
            dir[file_num].type = TYPE_DIR;
            dir[file_num].sort_type = dir_type_sort[TYPE_DIR];
            file_num++;
          }
          break;
      }
    }
    sceIoDclose(dp);
  }

  qsort(dir, file_num, sizeof(dir_t), compare_dir_str);

//  dir[0].num = file_num;

  return file_num;
}

/*---------------------------------------------------------------------------
  ディレクトリ読取り
  dir_t dir[]      : dir_t配列のポインタ
  const char *path : パス

  return int       : ファイル数, dir[0].numにも保存される
---------------------------------------------------------------------------*/
/*
int read_dir_2(dir_t dir[], const char *path, int read_dir_flag)
{
  SceUID dp;
  SceIoDirent entry;
  int num;
  int file_num = 0;
  int ret;

  ret = check_ms();

  dp = sceIoDopen(path);
  if(dp >= 0)
  {
    memset(&entry, 0, sizeof(entry));

    while((sceIoDread(dp, &entry) > 0))
    {
      num = strlen(entry.d_name);

      strcpy(dir[file_num].name, entry.d_name);
      switch(entry.d_stat.st_mode & FIO_S_IFMT)
      {
        case FIO_S_IFREG:
          dir[file_num].type = TYPE_ETC;
          dir[file_num].sort_type = dir_type_sort[TYPE_ETC];
          file_num++;
          break;

        case FIO_S_IFDIR:
          if(read_dir_flag == 1)
          {
            if((strcmp(&entry.d_name[0], ".") != 0) && (strcmp(&entry.d_name[0], "..") != 0))
            {
              dir[file_num].type = TYPE_DIR;
              dir[file_num].sort_type = dir_type_sort[TYPE_DIR];
              file_num++;
            }
          }
          break;
      }
    }
    sceIoDclose(dp);
  }

  dir[0].num = file_num;

  // ファイル名でソート
//  qsort(&dir[0], file_num - 1, sizeof(dir_t), compare_dir_str);

  return file_num;
}
*/

/*---------------------------------------------------------------------------
  MSのリード
  void* buf        : 読取りバッファ
  const char* path : パス
  int pos          : 読込み開始場所
  int size         : 読込みサイズ, 0を指定すると全てを読込む

  return int       : 読込みサイズ, エラーの場合は ERR_OPEN/ERR_READ を返す
---------------------------------------------------------------------------*/
/*
int ms_read(void* buf, const char* path, int pos, int size)
{
  SceUID fp;
  SceIoStat stat;
  int ret = ERR_OPEN;

  if(size == 0)
  {
    pos = 0;
    sceIoGetstat(path, &stat);
    size = stat.st_size;
  }

  fp = sceIoOpen(path, PSP_O_RDONLY, 0777);
  if(fp > 0)
  {
    sceIoLseek32(fp, pos, PSP_SEEK_SET);
    ret = sceIoRead(fp, buf, size);
    sceIoClose(fp);
    if(ret < 0)
      ret = ERR_READ;
  }
  return ret;
}
*/

/*---------------------------------------------------------------------------
  MSへのライト
  const void* buf  : 書込みバッファ
  const char* path : パス
  int pos          : 書込み開始場所
  int size         : 書込みサイズ

  return int       : 書込んだサイズ, エラーの場合は ERR_OPEN/ERR_WRITE を返す
---------------------------------------------------------------------------*/
/*
int ms_write(const void* buf, const char* path, int pos, int size)
{
  SceUID fp;
  int ret = ERR_OPEN;

  fp = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777);
  if(fp > 0)
  {
    sceIoLseek32(fp, pos, PSP_SEEK_SET);
    ret = sceIoWrite(fp, buf, size);
    sceIoClose(fp);
    if(ret < 0)
      ret = ERR_WRITE;
  }
  return ret;
}

int ms_write_apend(const void* buf, const char* path, int pos, int size)
{
  SceUID fp;
  int ret = ERR_OPEN;

  fp = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
  if(fp > 0)
  {
    sceIoLseek32(fp, pos, PSP_SEEK_SET);
    ret = sceIoWrite(fp, buf, size);
    sceIoClose(fp);
    if(ret < 0)
      ret = ERR_WRITE;
  }
  return ret;
}
*/
/*---------------------------------------------------------------------------
  ファイルリード
---------------------------------------------------------------------------*/
/*
int file_read(void* buf, const char* path, file_type type, int pos, int size)
{
  int ret = ERR_OPEN;

  switch(type)
  {
    case TYPE_ISO:
    case TYPE_SYS:
      ret = ms_read(buf, path, pos, size);
      break;

    case TYPE_CSO:
      ret = cso_read(buf, path, pos, size);
      break;

    case TYPE_UMD:
      ret = umd_read(buf, path, pos, size);
      break;

    default:
      break;
  }
  return ret;
}
*/

/*---------------------------------------------------------------------------
  ファイルライト
---------------------------------------------------------------------------*/
/*
int file_write(const void* buf, const char* path, file_type type, int pos, int size)
{
  u32 ret = ERR_OPEN;

  switch(type)
  {
    case TYPE_ISO:
      ret = ms_write(buf, path, pos, size);
      break;

    case TYPE_CSO:
      ret = cso_write(buf, path, pos, size, 9);
      break;

    case TYPE_UMD:
      break;

    default:
      break;
  }
  return ret;
}

// FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH
int set_file_mode(const char* path, int bits)
{
  SceIoStat stat;
  int ret;

  ret = sceIoGetstat(path, &stat);

  if(ret >= 0)
  {
    stat.st_mode |= (bits);
    ret = sceIoChstat(path, &stat, (FIO_S_IRWXU | FIO_S_IRWXG | FIO_S_IRWXO));
  }
  if(ret < 0)
    ret = ERR_CHG_STAT;

  return ret;
}
*/
/*---------------------------------------------------------------------------
---------------------------------------------------------------------------*/
int up_dir(char *path)
{
  int loop;
  int ret = ERR_OPEN;

  loop = strlen(path) - 2;

  while(path[loop--] != '/')
    ;

  if(path[loop - 1] != ':')
  {
    path[loop + 2] = '\0';
    ret = 0;
  }

  return ret;
}



int read_line(char* str,  SceUID fp, int num)
{
  char buf;
  int len = 0;
  int ret;

  do{
    ret = sceIoRead(fp, &buf, 1);
    if(ret == 1)
    {
      if(buf == '\n')
      {
        str[len] = '\0';
        len++;
        break;
      }
      if(buf != '\r')
      {
        str[len] = buf;
        len++;
      }
    }
  }while((ret > 0) && (len < num));

  return len;
}



//from umd dumper
int read_line_file(SceUID fp, char* line, int num)
{
  char buff[num];
  char* end;
  int len;
  int tmp;

  tmp = 0;
  len = sceIoRead(fp, buff, num);
  // エラーの場合 / on error
  if(len == 0)
    return -1;

  end = strchr(buff, '\n');

  // \nが見つからない場合 / not found \n
  if(end == NULL)
  {
    buff[num - 1] = '\0';
    strcpy(line, buff);
    return len;
  }

  end[0] = '\0';
  if((end != buff) && (end[-1] == '\r'))
  {
    end[-1] = '\0';
    tmp = -1;
  }

  strcpy(line, buff);
  sceIoLseek(fp, - len + (end - buff) + 1, SEEK_CUR);
  return end - buff + tmp;
}




int read_line_file_keepn(SceUID fp, char* line, int num)
{
  char buff[num];
  char* end;
  int len;
  int tmp;

  tmp = 1;
  len = sceIoRead(fp, buff, num);
  // エラーの場合 / on error
  if(len == 0)
    return -1;

  end = strchr(buff, '\n');

  // \nが見つからない場合 / not found \n
  if(end == NULL)
  {
    buff[num - 1] = '\0';
    strcpy(line, buff);
    return len;
  }

  //この処理は正しい？
  //buffの大きさをこえないように、末尾に\0を追加したい
  if( &end[1] < &buff[num] ){
    end[1] = '\0';
    /*
    if( (end[0] == '\r') )
    {
      end[0] = '\0';
      tmp = 0;
    }
    */
  }else{
    end[0] = '\0';
    tmp = 0;
  }

  strcpy(line, buff);
  sceIoLseek(fp, - len + (end - buff) + 1, SEEK_CUR);
  return end - buff + tmp;
}



/*
int get_umd_sector(const char* path, file_type type)
{
  int size = 0;
  int ret;

  ret = file_read(&size, path, type, 0x8050, 4); // 0x50から4byteがセクタ数
  if(ret < 0)
    size = ret;

  return size;
}
*/
/*
int get_umd_id(char* id, const char* path, file_type type)
{
  int ret;
  // 0x8373から10byteがUMD ID
  ret = file_read(id, path, type, 0x8373, 10);
  if(ret == 10)
    id[10] = '\0';
  else
    strcpy(id, "**********");

  return ret;
}
*/
/*
int get_umd_name(char* name, char* e_name, const char* id, int mode)
{
  static char buf[1024*256]; // 256KB
  static int init = 0;
  char *ptr;
  int ptr2 = 0;
  int ret = 0;

  if((init == 0)||(mode == 1))
  {
    ms_read(buf, "UMD_ID.csv", 0, 0);
    init = 1;
    if(mode == 1)
      return 0;
  }

  ptr = strstr((const char *)buf, id);

  if(ptr != NULL)
  {
    ptr += 11;

    while(*ptr == '\\')
      ptr++;

    while(*ptr != '\\')
    {
      name[ptr2] = *ptr;
      ptr++;
      ptr2++;
    }
    name[ptr2] = '\0';

    while(*ptr == '\\')
      ptr++;

    ptr2 = 0;
    while((*ptr != '\r') && (*ptr != '\n'))
    {
      e_name[ptr2] = *ptr;
      ptr++;
      ptr2++;
    }
    e_name[ptr2] = '\0';

  }
  else
  {
    ret = -1;
    name[0] = '\0';
    e_name[0] = '\0';
  }

  return ret;
}

int get_ms_free()
{
    unsigned int buf[5];
    unsigned int *pbuf = buf;
    int free = 0;
    int ret;

    //    buf[0] = 合計クラスタ数
    //    buf[1] = フリーなクラスタ数(ギリギリまで使いたいならこっち)
    //    buf[2] = フリーなクラスタ数(buf[3]やbuf[4]と掛けて1MB単位になるようになってる)
    //    buf[3] = セクタ当たりバイト数
    //    buf[4] = クラスタ当たりセクタ数
    ret = sceIoDevctl("ms0:", 0x02425818, &pbuf, sizeof(pbuf), 0, 0);

    if(ret >= 0)
      free = buf[1] * ((buf[3] * buf[4]) / 1024);// 空き容量取得(kb)

    return free;
}

*/

int checkMs(void)
{
	int ret = 0;
	SceUID dp = sceIoDopen("ms0:/");
	if(dp < 0){
		ret = check_ms();
	}else{
		sceIoDclose(dp);
	}
	
	return ret;
}

int check_ms()
{
  SceUID ms;
  int ret = DONE;

  ms = MScmIsMediumInserted();
  if(ms <= 0)
  {
 //   msg_win("", 0, MSG_CLEAR, 0);
 //   msg_win("Memory Stickを入れて下さい", 1, MSG_WAIT, 0);
 	makeWindow(LIBM_CHAR_WIDTH*9 ,LIBM_CHAR_HEIGHT*9 ,LIBM_CHAR_WIDTH*30 ,LIBM_CHAR_WIDTH*14, FG_COLOR,BG_COLOR );
	libmPrint( LIBM_CHAR_WIDTH*10 ,LIBM_CHAR_HEIGHT*10 , FG_COLOR,BG_COLOR,"Memory Stickを入れて下さい");
	libmPrint( LIBM_CHAR_WIDTH*10 ,LIBM_CHAR_HEIGHT*12 + 4 , FG_COLOR,BG_COLOR,"HOME:エスケープ");

    ms = -1;
    while(ms <= 0)
    {
      if( isButtonDown(PSP_CTRL_HOME) ) return -2;
      sceKernelDelayThread(1000);
      ms = MScmIsMediumInserted();
    }
 //   msg_win("", 0, MSG_CLEAR, 0);
//    msg_win("マウント中です", 1, MSG_WAIT, 0);
	libmPrint( LIBM_CHAR_WIDTH*10 ,LIBM_CHAR_HEIGHT*11 + 2 , FG_COLOR,BG_COLOR,"マウント中です");
	sceKernelDelayThread(1000000);
    ret = CANCEL;
  }
  return ret;
}

int check_file(const char* path)
{
  SceIoStat stat;

  return sceIoGetstat(path, &stat);
}


//form ciso.c in iso tool

/*---------------------------------------------------------------------------
  deflateの解凍を行う
    char* o_buff 解凍先
    int o_size   解凍先バッファサイズ
    char* i_buff 入力
    int i_size   入力サイズ

    返値 解凍後のサイズ / エラーの場合は負を返す
---------------------------------------------------------------------------*/
/*
int inflate_cso(char* o_buff, int o_size, const char* i_buff, int i_size)
{
  z_stream z;
  int size;

  // 初期化
  z.zalloc = Z_NULL;
  z.zfree = Z_NULL;
  z.opaque = Z_NULL;
  z.next_in = Z_NULL;
  z.avail_in = 0;
  if(inflateInit2(&z, -15) != Z_OK)
    return ERR_INFLATE;

  z.next_in = (unsigned char*)i_buff;
  z.avail_in = i_size;
  z.next_out = (unsigned char*)o_buff;
  z.avail_out = o_size;

  inflate(&z, Z_FINISH);

  // 出力サイズ
  size = o_size - z.avail_out;

  if(inflateEnd(&z) != Z_OK)
    return ERR_INFLATE;

  return size;
}
*/
/*---------------------------------------------------------------------------
  CSOから読込む
    char *buf        読込みバッファ
    const char *path パス
    int pos          読込み位置
    int size         読込みサイズ

    返値 実際に読み込んだ長さ / エラーの場合は負を返す
---------------------------------------------------------------------------*/
/*
int cso_read(char *buf, const char *path, int pos, int size)
{
  SceUID fp;
  int ret;
  int err;

  fp = sceIoOpen(path, PSP_O_RDONLY, 0777);

  if(fp < 0)
    return ERR_OPEN;

  ret = cso_read_fp(buf, fp, pos, size);

  err = sceIoClose(fp);

  if(fp < 0)
    return ERR_CLOSE;

  return ret;
}
*/

/*---------------------------------------------------------------------------
  CSOから連続で読込む
    char *buf 読込みバッファ
    SceUID fp ファイルポインタ
    int pos   読込み位置
    int size  読込みサイズ

    返値 実際に読み込んだ長さ / エラーの場合は負を返す

    事前にsceIoOpen / 終了後にsceIoCloseが必要
---------------------------------------------------------------------------*/
/*
int cso_read_fp(char *buf, SceUID fp, int pos, int size)
{
  static SceUID old_fp = 0;
  static CISO_H header;
  int start_sec;
  int max_sector;
  int end_sec;
  int sector_num;
  unsigned long long int now_pos = 0;
  unsigned long long int next_pos = 0;
  int read_size;
  unsigned int zip_flag;
  char tmp_buf[SECTOR_SIZE * 2];   // 展開済みデータバッファ
  char tmp_buf_2[SECTOR_SIZE * 2]; // 圧縮データ読み込みバッファ
  int ret;
  int err;
  int start_pos;
  int end_pos;

  // ヘッダー読込
  if(old_fp != fp)
  {
    err = sceIoLseek32(fp, 0, PSP_SEEK_SET);
    if(err < 0)
      return ERR_SEEK;

    err = sceIoRead(fp, &header, CISO_HEADER_SIZE);
    if(err < 0)
      return ERR_READ;

    old_fp = fp;
  }

  // 読込セクタ数を計算
  if((pos + size) > header.total_bytes)
    size = header.total_bytes - pos;

  max_sector = header.total_bytes / header.block_size - 1;
  start_sec = pos / SECTOR_SIZE;
  end_sec = (pos + size - 1) / SECTOR_SIZE;
  sector_num = start_sec;

  if(sector_num > max_sector)
    return ERR_SEEK;

  if(end_sec > max_sector)
    end_sec = max_sector;

  ret = 0;
  while(sector_num <= end_sec)
  {
    // セクタ番号からファイル位置と長さを取得
    err = sceIoLseek32(fp, CISO_HEADER_SIZE + (sector_num * 4), PSP_SEEK_SET);
    if(err < 0)
      return ERR_SEEK;

    err = sceIoRead(fp, &now_pos, 4);
    if(err < 0)
      return ERR_READ;

    zip_flag = now_pos & 0x80000000;
    now_pos = (now_pos & 0x7fffffff) << header.align;

    err = sceIoRead(fp, &next_pos, 4);
    if(err < 0)
      return ERR_READ;

    read_size = ((next_pos & 0x7fffffff) << header.align) - now_pos;

    // １セクタを読込
    err = sceIoLseek32(fp, now_pos, PSP_SEEK_SET);
    if(err < 0)
      return ERR_SEEK;

    if(zip_flag != 0)
    {
      // 未圧縮
      err = sceIoRead(fp, tmp_buf, header.block_size);
      if(err < 0)
        return ERR_READ;
    }
    else
    {
      // 圧縮済
      err = sceIoRead(fp, tmp_buf_2, read_size);
      if(err < 0)
        return ERR_READ;
      // バッファに展開
      err = inflate_cso(tmp_buf, header.block_size, tmp_buf_2, read_size);
      if(err < 0)
        return ERR_INFLATE;
    }

    // 指定バッファに転送
    if((sector_num > start_sec) && (sector_num < end_sec))
    {
      // 全転送
      memcpy(buf, tmp_buf, header.block_size);
      read_size = header.block_size;
    }
    else if((sector_num == start_sec) || (sector_num == end_sec))
    {
      // 部分転送
      start_pos = 0;
      end_pos = header.block_size;
      if(sector_num == start_sec)
        start_pos = pos - (start_sec * header.block_size);
      if(sector_num == end_sec)
        end_pos = (pos + size) - (end_sec * header.block_size);
      read_size = end_pos - start_pos;
      memcpy(buf, &tmp_buf[start_pos], read_size);
    }

    buf += read_size;
    ret += read_size;
    sector_num++;
  }

  return ret;
}
*/
