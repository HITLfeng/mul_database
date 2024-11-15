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
#define MEM_CTX_NAME_LEN 64
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

#define PAGE_INVAILD_VALUE 0

typedef enum {
    PAGE_FIX_ALLOC = 0, // 固定分配每页大小 4/4/4/4/4 默认
    PAGE_RANDOM_ALLOC   // 随机分配每页大小 4/15/128/5
} DbMemPageTypeT;

typedef struct DbMemFixPageSplit {
    uint32_t pageSlotAllocSize; // 每个槽位大小,固定分配页需要设置此值 4/8/16/32  注意
    // **用户第一次申请的时候才会决定改值是多少！**
    uint32_t pageFreeSlotCnt;   // 当前的空闲slot个数
    uint32_t pageTotalSlotCnt;  // 当前页总的slot个数
    void *nextFreeAddrInPage;   // 指向下一个free位置 是一个链表 仅供固定分配页使用
    void *headAddr;             // 指向page起始位置
} DbMemFixPageSplitT;

typedef struct DbMemPage {
    void *nextPageAddr;      // 指向下一页
    uint32_t pageIdx;        // 当前 page下标
    uint32_t pageSize;       // 每页大小 == initPageSize
    void *pageAddr;          // 当前页的虚拟地址
    DbMemPageTypeT pageType; // 页的类型
    // 以下仅仅供 fixSize使用
    DbMemFixPageSplitT fixPage; // 固定长度页信息
    bool isPageInitByFixSize;   // 标识页是否已经被分割使用
} DbMemPageT;

// 暂时不考虑扩容情况
struct DbMemCtx {
    //    uint32_t totalAllocPage; // 从父节点中拿到的page数量
    //    uint32_t totalUsedSize; // 当前memctx已使用的内存大小
    char memCtxName[MEM_CTX_NAME_LEN];
    uint32_t totalPageCnt; // 当前memCtx及所有子节点共占有的page数量
    uint32_t freePageCnt;                             // 当前的空闲页数量
    DbMemPageT *freePageList;                         // 空闲页链表
    DbMemCtxT *parentMemCtx;                          // 指向父节点的memCtx
    DbMemCtxT *childMemCtx[MEM_MAX_CHILD_NUM];        // 指向孩子节点的memCtx
    uint32_t childNum;                                // 该层直属的孩子节点个数
    void *bigMemAllocList[MEM_BIG_ALLOC_MAX_COUNT];   // 大块内存申请链表
    uint32_t bigMemAllocCnt;                          // 大块内存申请数量
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

DbMemCtxT *DbGetTopMemCtx() { return g_memCtxManager->topMemCtx; }

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
        freePage->pageSize = MEM_PAGE_SIZE;
        // freePage->nextFreeAddrInPage = pageAddr;
        // freePage->pageSlotAllocSize = 0;
        freePage->pageType = PAGE_FIX_ALLOC;
        freePage->fixPage = (DbMemFixPageSplitT) {0};
    }
}

// ************************************************************************************
// ********************   trace *******************************************************
// ************************************************************************************

void DbMemCtxMgrTrace(DbMemCtxManagerT *memCtxManager) {
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

/**
 * tools
 * @param memCtxManager
 * @return
 */
void DbMemFreeListPushFront(DbMemCtxT *memCtx, DbMemPageT *page) {
    page->nextPageAddr = memCtx->freePageList;
    memCtx->freePageList = page;
}
/**
 * tools
 * @param memCtxManager
 * @return
 */

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
    const char *topName = "top_dyn_memCtx";
    memcpy(topMemCtx->memCtxName, topName, STRLEN(topName));
    DbMemAddAllocSize(memCtxManager, allocSize);
    topMemCtx->freePageCnt = memCtxManager->initPageCnt;
    topMemCtx->totalPageCnt = memCtxManager->initPageCnt;
    topMemCtx->childNum = 0;

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
    uint32_t
    allocSize = (uint32_t)
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
    DbMemCtxMgrTrace(memCtxManager);
    g_memCtxManager = memCtxManager;
    return GMERR_OK;
}

bool DbIsTopMemCtx(DbMemCtxT *memCtx) { return memCtx->parentMemCtx == NULL; }

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
bool DbHasPageInSlot(DbMemCtxT *memCtx, uint32_t level) { return memCtx->fixSizeLevelList[level] != NULL; }

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
        // 借成功了,当前节点的总申请书 +1
        parentMemCtx->totalPageCnt++;
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
 * 清空页,包括管理结构、慎用
 * @param page
 */
void DbMemResetPage(DbMemPageT *page) { memset(page, 0x00, MEM_PAGE_SIZE); }

