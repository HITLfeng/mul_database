#include "include/kv_memory.h"

KVMemoryPoolManagerT *g_kvMemoryPoolManager;

bool IsAllocByMemoryPool(void *ptr)
{
    if ((uint8_t *)(g_kvMemoryPoolManager->memoryMangerPool[0]->memPtr) <= (uint8_t *)ptr &&
        (uint8_t *)ptr < (uint8_t *)(g_kvMemoryPoolManager->memoryMangerPool[0]->memPtr) +
                             MEM_POOL_SLOT_CNT * MEM_PAGE_SIZE)
    {
        return true;
    }
    return false;
}

static uint32_t GetSlotBlockSizeByIdx(uint32_t idx)
{
    DB_ASSERT(idx < MEM_POOL_SLOT_CNT);
    switch (idx)
    {
    case 0:
        return MEM_BLOCK_SIZE_LEVEL1;
    case 1:
        return MEM_BLOCK_SIZE_LEVEL2;
    case 2:
        return MEM_BLOCK_SIZE_LEVEL3;
    case 3:
        return MEM_BLOCK_SIZE_LEVEL4;
    case 4:
        return MEM_BLOCK_SIZE_LEVEL5;
    case 5:
        return MEM_BLOCK_SIZE_LEVEL6;
    case 6:
        return MEM_BLOCK_SIZE_LEVEL7;
    case 7:
        return MEM_BLOCK_SIZE_LEVEL8;
    case 8:
        return MEM_BLOCK_SIZE_LEVEL9;
    default:
        log_error(
            "GetSlotBlockSizeByIdx idx is invalid when exec KVMemoryPoolInit.");
        return 0;
    }
    log_error("GetSlotBlockSizeByIdx idx is invalid when exec KVMemoryPoolInit.");
    return 0;
}

static uint32_t ChooseSlotBlockIdByAllocSize(uint32_t allocSize)
{
    if (allocSize <= MEM_BLOCK_SIZE_LEVEL1)
    {
        return 0;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL2)
    {
        return 1;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL3)
    {
        return 2;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL4)
    {
        return 3;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL5)
    {
        return 4;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL6)
    {
        return 5;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL7)
    {
        return 6;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL8)
    {
        return 7;
    }
    else if (allocSize <= MEM_BLOCK_SIZE_LEVEL9)
    {
        return 8;
    }

    log_debug("ChooseSlotBlockIdByAllocSize allocSize is %u and can match slot.",
              allocSize);
    return INVALID_SLOT_INDEX;
}

void *GetMemPoolBySize(uint32_t allocSize)
{
    uint32_t slotId = ChooseSlotBlockIdByAllocSize(allocSize);
    if (slotId == INVALID_SLOT_INDEX)
    {
        return NULL;
    }
    return g_kvMemoryPoolManager->memoryMangerPool[slotId];
}

