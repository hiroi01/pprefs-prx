#ifndef CISO_H_

#define CISO_H_

#include "file.h"


int get_umd_id(char* id, const char* path, file_type type);


//form ciso.c in iso tool



typedef struct ciso_header
{
    unsigned char magic[4];         /* +00 : 'C','I','S','O'                 */
    unsigned long header_size;      /* +04 : header size (==0x18)            */
    unsigned long long total_bytes; /* +08 : number of original data size    */
    unsigned long block_size;       /* +10 : number of compressed block size */
    unsigned char ver;              /* +14 : version 01                      */
    unsigned char align;            /* +15 : align of index value            */
    unsigned char rsv_06[2];        /* +16 : reserved                        */
}CISO_H;

#define CISO_HEADER_SIZE (0x18)

/*---------------------------------------------------------------------------
  deflateの解凍を行う
    char* o_buff 解凍先
    int o_size   解凍先バッファサイズ
    char* i_buff 入力
    int i_size   入力サイズ

    返値 解凍後のサイズ / エラーの場合は負を返す
---------------------------------------------------------------------------*/
int inflate_cso(char* o_buff, int o_size, const char* i_buff, int i_size);

/*---------------------------------------------------------------------------
  CSOから読込む
    char *buf        読込みバッファ
    const char *path パス
    int pos          読込み位置
    int size         読込みサイズ

    返値 実際に読み込んだ長さ / エラーの場合は負を返す
---------------------------------------------------------------------------*/
int cso_read(char *buf, const char *path, int pos, int size);

/*---------------------------------------------------------------------------
  CSOから連続で読込む
    char *buf 読込みバッファ
    SceUID fp ファイルポインタ
    int pos   読込み位置
    int size  読込みサイズ

    返値 実際に読み込んだ長さ / エラーの場合は負を返す

    事前にsceIoOpen / 終了後にsceIoCloseが必要
---------------------------------------------------------------------------*/
int cso_read_fp(char *buf, SceUID fp, int pos, int size);

#endif