void DbMemInitAndSplitPage(DbMemPageT *page, uint32_t levelIdx) {
    page->isPageInitByFixSize = true;
    page->fixPage.pageSlotAllocSize = DbPageGetSizeByLevel(levelIdx);
    page->fixPage.pageTotalSlotCnt = MEM_PAGE_SIZE / page->fixPage.pageSlotAllocSize;
    page->fixPage.pageFreeSlotCnt = page->fixPage.pageTotalSlotCnt;
    page->fixPage.headAddr = page->pageAddr;
    page->fixPage.nextFreeAddrInPage = page->fixPage.headAddr;
    // 开始切分
    DbMemResetPage(page->pageAddr);
    uint8_t *pCurPos = (uint8_t *) page->fixPage.headAddr;
    for (uint32_t i = 0; i < page->fixPage.pageTotalSlotCnt - 1; i++) {
        *(uint8_t * *)(pCurPos) = pCurPos + page->fixPage.pageSlotAllocSize;
        pCurPos += page->fixPage.pageSlotAllocSize;
    }
    // 最后一个内存块的指针设置为NULL，表示链表结束
    *(uint8_t * *)(pCurPos) = NULL;
}

void *DbMemPageFindSlotWithLevelList(DbMemPageT *levelPageList) {
    if (levelPageList == NULL) {
        return NULL;
    }
    void *slotAddr = NULL;
    if (levelPageList->fixPage.pageFreeSlotCnt > 0) {
        slotAddr = levelPageList->fixPage.nextFreeAddrInPage;
        levelPageList->fixPage.nextFreeAddrInPage = *(uint8_t **) slotAddr;
        levelPageList->fixPage.pageFreeSlotCnt--;
        log_trace("find slot in level page list. and page idx is %u. curr free cnt in this page is %u.",
                  levelPageList->pageIdx, levelPageList->fixPage.pageFreeSlotCnt);
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
    if (memCtx == NULL) {
        memCtx = DbGetTopMemCtx();
    }
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
        log_trace("alloc big size is %u, curr alloc cnt is %u.", allocSize, memCtx->bigMemAllocCnt);
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
        log_trace("first alloc %u level page, level limit is %u.", levelIdx, DbPageGetSizeByLevel(levelIdx));
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
        log_trace("Page is full. alloc %u level page, level limit is %u.", levelIdx, DbPageGetSizeByLevel(levelIdx));
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

bool DbIsPtrAllocInPage(void *pageAddr, void *ptr) {
    return (uint8_t *) ptr >= (uint8_t *) pageAddr && (uint8_t *) ptr < (uint8_t *) pageAddr + MEM_PAGE_SIZE;
}

// 检测 地址 void *ptr 是否申请自 pageList 是返回 page 否返回NULL
DbMemPageT *DbGetPageByPtrInPageList(DbMemPageT *pageList, void *ptr) {
    if (pageList == NULL) {
        return NULL;
    }
    DbMemPageT *currPage = pageList;
    void *pageAddr = currPage->pageAddr;
    if (DbIsPtrAllocInPage(pageAddr, ptr)) {
        return currPage;
    }
    return DbGetPageByPtrInPageList(currPage->nextPageAddr, ptr);
}

void DbDynFreeInPage(DbMemPageT *page, void *ptr) {
    *(uint8_t **) ptr = page->fixPage.nextFreeAddrInPage;
    page->fixPage.nextFreeAddrInPage = ptr;
    page->fixPage.pageFreeSlotCnt++;
}

bool IsPageEqual(DbMemPageT *page1, DbMemPageT *page2) {
    return page1 == page2;
}

void DbMemResetPage(DbMemPageT *page) {
    page->nextPageAddr = NULL;
    page->isPageInitByFixSize = false;
    page->fixPage.pageSlotAllocSize = PAGE_INVAILD_VALUE;
    page->fixPage.pageFreeSlotCnt = PAGE_INVAILD_VALUE;
    page->fixPage.pageTotalSlotCnt = PAGE_INVAILD_VALUE;
    page->fixPage.nextFreeAddrInPage = NULL;
    page->fixPage.headAddr = NULL;
}

// 回收空的页 去对应的槽位寻找
void DbDynMemCtxRecycle(DbMemCtxT *memCtx, DbMemPageT *page, uint32_t slotId) {
    if (page->fixPage.pageFreeSlotCnt < page->fixPage.pageTotalSlotCnt) {
        return;
    }
    // 这里释放这页，然后重新挂会当前memCtx的freeList上}
    // TODO:
    DbMemPageT *slotPageList = memCtx->fixSizeLevelList[slotId];
    DbMemPageT *prevPage = NULL;
    DbMemPageT *currPage = slotPageList;
    while (currPage != NULL) {
        if (IsPageEqual(currPage, page)) {
            // RESET page
            if (prevPage == NULL) {
                // first in
                memCtx->fixSizeLevelList[slotId] = NULL; // clear to empty
            } else {
                prevPage->nextPageAddr = currPage->nextPageAddr;
            }
            // reset page
            DbMemResetPage(page);
            // push front memCtx freeList
            DbMemFreeListPushFront(page);
            log_trace("page %u has recycle", slotId);
            break;
        }
        prevPage = currPage;
        currPage = currPage->nextPageAddr;
    }
    log_warn("page %u recycle unsucc.", slotId);
}

/**
 * 释放内存
 */
void DbDynMemCtxFree(DbMemCtxT *memCtx, void *ptr) {
    if (memCtx == NULL) {
        memCtx = DbGetTopMemCtx();
    }
    // 效率低下！
    for (uint32_t i = 0; i < MEM_FIX_SIZE_LEVEL; ++i) {
        if (memCtx->fixSizeLevelList[i] == NULL) {
            continue;
        }
        // 检查能否找到这个地址
        DbMemPageT *currPageList = memCtx->fixSizeLevelList[i];
        DbMemPageT *targetPage = DbGetPageByPtrInPageList(currPageList, ptr);
        if (targetPage == NULL) {
            continue;
        }
        // 找到了
        DbDynFreeInPage(targetPage, ptr);
        log_trace("ptr %p release success, page idx is %u.", ptr, targetPage->pageIdx);
        // 尝试回收该页
        DbDynMemCtxRecycle(memCtx, targetPage, i);
        return;
    }
    // 没找到 遍历大对象列表
    for (uint32_t i = 0; i < memCtx->bigMemAllocCnt; ++i) {
        // TODO: 确定两个 void * 能不能直接比较
        if ((uint8_t * )(memCtx->bigMemAllocList[i]) != (uint8_t *) ptr) {
            continue;
        }
        // 找到
        DbFree(ptr);
        for (uint32_t j = i + 1; j < memCtx->bigMemAllocCnt; ++j) {
            memCtx->bigMemAllocList[j - 1] = memCtx->bigMemAllocList[j];
        }
        memCtx->bigMemAllocCnt--;
        log_trace("ptr %p release success, big alloc idx is %u.", ptr, i);
        return;
    }
    log_error("memctx dont match this ptr!");
    DB_ASSERT(false);
}

// fix size 分级:
// uint32_t fixSizeLevel[9] = {4,8,16,32,64,128,256,512,1024};

Status DbMemCtxCreateCheck(DbMemCtxT *memCtx, const char *name) {
    if (memCtx->childNum >= MEM_MAX_CHILD_NUM) {
        log_error("child has up to limit %u.", MEM_MAX_CHILD_NUM);
        return GMERR_MEMCTX_CREATED_NEW_FAILED;
    }
    if (STRLEN(name) > MEM_CTX_NAME_LEN) {
        log_error("new memctx name %s is long than %u.", name, MEM_CTX_NAME_LEN);
        return GMERR_MEMCTX_CREATED_NAME_TOO_LONG;
    }
    return GMERR_OK;
}

/**
 * 从父节点申请一个新的memCtx,并马上分配 1 个页下去
 * @param memCtx
 * @param childMemCtx
 * @return
 */
Status DbCreateMemCtx(DbMemCtxT *memCtx, const char *name, DbMemCtxT **childMemCtx) {
    DB_POINT(childMemCtx);
    if (memCtx == NULL) {
        memCtx = DbGetTopMemCtx();
    }
    Status ret = GMERR_OK;

    ret = DbMemCtxCreateCheck(memCtx, name);
    if (ret != GMERR_OK) {
        return ret;
    }
    // 申请一块新的memCtx
    DbMemCtxT *newMemCtx = DbDynMemCtxAlloc(memCtx, sizeof(DbMemCtxT));
    if (newMemCtx == NULL) {
        log_error("alloc new memctx failed when exec DbCreateMemCtx and alloc size is %u.", sizeof(DbMemCtxT));
        return GMERR_MEMCTX_DYN_ALLOC_FAILED;
    }
    // 注意：此接口 DbDynMemCtxAlloc 内部已经memset过 只需要赋值父节点即可
    newMemCtx->parentMemCtx = memCtx;

    // 父节点分配一个页
    ret = DbGetFreePageFromParentMemCtx(memCtx, newMemCtx);
    if (ret != GMERR_OK) {
        DbDynMemCtxFree(memCtx, newMemCtx);
        return ret;
    }
    memcpy(newMemCtx->memCtxName, name, STRLEN(name));

    memCtx->childMemCtx[memCtx->childNum] = newMemCtx;
    memCtx->childNum++;
    *childMemCtx = newMemCtx;
    return GMERR_OK;
}




// 20241114 TODO:
/*
 * 1. 拆分文件 trace util mainfunc
 * 2. 考虑重构页 不从freelist中剔除，而是标记删除
 * 3. DbMemCtxReset 和 DbMemCtxDelete 尽快完善 闭环 memCtx 特性 SR！
 */

// 该函数不能修改 currPage->nextPageAddr
typedef (void)(*HandlerPage)(
DbMemPageT *page
);

void DbMemCtxDealPageList(DbMemPageT *pageList, HandlerPage handler) {
    DbMemPageT *currPage = pageList;
    while (currPage != NULL) {
        handler(currPage);
        currPage = currPage->nextPageAddr;
    }
}

void ResetSinglePage(DbMemPageT *page) {
    DB_POINT(page);
    page->isPageInitByFixSize = false;
    page->fixPage.headAddr = NULL;
    page->fixPage.nextFreeAddrInPage = NULL;
    page->fixPage.pageFreeSlotCnt = PAGE_INVAILD_VALUE;
    page->fixPage.pageTotalSlotCnt = PAGE_INVAILD_VALUE;
    page->fixPage.pageSlotAllocSize = PAGE_INVAILD_VALUE;
}

uint32_t GetPageListLength(DbMemPageT *pageList) {
    if (pageList == NULL) {
        return 0;
    }
    uint32_t length = 0;
    DbMemPageT *currPage = pageList;
    while (currPage != NULL) {
        currPage = currPage->nextPageAddr;
        ++length;
    }
    return length;
}

void PageListHeadInsert(DbMemPageT *pageList, DbMemCtxT *memCtx) {
    DB_POINT(pageList);
    memCtx->freePageCnt += GetPageListLength(pageList);
    // find last page
    DbMemPageT *currPage = pageList;
    while (currPage->nextPageAddr != NULL) {
        currPage = currPage->nextPageAddr;
    }
    currPage->nextPageAddr = memCtx->freePageList;
    memCtx->freePageList = pageList;
}


/**
 * 将当前memCtx内申请的内存全部释放，不会影响子节点！
 * @param memCtx 要reset的memCtx
 */
void DbMemCtxReset(DbMemCtxT *memCtx) {
    // 遍历 每个槽位
    for (uint32_t i = 0; i < MEM_FIX_SIZE_LEVEL; ++i) {
        DbMemPageT *pageList = memCtx->fixSizeLevelList[i];
        if (pageList == NULL) {
            continue;
        }

        // 单个槽位的页、全部释放 reset page
        DbMemCtxDealPageList(pageList, ResetSinglePage);
        // 将当前页挂载到freeList
        PageListHeadInsert(pageList, memCtx);
    }
    // 遍历所有大对象
    for (uint32_t i = 0; i < memCtx->bigMemAllocCnt; ++i) {
        DbFree(memCtx->bigMemAllocList[i]);
    }
    memCtx->bigMemAllocCnt = 0;
}


void DbMemCtxDeleteInner(DbMemCtxT *memCtx) {
    for (uint32_t i = 0; i < memCtx->childNum; ++i) {
        DbMemCtxT *childMemCtx = memCtx->childMemCtx[i];
        DbMemCtxDeleteInner(memCtx);
    }
    // find leaf memctx node
    DbMemCtxReset(memCtx);
    uint32_t freePageCnt = GetPageListLength(memCtx);
    DB_ASSERT(freePageCnt == memCtx->freePageCnt);
    // 将全部页上交给父节点
    DbMemCtxT *parentMemCtx = memCtx->parentMemCtx;
    PageListHeadInsert(memCtx->freePageList, parentMemCtx);
    DbDynMemCtxFree(parentMemCtx, memCtx);
}

/**
 * 一把级联删除当前memCtx以及所有的子节点 暂不支持对顶层memCtx进行此操作！
 * @param memCtx
 */
Status DbMemCtxDelete(DbMemCtxT *memCtx) {
    // 复杂
    if (DbIsTopMemCtx(memCtx)) {
        log_error("top memCtx is not allowed to delete.");
        return GMERR_MEMCTX_OPERATOR_ILLEGAL;
    }
    DbMemCtxT *parentMemCtx = memCtx->parentMemCtx;
    uint32_t i = 0;
    for (; i < parentMemCtx->childNum; ++i) {
        if (parentMemCtx->childMemCtx[i] == memCtx) {
            // find
            break;
        }
    }
    if (i < parentMemCtx->childNum) {
        for (uint32_t j = i; j < parentMemCtx->childNum - 1; ++j) {
            parentMemCtx->childMemCtx[j] = parentMemCtx->childMemCtx[j + 1];
        }
        DbMemCtxDeleteInner(memCtx);
    } else {
        log_error("delete failed and curr memctx is not a invaild tree.");
        return GMERR_MEMCTX_TREE_INVAILD;
    }
    return GMERR_OK;
}