Status KVMemoryPoolInit()
{
    // DB_POINT(memoryPool);

    g_kvMemoryPoolManager =
        (KVMemoryPoolManagerT *)malloc(sizeof(KVMemoryPoolManagerT));
    if (g_kvMemoryPoolManager == NULL)
    {
        log_error(
            "malloc memoryPoolManager memory failed when exec KVMemoryPoolInit.");
        return GMERR_MEMORY_ALLOC_FAILED;
    }

    // 预计初始化36M内存资源池
    KVMemoryPoolT *memPtrArray =
        (KVMemoryPoolT *)malloc(MEM_PAGE_SIZE * MEM_POOL_SLOT_CNT);
    if (memPtrArray == NULL)
    {
        log_error("malloc memoryPool memory failed when exec KVMemoryPoolInit.");
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    memset(memPtrArray, 0, MEM_PAGE_SIZE);

    for (uint32_t i = 0; i < MEM_POOL_SLOT_CNT; i++)
    {
        memPtrArray[i].memPtr = memPtrArray + i;
        memPtrArray[i].currMemPtr = memPtrArray[i].memPtr;
        memPtrArray[i].memBlockSize = GetSlotBlockSizeByIdx(i);
        memPtrArray[i].freeBlockCnt = MEM_PAGE_SIZE / memPtrArray[i].memBlockSize;

        // 初始 block: 使用情况如下
        // 1 2 3 4 5 6 7 ...
        // 某一刻释放了 2 5 下次申请，从几开始呢？如何管理空闲内存块
        // 32 bytes中抽出4/8个字节，作为指针，将所有block串联起来，形成链表

        // 如果使用 *pCurList，我们只能存储一个 uint8_t
        // 类型的值，这不足以存储一个指针。 而使用 *(uint8_t
        // **)(pCurList)，我们可以存储一个指向 uint8_t
        // 类型的指针，从而正确地建立空闲内存块链表。 初始化空闲内存块链表
        uint8_t *pCurList = (uint8_t *)memPtrArray[i].memPtr;
        for (uint32_t j = 0; j < memPtrArray[i].freeBlockCnt - 1; j++)
        {
            *(uint8_t **)(pCurList) = pCurList + memPtrArray[i].memBlockSize;
            pCurList += memPtrArray[i].memBlockSize;
        }
        // 最后一个内存块的指针设置为NULL，表示链表结束
        *(uint8_t **)(pCurList) = NULL;
        log_debug("init memoryPool %d success. addr is %p", i,
                  memPtrArray[i].memPtr);
    }

    return GMERR_OK;
}

void KVMemoryPoolUninit()
{
    if (g_kvMemoryPoolManager == NULL)
    {
        return;
    }
    for (uint32_t i = 0; i < MEM_POOL_SLOT_CNT; i++)
    {
        free(g_kvMemoryPoolManager->memoryMangerPool[i]->memPtr);
    }
    free(g_kvMemoryPoolManager);
    g_kvMemoryPoolManager = NULL;
}


void *KVMalloc(uint32_t allocSize)
{
    void *ptr = malloc(allocSize);
    if (ptr == NULL)
    {
        log_error("malloc memory failed when exec KVMalloc.");
    }
    return ptr;
}

void *KVFree(void *ptr) { free(ptr); }

// 尝试从memoryPool中分配一块内存 分配失败则调用malloc
void *KVMemPoolAlloc(uint32_t allocSize)
{
    DB_POINT(g_kvMemoryPoolManager);
    KVMemoryPoolT *memoryPool = GetMemPoolBySize(allocSize);
    if (memoryPool == NULL)
    {
        log_debug("no memoryPool matched when exec KVMemAlloc. allocSize is %u.",
                  allocSize);
        return NULL;
    }

    if (memoryPool->freeBlockCnt == 0)
    {
        // TODO: 拓展page？
        log_warn("no free block when exec KVMemAlloc, allocSize is %u. pool "
                   "block size is %u.",
                   allocSize, memoryPool->memBlockSize);
        return NULL;
    }

    // 从空闲内存块链表中取出一个内存块
    void *ptr = memoryPool->currMemPtr;
    // 这里外部控制不好会踩存
    memoryPool->currMemPtr = *(uint8_t **)ptr;
    memoryPool->freeBlockCnt--;

    // 清空ptr
    memset(ptr, 0, memoryPool->memBlockSize);
    return ptr;
}

void KVMemPoolFree(void *ptr, uint32_t freeSize)
{
    DB_POINT(g_kvMemoryPoolManager);
    KVMemoryPoolT *memoryPool = GetMemPoolBySize(freeSize);
    if (memoryPool == NULL)
    {
        log_debug("no memoryPool matched when exec KVMemFree. freeSize is %u.",
                  freeSize);
        return;
    }
    // 将ptr指向的内存块插入空闲内存块链表
    *(uint8_t **)ptr = memoryPool->currMemPtr;
    memoryPool->currMemPtr = ptr;
    memoryPool->freeBlockCnt++;
}

void *KVMemAlloc(uint32_t allocSize)
{
    // 优先从内存池申请
    void *ptr = KVMemPoolAlloc(allocSize);
    if (ptr == NULL)
    {
        ptr = KVMalloc(allocSize);
    }
    return ptr;
}

void KVMemFree(void *ptr, uint32_t allocSize)
{
    if (ptr == NULL)
    {
        return;
    }
    // 释放内存池中的内存
    if (IsAllocByMemoryPool(ptr))
    {
        KVMemPoolFree(ptr, allocSize);
        return;
    }
    KVFree(ptr);
}