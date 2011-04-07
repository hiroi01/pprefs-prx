// from libmenu


#ifndef MEMORY_H
#define MEMORY_H


/*=========================================================
	マクロ
=========================================================*/
#define MEMORY_PAGE_SIZE 256

/* どこぞから拝借した、メモリアラインを調整するマクロ */
#define MEMORY_ALIGN_ADDR( align, addr ) ( ( (uintptr_t)( addr ) + ( align ) - 1 ) & ( ~( ( align ) - 1 ) ) )

/*=========================================================
	ローカルマクロ
=========================================================*/
#define MEMORY_POWER_OF_TWO( x ) ( ! ( ( x ) & ( ( x ) - 1 ) ) )

typedef struct
{
	SceUID  blockId;
	SceSize size;
} memory_header;


/*=========================================================
	型宣言
=========================================================*/
typedef enum {
	MEMORY_KERN_HI = 1,
	MEMORY_USER,
	MEMORY_KERN_HI_MIRROR,
	MEMORY_KERN_LO,
	MEMORY_VOLATILE,
	MEMORY_USER_MIRROR
} MemoryPartition;


/*=========================================================

=========================================================*/

void *memoryAllocEx( const char *name, MemoryPartition partition, unsigned int align, SceSize size, int type, void *addr );

#if USE_KERNEL_LIBRARY
void *malloc( size_t size );
void *memalign(size_t boundary, size_t size);
void free( void *memblock );

#define memoryAlloc malloc
#define memoryAlign memalign
#define memoryFree free

#else

void *memoryAlloc( size_t size );
void *memoryAlign(size_t boundary, size_t size);
void memoryFree( void *memblock );

#endif


#endif

