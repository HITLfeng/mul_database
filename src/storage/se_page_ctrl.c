#include "se_common.h"

// se层page使用页管理 一页 4096 bytes
#define SE_SINGLE_PAGE_SIZE 4096

// se层初始申请的页的个数 可动态扩容，不会缩容
#define SE_INIT_PAGE_COUNT 5

#define SE_INIT_PAGE_POOL_SIZE 360
#define SE_PAGE_SIZE 4096

// next addr + pre addr + pageId + slotId + isDelete(标记删除)
#define SE_PAGE_ROW_EXTRA_SIZE (sizeof(void *) + sizeof(void *) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t))

// typedef struct SePageCtrl {
//     uint32_t pageCnt; // 当前的页数
//     uint32_t pageSize; // 页大小
//     uint32_t pageUsed; // 已使用页数
//     uint32_t pageFree; // 未使用页数

// } SePageCtrlT;

// 页管理
// typedef struct PageCtrl {
//     uint32_t pageCnt; // 页数
//     uint32_t pageSize; // 页大小
//     uint32_t pageUsed; // 已使用页数
//     uint32_t pageFree; // 未使用页数
//     uint32_t pageBegin; // 页起始位置
//     uint32_t pageEnd; // 页结束位置
//     uint32_t pageCurr; // 当前页位置
//     uint32_t rowCnt; // 记录数
//     uint32_t rowSize; // 一行记录长度
//     uint32_t rowUsed; // 已使用记录数
//     uint32_t rowFree; // 未使用记录数
//     uint32_t rowBegin; // 记录起始位置
//     uint32_t rowEnd; // 记录结束位置
//     uint32_t rowCurr; // 当前记录位置
// } PageCtrlT;

// typedef struct HeapAddr {
//     uint32_t pageId;
//     uint32_t slotId;
// } HeapAddrT;

/**
 * README.md
 *
 *
 * SE 模块设计思路
 * 1. 申请 360 块 内存页 作为初始页 暂时不支持拓展 后续要拓展也是按照 360 为单位进行新增
 * 2. 每页有两种状态 used | free
 * 3. 每张 label 上关联的 container 会申请页 申请到的页 使用 page 指针串联起来 | 同时记录pageId 使用 map 存储
 * 每个空闲页！
 */

typedef struct SePageCtrl {
    uint32_t allPageCnt;       // 当前的 page 总数 used + free
    uint32_t usedPageCnt;      // 当前的 used page 数
    uint32_t freePageCnt;      // 当前的 free page 数
    uint32_t pageSize;         // 每页大小
    DbHashMapT *freePagePool;  // 空闲页池
    DbHashMapT *usingPagePool; // 使用中的页池
} SePageCtrlT;

SePageCtrlT *g_sePageCtrl = NULL;

SePageCtrlT *SeGetPageCtrlMng(void) {
    DB_ASSERT(g_sePageCtrl);
    return g_sePageCtrl;
}

// typedef uint32_t StatusInner; // SE内部错误码

void InitSePage(SePageT *pageMng, void *page, uint32_t pageId) {
    pageMng->nextPage = NULL;
    pageMng->pageAddr = page;
    pageMng->pageId = pageId;
    pageMng->pageInfo = (SePageInfoT){0};
    // pageMng->useSlotList = NULL;
    // pageMng->freeSlotList = NULL;
    // pageMng->slotSize = 0;
}

void SeInitPageCtrlInner(SePageT *pageMngList, void *pageList, DbHashMapT *freePagePool) {
    for (uint32_t i = 0; i < SE_INIT_PAGE_POOL_SIZE; ++i) {
        SePageT *pageMng = &pageMngList[i];
        void *page = (uint8_t *)pageList + i * SE_SINGLE_PAGE_SIZE;
        // 插入 freePagePool
        InitSePage(pageMng, page, i);
        uint32_t *pageId = (uint32_t *)DbDynMemCtxAlloc(freePagePool->memCtx, sizeof(uint32_t));
        if (pageId == NULL) {
            log_error("Alloc pageId failed. Alloc size id %u.", sizeof(uint32_t));
            DB_ASSERT(false);
            return;
            // return GMERR_MEMORY_ALLOC_FAILED;
        }
        Status ret = DbHashMapInsert(freePagePool, pageId, pageMng);
        DB_ASSERT(ret == GMERR_OK);
    }
    DB_ASSERT(DbHashGetSize(freePagePool) == SE_INIT_PAGE_POOL_SIZE);
}

