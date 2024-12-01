#include "se_common.h"

void InitContainerWithLaebl(HeapContainerT *container, SeLabelInfoT *labelInfo)
{
    container->pageList = NULL;
    container->pageCnt = 0;
    container->useSlotList = NULL;
    container->recordCnt = 0;
    container->labelInfo.dbId = labelInfo->dbId;
    container->labelInfo.labelId = labelInfo->labelId;
    container->labelInfo.recordLen = labelInfo->recordLen;
}

Status HeapContainerCreate(SeLabelInfoT *labelInfo)
{
    // 获取运行上下文
    SERunCtxT *runCtx = SEGetRunCtx();

    // TODO: HERE jixu xie 202411242207
    HeapContainerT *container = (HeapContainerT *) DbDynMemCtxAlloc(runCtx->memCtx, sizeof(HeapContainerT));
    if (container == NULL) {
        log_error("Alloc container failed. Alloc size id %u.", sizeof(HeapContainerT));
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    InitContainerWithLaebl(container, labelInfo);

    uint32_t * pageId = (uint32_t * )
    DbDynMemCtxAlloc(runCtx->memCtx, sizeof(uint32_t));
    if (pageId == NULL) {
        log_error("Alloc container failed. Alloc size id %u.", sizeof(uint32_t));
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    *pageId = labelInfo->labelId;

    // 插入 container map
    return DbHashMapInsert(runCtx->containerMap, pageId, container);
}

// 每个 label 对应一个 container




// 内部接口 insert 一条记录 记入 page 中

SePageT *HeapContainerGetPage(HeapContainerT *container)
{
    if (container->pageList == NULL) {
        return NULL;
    }
    SePageT *currPage = container->pageList;
    while (currPage != NULL) {
        if (currPage->pageInfo.slotFreeCnt > 0) {
            return currPage;
        }
        currPage = currPage->nextPage;
    }
    return NULL;
}

//Status HeapAllocNewPage(HeapContainerT *container, SePageT **outPage)
//{
//    uint32_t *pageId = NULL;
//    SePageT *curPage = NULL;
//    DbHashMapFetch()
//}

//void *HeapGetSlotNextAddr(void *slot);
//void *HeapGetSlotPrevAddr(void *slot);
//uint32_t HeapGetSlotId(void *slot);

void *HeapGetPageFreeSlot(SePageT *page)
{
    DB_ASSERT(page->pageInfo.slotFreeCnt > 0);
    // 当前函数流程中 page 不应该没有空闲 slot
    void *freeSlot = page->pageInfo.nextFreeSlot;
    page->pageInfo.nextFreeSlot = HeapGetSlotNextAddr(freeSlot);
    page->pageInfo.slotUsedCnt++;
    page->pageInfo.slotFreeCnt--;
    // next prev 全部置空
    HeapSetSlotNextAddr(freeSlot, NULL);
    HeapSetSlotPrevAddr(freeSlot, NULL);
    return freeSlot;
}

Status HeapInsert(HeapContainerT *container, void *dataBuf, HeapAddrT *addr)
{
    Status ret = GMERR_OK;
    // 1.判断是否需要 申请新的页下来
    SePageT *page = HeapContainerGetPage(container);
    if (page == NULL) {
        // 申请新的页
        ret = HeapAllocAndInitNewPage(container, &page);
        if (ret != GMERR_OK) {
            return ret;
        }
    }
    DB_ASSERT(page);
    // 2.从该页中获取空闲槽位
    void *slot = HeapGetPageFreeSlot(page);
    // 3.将buf写入slot
    memcpy(HeapGetDataPos(slot), dataBuf, container->labelInfo.recordLen);

    // 4.slot 插入 container 中的记录链
    if (container->useSlotList == NULL) {
        // 设置为头节点
        HeapSetSlotNextAddr(slot, slot);
        HeapSetSlotPrevAddr(slot, slot);
        container->useSlotList = slot;
    } else {
        // DB_ASSERT(container->useSlotList != NULL);
        // HEAD0 == HEAD1    newHEAD
        void *headSlot = container->useSlotList;
        // 获取尾节点
        void *tailSlot = HeapGetSlotPrevAddr(headSlot);
        HeapSetSlotNextAddr(tailSlot, slot);
        HeapSetSlotPrevAddr(slot, tailSlot);
        HeapSetSlotNextAddr(slot, headSlot);
        HeapSetSlotPrevAddr(headSlot, slot);
    }
    // 5.更新 pageId + slotId
    HeapSetPageId(slot, page->pageId);
    HeapSetSlotId(slot, HeapGetSlotId(slot));
    // 6.更新 container 相关结构体内容
    container->recordCnt++;
    container->lastRecordSlot = slot;
    // 7.赋值 ADDR
    if (addr != NULL) {
        addr->pageId = page->pageId;
        addr->slotId = HeapGetSlotId(slot);
    }
    return GMERR_OK;
}

// TODO: heap 删除 和 更新接口
Status HeapDelete();

Status HeapUpdate();


Status SEHeapInsertRow(uint32_t labelId, uint8_t *dataBuf, HeapAddrT *addr)
{
    // 获取运行上下文
    SERunCtxT *runCtx = SEGetRunCtx();
    uint32_t targetLabelId = labelId;
    // 根据表ID获取容器
    HeapContainerT *container = (HeapContainerT *) DbHashMapFind(runCtx->containerMap, &targetLabelId);
    if (container == NULL) {
        log_error("container is not exist, label id is %u.", targetLabelId);
        return GMERR_STORAGE_CONTAINER_NOT_EXIST;
    }
    Status ret = HeapInsert(container, dataBuf, addr);
    if (ret != GMERR_OK) {
        return ret;
    }
    return GMERR_OK;
}