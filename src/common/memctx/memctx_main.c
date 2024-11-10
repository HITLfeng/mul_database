//
// Created by 94581 on 2024/11/9.
//

// #include <stdint.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "db_memctx.h"

/**
 *
 */

#define MEM_DEBUG 0

#define MEM_PAGE_SIZE 4096
// 初始的分配的页的数量  256 页
#define MEM_TOP_INIT_PAGE_COUNT 256
// 每个memCtx最多创建的子memCtx个数
#define MEM_MAX_CHILD_NUM 16
// 超过1024的暂时不使用页申请
#define MEM_BIG_ALLOC_SIZE 1024
// 一个memCtx上最多申请64个超大内存
#define MEM_BIG_ALLOC_MAX_COUNT 64
#define MEM_FIX_SIZE_LEVEL 8
#define MEM_INVAILD_LEVEL_INDEX 0xffffffff

typedef enum {
    PAGE_FIX_ALLOC = 0, // 固定分配每页大小 4/4/4/4/4 默认
    PAGE_RANDOM_ALLOC // 随机分配每页大小 4/15/128/5
} DbMemPageTypeT;

typedef struct DbMemFixPageSplit {
    uint32_t pageSlotAllocSize; // 每个槽位大小,固定分配页需要设置此值 4/8/16/32  注意 **用户第一次申请的时候才会决定改值是多少！**
    uint32_t pageFreeSlotCnt; // 当前的空闲slot个数
    uint32_t pageTotalSlotCnt; // 当前页总的slot个数
    void *nextFreeAddrInPage; // 指向下一个free位置 是一个链表 仅供固定分配页使用
    void *headAddr; // 指向page起始位置
} DbMemFixPageSplitT;

typedef struct DbMemPage {
    void *nextPageAddr; // 指向下一页
    uint32_t pageIdx; // 当前 page下标
//    uint32_t pageSize; // 每页大小 == initPageSize
    void *pageAddr; // 当前页的虚拟地址
    DbMemPageTypeT pageType; // 页的类型
    // 以下仅仅供 fixSize使用
    DbMemFixPageSplitT fixPage; // 固定长度页信息
    bool isPageInitByFixSize; // 标识页是否已经被分割使用
} DbMemPageT;

typedef struct DbMemCtx DbMemCtxT;

// 暂时不考虑扩容情况
struct DbMemCtx {
//    uint32_t totalAllocPage; // 从父节点中拿到的page数量
//    uint32_t totalUsedSize; // 当前memctx已使用的内存大小
    uint32_t freePageCnt; // 当前的空闲页数量
    DbMemPageT *freePageList; // 空闲页链表
    DbMemCtxT *parentMemCtx; // 指向父节点的memCtx
    DbMemCtxT *childMemCtx[MEM_MAX_CHILD_NUM]; // 指向孩子节点的memCtx
    uint32_t childNum; // 该层直属的孩子节点个数
    void *bigMemAllocList[MEM_BIG_ALLOC_MAX_COUNT]; // 大块内存申请链表
    uint32_t bigMemAllocCnt; // 大块内存申请数量
    DbMemPageT *fixSizeLevelList[MEM_FIX_SIZE_LEVEL]; // 层级 4 8 16 32 64 128 256 512 1024
};

typedef struct DbMemCtxManager {
    uint32_t totalAllocSize; // 当前内存管理器向 OS 申请的总内存大小
    uint32_t initPageSize;   // 初始每页大小
    uint32_t initPageCnt;    // 初始最顶层有多少页

    DbMemCtxT *topMemCtx; // 最顶层的 memCtx
} DbMemCtxManagerT;



// DbMemCtxT g_topDynMemCtx = NULL;
DbMemCtxManagerT *g_memCtxManager = NULL;

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
        freePage->nextPageAddr = i == pageCnt - 1 ? NULL : (uint8_t *) freePage + sizeof(DbMemPageT);
        // freePage->nextFreeAddrInPage = pageAddr;
        // freePage->pageSlotAllocSize = 0;
        freePage->pageType = PAGE_FIX_ALLOC;
        freePage->fixPage = (DbMemFixPageSplitT){0};
    }
}

// ************************************************************************************
// ********************   trace *******************************************************
// ************************************************************************************


