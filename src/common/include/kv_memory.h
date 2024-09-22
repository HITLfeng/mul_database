#ifndef __KV__MEMORY_H__
#define __KV__MEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// jemalloc tcmalloc
// 内核空间/用户空间 管理堆内存、共享内存（均为虚拟内存）

// 分配内存 如一次从操作系统申请4K 避免内存碎片

// 配置内存池page和block大小
#define MEM_PAGE_SIZE 4096

#define MEM_BLOCK_SIZE_LEVEL1 4
#define MEM_BLOCK_SIZE_LEVEL2 8
#define MEM_BLOCK_SIZE_LEVEL3 16
#define MEM_BLOCK_SIZE_LEVEL4 32
#define MEM_BLOCK_SIZE_LEVEL5 64
#define MEM_BLOCK_SIZE_LEVEL6 128
#define MEM_BLOCK_SIZE_LEVEL7 256
#define MEM_BLOCK_SIZE_LEVEL8 512
#define MEM_BLOCK_SIZE_LEVEL9 1024
// 层级：4，8，16，32，64，128，256，512，1024
#define MEM_POOL_SLOT_CNT 9

#define INVALID_SLOT_INDEX MEM_POOL_SLOT_CNT

typedef struct KVMemoryPool
{
    // |****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|****|
    // memPtr 指向内存块起始地址                      currMemPtr
    // 指向当前正在使用的地址
    void *memPtr;          // 指向内存块起始地址
    void *currMemPtr;      // 指向当前正在使用的地址
    uint32_t memBlockSize; // 内存块大小，如4k一块申请下来后，按照 32
                           // 字节为单位分配，这里可以分为128份 // 这里是 32 字节
    uint32_t freeBlockCnt; // 当前内存块中空闲内存块数量
} KVMemoryPoolT;

typedef struct KVMemoryPoolManager
{
    // 层级：4，8，16，32，64，128，256，512，1024
    KVMemoryPoolT *memoryMangerPool[MEM_POOL_SLOT_CNT];
} KVMemoryPoolManagerT;


Status KVMemoryPoolInit();
void KVMemoryPoolUninit();

// 服务端申请释放内存接口 方便资源统一管理 不要调用malloc
void *KVMemAlloc(uint32_t allocSize);
void KVMemFree(void *ptr, uint32_t allocSize);


#ifdef __cplusplus
}
#endif

#endif // __KV__MEMORY_H__