Status SeInitPageCtrl() {
    uint32_t pageMngSize = SE_INIT_PAGE_POOL_SIZE * sizeof(SePageT);
    uint32_t pageSize = SE_INIT_PAGE_POOL_SIZE * SE_SINGLE_PAGE_SIZE;
    // 1. 申请 SE_INIT_PAGE_POOL_SIZE 页 SePageT (sizeof(SePageT))
    SePageT *pageMngList = (SePageT *)DbMalloc(pageMngSize);
    if (pageMngList == NULL) {
        log_error("Alloc pageMngList failed. Alloc size id %u.", pageMngSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    // 2. 申请 SE_INIT_PAGE_POOL_SIZE 页 page (4096)
    void *pageList = DbMalloc(pageSize);
    if (pageList == NULL) {
        log_error("Alloc pageList failed. Alloc size id %u.", pageSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }

    // 3. 初始化
    // 使用data MemCtx
    DbMemCtxT *dataMemCtx = DbGetDataMemCtx();
    DB_ASSERT(dataMemCtx != NULL);
    // 创建freePagePool
    DbHashMapT *freePagePool = NULL;

    Status ret = DbHashMapCreate(&freePagePool, DbHashUInt32, DbCmpUInt32, dataMemCtx);
    if (ret != GMERR_OK) {
        DbFree(pageMngList);
        DbFree(pageList);
        DbMemCtxReset(dataMemCtx);
        return ret;
    }
    SeInitPageCtrlInner(pageMngList, pageList, freePagePool);

    // 创建 usingPagePool
    DbHashMapT *usingPagePool = NULL;
    ret = DbHashMapCreate(&usingPagePool, DbHashUInt32, DbCmpUInt32, dataMemCtx);
    if (ret != GMERR_OK) {
        DbFree(pageMngList);
        DbFree(pageList);
        DbMemCtxReset(dataMemCtx);
        return ret;
    }

    // 4. 设置全局变量
    SePageCtrlT *sePageCtrl = (SePageCtrlT *)DbDynMemCtxAlloc(dataMemCtx, sizeof(SePageCtrlT));
    if (sePageCtrl == NULL) {
        DbFree(pageMngList);
        DbFree(pageList);
        DbMemCtxReset(dataMemCtx);
        return ret;
    }

    sePageCtrl->allPageCnt = SE_INIT_PAGE_POOL_SIZE;
    sePageCtrl->freePageCnt = SE_INIT_PAGE_POOL_SIZE;
    sePageCtrl->usedPageCnt = 0;
    sePageCtrl->pageSize = SE_PAGE_SIZE;
    sePageCtrl->freePagePool = freePagePool;
    sePageCtrl->usingPagePool = usingPagePool;
    // 初始化 runCtx
    ret = SEInitRunCtx();
    if (ret != GMERR_OK) {
        return ret;
    }
    g_sePageCtrl = sePageCtrl;
    // 初始化完成
    return GMERR_OK;
}

void HeapSetSlotNextAddr(void *slot, void *addr) { *(uint8_t **)slot = addr; }

void HeapSetSlotPrevAddr(void *slot, void *addr) { *(uint8_t **)((uint8_t *)slot + sizeof(void *)) = addr; }

void *HeapGetSlotNextAddr(void *slot) { return (void *)(*(uint8_t **)slot); }

void *HeapGetSlotPrevAddr(void *slot) { return (void *)(*(uint8_t **)((uint8_t *)slot + sizeof(void *))); }

uint32_t HeapGetSlotId(void *slot) {
    return *(uint32_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *) + sizeof(uint32_t));
}

uint32_t HeapGetPageId(void *slot) { return *(uint32_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *)); }

void HeapSetSlotId(void *slot, uint32_t slotId) {
    *(uint32_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *) + sizeof(uint32_t)) = slotId;
}

void HeapSetPageId(void *slot, uint32_t pageId) {
    *(uint32_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *)) = pageId;
}

// 设置删除标志位
void HeapSetDeleteFlag(void *slot) {
    *(uint8_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *) + sizeof(uint32_t) + sizeof(uint32_t)) = SE_SLOT_DELETE;
}

void HeapSetUsingFlag(void *slot) {
    *(uint8_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *) + sizeof(uint32_t) + sizeof(uint32_t)) = SE_SLOT_USING;
}

void HeapSetFreeFlag(void *slot) {
    *(uint8_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *) + sizeof(uint32_t) + sizeof(uint32_t)) = SE_SLOT_USING;
}

SeSlotStateT HeapGetSlotFlag(void *slot) {
    return *(uint8_t *)((uint8_t *)slot + sizeof(void *) + sizeof(void *) + sizeof(uint32_t) + sizeof(uint32_t));
}

