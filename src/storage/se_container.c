#include "se_common.h"

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

Status HeapInsert(HeapContainerT *container, void *dataBuf)
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
    if (container->pageCnt == 0) {
        // 设置为头节点
        HeapSetSlotNextAddr(slot, slot);
        HeapSetSlotPrevAddr(slot, slot);
        container->useSlotList = slot;
    } else {
        DB_ASSERT(container->useSlotList != NULL);
        // HEAD0 == HEAD1    newHEAD
        void *headSlot = container->useSlotList;
        // 获取尾节点
        void *tailSlot = HeapGetSlotPrevAddr(headSlot);
        HeapSetSlotNextAddr(tailSlot, slot);
        HeapSetSlotPrevAddr(slot, tailSlot);
        HeapSetSlotNextAddr(slot, headSlot);
        HeapSetSlotPrevAddr(headSlot, slot);
    }
    // 5.更新 container 相关结构体内容
    container->recordCnt++;
    return GMERR_OK;
}

// TODO: heap 删除 和 更新接口
Status HeapDelete();
Status HeapUpdate();