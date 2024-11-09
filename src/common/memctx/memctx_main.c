//
// Created by 94581 on 2024/11/9.
//

//#include <stdint.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "db_memctx.h"

/**
 *
 */

#define MEM_DEBUG 1


#define MEM_PAGE_SIZE 4096
// 初始的分配的页的数量  256 页
#define MEM_TOP_INIT_PAGE_COUNT 256

typedef struct DbMemPage {
    void *nextPageAddr; // 指向下一页
    uint32_t pageIdx; // 当前 page下标
//    uint32_t pageSize; // 每页大小 == initPageSize
    void *pageAddr; // 当前页的虚拟地址
} DbMemPageT;

// 暂时不考虑扩容情况
typedef struct DbMemCtx {
//    uint32_t totalAllocPage; // 从父节点中拿到的page数量
//    uint32_t totalUsedSize; // 当前memctx已使用的内存大小
    uint32_t freePageCnt; // 当前的空闲页数量
    DbMemPageT *freePageList; // 空闲页链表
} DbMemCtxT;

typedef struct DbMemCtxManager {
    uint32_t totalAllocSize; // 当前内存管理器向 OS 申请的总内存大小
    uint32_t initPageSize; // 初始每页大小
    uint32_t initPageCnt; // 初始最顶层有多少页

    DbMemCtxT *topMemCtx; // 最顶层的 memCtx
} DbMemCtxManagerT;

//DbMemCtxT g_topDynMemCtx = NULL;
DbMemCtxManagerT g_memCtxManager = NULL;

void *DbMalloc(uint32_t allocSize) {
    void *ptr = malloc(allocSize);
    if (ptr == NULL) {
        log_error("malloc memory failed when exec DbMalloc.");
    }
    return ptr;
}

void DbFree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    free(ptr);
}

void DbMemAddAllocSize(DbMemCtxManagerT *memCtxManager, uint32_t allocSize) {
    memCtxManager->totalAllocSize += allocSize;
}

/**
 * 初始化free 鏈表
 * @param freePageList
 * @param pageCnt
 */
void DbInitFreeList(DbMemPageT *freePageList, uint32_t pageCnt, void *pageAddr) {
    for (uint32_t i = 0; i < pageCnt; ++i) {
        DbMemPageT *freePage = &freePageList[i];
        freePage->pageIdx = i;
        freePage->pageAddr = (uint8_t *) pageAddr + i * MEM_PAGE_SIZE;
        freePage->nextPageAddr = i == pageCnt - 1 ? NULL : (uint8_t *) pageAddr + (i + 1) * MEM_PAGE_SIZE;
    }
}

/*
 * 初始化 g_dynMemCtx 内部調用接口
 */
Status DbInitTopMemCtx(DbMemCtxManagerT *memCtxManager) {
    uint32_t allocSize = sizeof(DbMemCtxT);
    DbMemCtxT *topMemCtx = (DbMemCtxT *) DbMalloc(allocSize)
    if (topMemCtx == NULL) {
//        DbFree(memCtxManager);
        log_error("malloc error when init topMemCtx. alloc size is %u.", allocSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    memset(topMemCtx, 0x00, allocSize);
    DbMemAddAllocSize(memCtxManager, allocSize);
    topMemCtx->freePageCnt = memCtxManager->initPageCnt;

    allocSize = memCtxManager->initPageCnt * sizeof(DbMemPageT);
    topMemCtx->freePageList = (DbMemPageT *) DbMalloc(allocSize);
    if (topMemCtx->freePageList == NULL) {
        DbFree(topMemCtx);
        log_error("malloc error when init freePageList. alloc size is %u.", allocSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    memset(topMemCtx->freePageList, 0x00, allocSize);
    DbMemAddAllocSize(memCtxManager, allocSize);

    // 申請實際頁
    allocSize = memCtxManager->initPageCnt * memCtxManager->initPageSize;
    void *pageAddr = DbMalloc(allocSize);
    if (pageAddr == NULL) {
        DbFree(topMemCtx->freePageList);
        DbFree(topMemCtx);
        log_error("malloc error when init pageAddr. alloc size is %u.", allocSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    memset(pageAddr, 0x00, allocSize);
    DbMemAddAllocSize(memCtxManager, allocSize);

    // 構造 鏈表順序
    DbInitFreeList(topMemCtx->freePageList, memCtxManager->initPageCnt, pageAddr);
    memCtxManager->topMemCtx = topMemCtx;
    return GMERR_OK;
}

/*
 * 服务器启动时初始化 g_memCtxManager
 */
Status DbInitMemManager() {
    uint32_t allocSize = (uint32_t)
    sizeof(DbMemCtxManagerT);
    DbMemCtxManagerT *memCtxManager = (DbMemCtxManagerT *) DbMalloc(allocSize);
    if (memCtxManager == NULL) {
        log_error("malloc error when DbInitMemManager. alloc size is %u.", allocSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    memset(memCtxManager, 0x00, allocSize);
    memCtxManager->totalAllocSize = allocSize;
    memCtxManager->initPageSize = MEM_PAGE_SIZE;
    memCtxManager->initPageCnt = MEM_TOP_INIT_PAGE_COUNT;
    Status ret = DbInitTopMemCtx(memCtxManager);
    if (ret != GMERR_OK) {
        DbFree(memCtxManager);
        return ret;
    }
    return GMERR_OK;
}


/*
 * 从传入的memCtx中申请内存
 */
void *DbDynMemCtxAlloc(DbMemCtxT *memCtx, uint32_t allocSize);







