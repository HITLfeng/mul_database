#include "se_common.h"


//typedef struct SeRunCtx {} SeRunCtxT;

// void SEFixedHeapInit(FixedHeapT *heap, uint32_t rowSize) {
//     memset(heap, 0x00, sizeof(FixedHeapT));
//     heap->rowCnt = 0;
//     heap->rowSize = rowSize;
//     heap->pageSize = SE_HEAP_PAGE_SIZE;

//     void *page = KVMemAlloc(heap->pageSize);
//     DB_ASSERT(page != NULL);
//     heap->pageBegin = page;
//     heap->currPos = page;
//     heap->pageEnd = page + heap->pageSize;
// }

// void SEFixedHeapInsertRow(FixedHeapT *heap, void *rowBuf) {
//     if (heap->currPos + heap->rowSize > heap->pageEnd) {
//         // TODO 申请新页
//         // 先报错
//         DB_ASSERT(false);
//     }
//     memcpy(heap->currPos, rowBuf, heap->rowSize);
//     heap->currPos += heap->rowSize;
//     heap->rowCnt++;
// }

HeapContainerT *GetHeapContainerByLabelId(uint32_t labelId) {
    SERunCtxT *runCtx = SEGetRunCtx();
    uint32_t targetLabelId = labelId;
    // 根据表ID获取容器
    HeapContainerT *container = (HeapContainerT *) DbHashMapFind(runCtx->containerMap, &targetLabelId);
    if (container == NULL) {
        return NULL;
    }
    return container;
}

Status SEHeapOpenLabelCursor(uint32_t labelId, LabelCursorT *labelCursor)
{
    DB_POINT(labelCursor);
    if (labelCursor->labelId != 0) {
        log_error("error when init labelCursor. label id is %u and not equal to 0.", labelCursor->labelId);
        return GMERR_STORAGE_LABELCURSOR_USED;
    }
    HeapContainerT *container = GetHeapContainerByLabelId(labelId);
    if (container == NULL) {
        log_error("container is not exist, label id is %u.", targetLabelId);
        return GMERR_STORAGE_CONTAINER_NOT_EXIST;
    }
    labelCursor->labelId = labelId;
    labelCursor->container = container;
    return GMERR_OK;
}



Status HeapGetNextSlot(HeapAddrT *addr, void **slot)
{
    void *currSlot = GetSlotByAddr(addr);
    if (currSlot == NULL) {
        log_error("GetSlotByAddr failed and pageId: %u slotId: %u.", addr->pageId, addr->slotId);
        return GMERR_STORAGE_INVAILD_HEAP_ADDR;
    }
    // 获取下一个地址
    void *nextSlot = HeapGetSlotNextAddr(currSlot);
    // TODO: 现在好像是双向链表 要么改成单向 页面设置可区分的头节点
    if (nextSlot == NULL) {
        return GMERR_NO_DATA;
    }

    *slot = currSlot;
    addr->pageId = HeapGetPageId(nextSlot);
    addr->pageId = HeapGetSlotId(nextSlot);
    return GMERR_OK;
}

Status SEHeapFetchNextWithCond(LabelCursorT *labelCursor) {

}