void *HeapGetDataPos(void *slot) { return (void *)((uint8_t *)slot + SE_PAGE_ROW_EXTRA_SIZE); }

void HeapInitPage(HeapContainerT *container, SePageT *page) {
    memset(page->pageAddr, 0x00, SE_PAGE_SIZE);
    page->pageInfo = (SePageInfoT){0};
    page->pageInfo.recordSize = container->labelInfo.recordLen;
    page->pageInfo.slotSize = page->pageInfo.recordSize + SE_PAGE_ROW_EXTRA_SIZE;
    page->pageInfo.slotTotalCnt = SE_PAGE_SIZE / page->pageInfo.slotSize;

    DB_ASSERT(page->pageInfo.slotTotalCnt >= 2);
    page->pageInfo.slotFreeCnt = page->pageInfo.slotTotalCnt;
    page->pageInfo.slotUsedCnt = 0;
    page->pageInfo.nextFreeSlot = page->pageAddr; // free 指向 页起始地址

    void *currSlot = NULL;
    // 分割 page
    for (uint32_t i = 1; i < page->pageInfo.slotTotalCnt - 1; ++i) {
        currSlot = (uint8_t *)page->pageAddr + i * page->pageInfo.slotSize;
        // 设置 next addr
        HeapSetSlotNextAddr(currSlot, (uint8_t *)page->pageAddr + (i + 1) * page->pageInfo.slotSize);
        // 设置 prev addr
        HeapSetSlotPrevAddr(currSlot, (uint8_t *)page->pageAddr + (i - 1) * page->pageInfo.slotSize);
        HeapSetSlotId(currSlot, i);
    }
    // 设置 第一个 和 最后一个 slot
    // 设置 next addr
    currSlot = page->pageAddr;
    HeapSetSlotNextAddr(currSlot, (uint8_t *)page->pageAddr + page->pageInfo.slotSize);
    HeapSetSlotId(currSlot, 0);
    // 设置 prev addr
    currSlot = (uint8_t *)page->pageAddr + (page->pageInfo.slotTotalCnt - 1) * page->pageInfo.slotSize;
    HeapSetSlotPrevAddr(currSlot,
                        (uint8_t *)page->pageAddr + (page->pageInfo.slotTotalCnt - 2) * page->pageInfo.slotSize);
    HeapSetSlotId(currSlot, page->pageInfo.slotTotalCnt - 1);
}

Status HeapAllocAndInitNewPage(HeapContainerT *container, SePageT **outPage) {
    SePageCtrlT *pageCtrl = SeGetPageCtrlMng();
    uint32_t *pageId = NULL;
    SePageT *curPage = NULL;
    DbHashMapIter mapIter = 0;
    Status ret = DbHashMapFetch(pageCtrl->freePagePool, (void **)&pageId, (void **)&curPage, &mapIter);
    if (ret != GMERR_OK) {
        return ret;
    }

    // 初始化 page
    HeapInitPage(container, curPage);
    // 挂载 page 到container
    curPage->nextPage = container->pageList;
    container->pageList = curPage;
    container->pageCnt++;
    // 删除 map 对应元素
    ret = DbHashMapDelete(pageCtrl->freePagePool, pageId, false); // 删除时不释放内存！
    if (ret != GMERR_OK) {
        return ret;
    }
    // 插入到使用 页池
    ret = DbHashMapInsert(pageCtrl->usingPagePool, pageId, curPage);
    if (ret != GMERR_OK) {
        // 尝试重新插入 free mapPool
        Status retryRet = DbHashMapInsert(pageCtrl->freePagePool, pageId, curPage);
        DB_ASSERT(retryRet == GMERR_OK);
        return ret;
    }
    *outPage = curPage;
    return GMERR_OK;
}

void *GetSlotByAddr(HeapAddrT *addr) {
    DB_POINT(addr);
    SePageCtrlT *pageCtrl = SeGetPageCtrlMng();
    uint32_t pageId = addr->pageId;
    SePageT *curPage = (SePageT *)DbHashMapFind(pageCtrl->usingPagePool, &pageId);
    if (curPage == NULL) {
        log_warn("Page not found. Page id %u.", addr->pageId);
        return NULL;
    }
    DB_ASSERT(addr->slotId < curPage->pageInfo.slotTotalCnt);
    return (uint8_t *)curPage->pageAddr +
           addr->slotId * curPage->pageInfo.slotSize; // TODO:后续可以加判断看当前slot是否空闲
}