void DbMemCtxMgrTrace(DbMemCtxManagerT *memCtxManager)
{
    if (!MEM_DEBUG) {
        return;
    }
    log_trace("memCtxManager->totalAllocSize = %u.", memCtxManager->totalAllocSize);
    log_trace("memCtxManager->initPageSize = %u.", memCtxManager->initPageSize);
    log_trace("memCtxManager->initPageCnt = %u.", memCtxManager->initPageCnt);
    log_trace("memCtxManager->topMemCtx = %p.", memCtxManager->topMemCtx);
    DbMemCtxT *memCtx = memCtxManager->topMemCtx;
    log_trace("memCtx->freePageCnt = %u.", memCtx->freePageCnt);
    DbMemPageT *freePage = memCtx->freePageList;
    while (freePage != NULL) {
        log_trace("freePage->pageIdx = %u.", freePage->pageIdx);
        log_trace("    freePage->pageAddr = %p.", freePage->pageAddr);
        log_trace("    freePage->nextPageAddr = %p.", freePage->nextPageAddr);
        freePage = freePage->nextPageAddr;
    }
}

/*
 * 初始化 g_dynMemCtx 内部調用接口
 */
Status DbInitTopMemCtx(DbMemCtxManagerT *memCtxManager) {
    uint32_t allocSize = sizeof(DbMemCtxT);
    DbMemCtxT *topMemCtx = (DbMemCtxT *) DbMalloc(allocSize);
    if (topMemCtx == NULL) {
        //        DbFree(memCtxManager);
        log_error("malloc error when init topMemCtx. alloc size is %u.", allocSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    memset(topMemCtx, 0x00, allocSize);
    DbMemAddAllocSize(memCtxManager, allocSize);
    topMemCtx->freePageCnt = memCtxManager->initPageCnt;
    topMemCtx->childNum = 0;

    allocSize = memCtxManager->initPageCnt * sizeof(DbMemPageT);
    topMemCtx->freePageList = (DbMemPageT *)DbMalloc(allocSize);
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
    uint32_t allocSize = (uint32_t)sizeof(DbMemCtxManagerT);
    DbMemCtxManagerT *memCtxManager = (DbMemCtxManagerT *)DbMalloc(allocSize);
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
    DbMemCtxMgrTrace(memCtxManager);
    return GMERR_OK;
}

bool DbIsTopMemCtx(DbMemCtxT *memCtx) {
    return memCtx->parentMemCtx == NULL;
}

/**
 *
 * @param memCtx
 * @param getPageMemCtx 要获取页的memCtx
 * @return
 */
Status DbGetFreePageFromParent(DbMemCtxT *memCtx, DbMemCtxT *getPageMemCtx) {
    if (DbIsTopMemCtx(memCtx)) {
        // TODO: 根节点的话,向OS申请新的页
        log_error("top memCtx page is not enough to create a new memCtx.");
        return GMERR_MEMCTX_ERROR_NO_PAGE_ALLOC;
    }
    DbMemCtxT *parentMemCtx = memCtx->parentMemCtx;
    if (parentMemCtx->freePageCnt == 0) {
        // 父节点也没有空闲页,继续向上找
        Status ret = DbGetFreePageFromParent(parentMemCtx, getPageMemCtx);
        if (ret != GMERR_OK) {
            return ret;
        }
    }
    // 拿到父节点的这个page
    DbMemPageT *freePage = &parentMemCtx->freePageList[0];
    parentMemCtx->freePageList = freePage->nextPageAddr;
    parentMemCtx->freePageCnt--;
    // 头插到孩子memCtx中
    DbMemPageT *childOldFreePageList = getPageMemCtx->freePageList;
    getPageMemCtx->freePageList = freePage;
    freePage->nextPageAddr = childOldFreePageList;
    getPageMemCtx->freePageCnt++;

    return GMERR_OK;
}

/**
 * 从父节点申请一个新的memCtx,并马上分配 1 个页下去
 * @param memCtx
 * @param childMemCtx
 * @return
 */
// Status DbCreateMemCtx(DbMemCtxT *memCtx, DbMemCtxT **childMemCtx) {
//     DB_POINT2(memCtx, childMemCtx);
//     Status ret = GMERR_OK;
//     if (memCtx->freePageCnt == 0) {
//         ret = DbGetFreePageFromParent();
//         // 找父节点要

//     }

// }

uint32_t DbGetFixSizeLevel(uint32_t allocSize) {
    DB_ASSERT(allocSize > 0 && allocSize <= MEM_BIG_ALLOC_SIZE);
    uint32_t fixSizeLevel[MEM_FIX_SIZE_LEVEL] = {8, 16, 32, 64, 128, 256, 512, 1024};
    for (uint32_t i = 0; i < MEM_FIX_SIZE_LEVEL; ++i) {
        if (allocSize <= fixSizeLevel[i]) {
            return i;
        }
    }
    // 不应该走到这里
    DB_ASSERT(false);
    return MEM_INVAILD_LEVEL_INDEX;
}

uint32_t DbPageGetSizeByLevel(uint32_t level) {
    DB_ASSERT(level >= 0 && level < MEM_FIX_SIZE_LEVEL);
    uint32_t fixSizeLevel[MEM_FIX_SIZE_LEVEL] = {8, 16, 32, 64, 128, 256, 512, 1024};
    return fixSizeLevel[level];
}

/*
 * 判断对应槽位是否已经挂载了page
 * true 是
 */
bool DbHasPageInSlot(DbMemCtxT *memCtx, uint32_t level) {
    return memCtx->fixSizeLevelList[level] != NULL;
}

/**
 * 从当前memCtx中申请一页出来,没有的话会自动像父节点借
 * @param memCtx 要借的memCtx
 * @return
 */
Status DbGetFreePageFromParentMemCtx(DbMemCtxT *parentMemCtx, DbMemCtxT *memCtx) {
    Status ret = GMERR_OK;
    if (parentMemCtx == NULL) {
        log_error("top memCtx page is not enough to alloc a new page. parentMemCtx == NULL.");
        return GMERR_MEMCTX_ERROR_NO_PAGE_ALLOC;
    }
    // 判断当前memCtx能不能借
    if (parentMemCtx->freePageCnt == 0) {
        // 继续向父节点借
        ret = DbGetFreePageFromParentMemCtx(parentMemCtx->parentMemCtx, memCtx);
        if (ret != GMERR_OK) {
            return ret;
        }
    } else {
        // 拿到父节点的这个page
        DbMemPageT *freePage = &parentMemCtx->freePageList[0];
        parentMemCtx->freePageList = freePage->nextPageAddr;
        parentMemCtx->freePageCnt--;
        // 头插到孩子memCtx中
        DbMemPageT *childOldFreePageList = memCtx->freePageList;
        memCtx->freePageList = freePage;
        freePage->nextPageAddr = childOldFreePageList;
        memCtx->freePageCnt++;
    }
    return GMERR_OK;
}

Status DbAllocPageFromCurrMemCtxInner(DbMemCtxT *memCtx) {
    Status ret = GMERR_OK;
    if (memCtx->freePageCnt == 0) {
        ret = DbGetFreePageFromParentMemCtx(memCtx->parentMemCtx, memCtx);
        if (ret != GMERR_OK) {
            return ret;
        }
    }
    return GMERR_OK;
}

Status DbAllocPageFromCurrMemCtx(DbMemCtxT *memCtx, DbMemPageT **page) {
    Status ret = GMERR_OK;
    if (memCtx->freePageCnt == 0) {
        ret = DbAllocPageFromCurrMemCtxInner(memCtx);
        if (ret != GMERR_OK) {
            return ret;
        }
    }
    // 从 free list 中拿一页出来！
    DbMemPageT *freePage = &memCtx->freePageList[0];
    memCtx->freePageList = freePage->nextPageAddr;
    memCtx->freePageCnt--;
    freePage->nextPageAddr = NULL;
    *page = freePage;
    return GMERR_OK;
}

/**
 * 清空页
 * @param page
 */
void DbMemResetPage(DbMemPageT *page) {
    memset(page, 0x00, MEM_PAGE_SIZE);
}

void DbMemInitAndSplitPage(DbMemPageT *page, uint32_t levelIdx) {
    page->isPageInitByFixSize = true;
    page->fixPage.pageSlotAllocSize = DbPageGetSizeByLevel(levelIdx);
    page->fixPage.pageTotalSlotCnt = MEM_PAGE_SIZE / page->fixPage.pageSlotAllocSize;
    page->fixPage.pageFreeSlotCnt = page->fixPage.pageTotalSlotCnt;
    page->fixPage.headAddr = page->pageAddr;
    page->fixPage.nextFreeAddrInPage = page->fixPage.headAddr;
    // 开始切分
    DbMemResetPage(page);
    uint8_t *pCurPos = (uint8_t *) page->fixPage.headAddr;
    for (uint32_t i = 0; i < page->fixPage.pageTotalSlotCnt - 1; i++) {
        *(uint8_t * *)(pCurPos) = pCurPos + page->fixPage.pageSlotAllocSize;
        pCurPos += page->fixPage.pageSlotAllocSize;
    }
    // 最后一个内存块的指针设置为NULL，表示链表结束
    *(uint8_t * *)(pCurPos) = NULL;
}

void *DbMemPageFindSlotWithLevelList(DbMemPageT *levelPageList) {
    if (levelPageList->nextPageAddr == NULL) {
        return NULL;
    }
    void *slotAddr = NULL;
    if (levelPageList->fixPage.pageFreeSlotCnt > 0) {
        slotAddr = levelPageList->fixPage.nextFreeAddrInPage;
        levelPageList->fixPage.nextFreeAddrInPage = *(uint8_t **) slotAddr;
        levelPageList->fixPage.pageFreeSlotCnt--;
        // 默认初始化为 0
        memset(slotAddr, 0x00, levelPageList->fixPage.pageSlotAllocSize);
        return slotAddr;
    }
    return DbMemPageFindSlotWithLevelList(levelPageList->nextPageAddr);
}

/*
 * 获取新page页，挂载到对应位置，并完成分片
 */
Status DbMemGetAndSplitPage(DbMemCtxT *memCtx, uint32_t levelIdx) {
    // 如果对应位置没有page 申请一页 然后进行分割
    DbMemPageT *page = NULL;
    Status ret = DbAllocPageFromCurrMemCtx(memCtx, &page);
    if (ret != GMERR_OK) {
        return ret;
    }
    DbMemInitAndSplitPage(page, levelIdx);
    // 挂载到对应位置
    DbMemPageT *oldPage = memCtx->fixSizeLevelList[levelIdx];
    memCtx->fixSizeLevelList[levelIdx] = page;
    page->nextPageAddr = oldPage;
    return GMERR_OK;
}

/*
 * 从传入的memCtx中申请内存
 */
void *DbDynMemCtxAlloc(DbMemCtxT *memCtx, uint32_t allocSize) {
    Status ret = GMERR_OK;
    if (allocSize > MEM_BIG_ALLOC_SIZE) {
        void *pMem = DbMalloc(allocSize);
        if (pMem == NULL) {
            ret = GMERR_MEMORY_ALLOC_FAILED;
            log_error("malloc error when DbDynMemCtxAlloc. alloc size is %u. ret is %u.", allocSize, ret);
            return NULL;
        }
        memset(pMem, 0x00, allocSize);
        memCtx->bigMemAllocList[memCtx->bigMemAllocCnt] = pMem;
        memCtx->bigMemAllocCnt++;
        return pMem;
    }
    // else
    // 1. 判断当前allocSize对应的长度的页是否以及创建
    // 创建 且非满 直接分配内存
    // 创建 且满 使用新的页 初始化为fix size 然后进行分配，同时要将新页挂在同一款fix size 页之下 4->4 16 32->32->32
    uint32_t levelIdx = DbGetFixSizeLevel(allocSize);
    if (levelIdx == MEM_INVAILD_LEVEL_INDEX) {
        ret = GMERR_MEMCTX_ERROR_INVAILD_LEVEL_INDEX;
        log_error("levelIdx %u is invaild. ret is %u.", levelIdx, ret);
        return NULL;
    }

    // addr地址
    void *slotAddr = NULL;
    if (!DbHasPageInSlot(memCtx, levelIdx)) {
        ret = DbMemGetAndSplitPage(memCtx, levelIdx);
        if (ret != GMERR_OK) {
            return NULL;
        }
        // 返回回去地址哈
    } else {
        // 遍历当前所有页 看有没有空余位置原来分配出去
        // 获取当前链表
        DbMemPageT *levelPageList = memCtx->fixSizeLevelList[levelIdx];
        slotAddr = DbMemPageFindSlotWithLevelList(levelPageList);
        if (slotAddr != NULL) {
            return slotAddr;
        }
        // 说明当前页已满继续申请新页
        ret = DbMemGetAndSplitPage(memCtx, levelIdx);
        if (ret != GMERR_OK) {
            return NULL;
        }
    }
    // 走到这里直接返回最新的一页就可以
    slotAddr = DbMemPageFindSlotWithLevelList(memCtx->fixSizeLevelList[levelIdx]);
    if (slotAddr == NULL) {
        // 逻辑上这里不会为NULL
        ret = GMERR_MEMCTX_ERROR_UNEXCEPT_NULL;
        log_error("slotAddr is NULL. ret is %u.", ret);
        DB_ASSERT(false);
        return NULL;
    }
    return slotAddr;
}

//fix size 分级:
//uint32_t fixSizeLevel[9] = {4,8,16,32,64,128,256,512,1024};






