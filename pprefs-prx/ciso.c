#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ciso.h"
#include "common.h"


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



/*---------------------------------------------------------------------------
  ファイルリード
---------------------------------------------------------------------------*/

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
/*
    case TYPE_UMD:
      ret = umd_read(buf, path, pos, size);
      break;
*/
    default:
      break;
  }
  return ret;
}






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



//form ciso.c in iso tool

/*---------------------------------------------------------------------------
  deflateの解凍を行う
    char* o_buff 解凍先
    int o_size   解凍先バッファサイズ
    char* i_buff 入力
    int i_size   入力サイズ

    返値 解凍後のサイズ / エラーの場合は負を返す
---------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------
  CSOから読込む
    char *buf        読込みバッファ
    const char *path パス
    int pos          読込み位置
    int size         読込みサイズ

    返値 実際に読み込んだ長さ / エラーの場合は負を返す
---------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------
  CSOから連続で読込む
    char *buf 読込みバッファ
    SceUID fp ファイルポインタ
    int pos   読込み位置
    int size  読込みサイズ

    返値 実際に読み込んだ長さ / エラーの場合は負を返す

    事前にsceIoOpen / 終了後にsceIoCloseが必要
---------------------------------------------------------------------------*/
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

