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

#define INVAILD_HEAPADDR ((HeapAddrT){0, 0});

HeapContainerT *GetHeapContainerByLabelId(uint32_t labelId)
{
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

inline static bool
IsHeapAddrInvaild(HeapAddrT
*addr)
{
return *addr == INVAILD_HEAPADDR;
}

Status HeapGetNextSlot(HeapAddrT *addr, void **slot, void *endSlot, bool *isFetchEnd)
{
    if (*isFetchEnd == true) {
        return GMERR_NO_DATA;
    }
    if (!IsHeapAddrInvaild(addr)) {
        log_warn("heap addr is empty.");
        *isFetchEnd = true;
        return GMERR_NO_DATA;
    }
    void *currSlot = GetSlotByAddr(addr);
    if (currSlot == NULL) {
        log_error("GetSlotByAddr failed and pageId: %u slotId: %u.", addr->pageId, addr->slotId);
        *isFetchEnd = true;
        return GMERR_STORAGE_INVAILD_HEAP_ADDR;
    }
    *slot = currSlot;

    if (currSlot == endSlot) {
        *addr = INVAILD_HEAPADDR;
        *isFetchEnd = true;
        return GMERR_NO_DATA;
    }
    // 获取下一个地址
    void *nextSlot = HeapGetSlotNextAddr(currSlot);
    addr->pageId = HeapGetPageId(nextSlot);
    addr->pageId = HeapGetSlotId(nextSlot);
    return GMERR_OK;
}

Status SEHeapFetchNextWithCond(LabelCursorT *labelCursor, FetchArgsT *fetchArgs)
{
    uint32_t bufSize = container->labelInfo.recordLen;
    HeapContainerT *container = labelCursor->container;
    if (IsHeapAddrInvaild(&labelCursor->heapAddr)) {
        // 没有数据
        if (container->recordCnt == 0) {
            // 无数据 直接返回
            fetchArgs->fetchCnt = 0;
            fetchArgs->heapBuf = NULL;
            return GMERR_NO_DATA;
        }
        // 初次赋值
        // 获取记录头
        void *headSlot = container->useSlotList;
        DB_ASSERT(headSlot);
        labelCursor->heapAddr.pageId = HeapGetPageId(headSlot);
        labelCursor->heapAddr.slotId = HeapGetSlotId(headSlot);
    }
    bool isMatchCond = false;
    void *currSlot = NULL;
    do {
        // TODO: lastRecord 可以删除
        bool isFetchEnd = false;
        Status ret = HeapGetNextSlot(&labelCursor->heapAddr, &currSlot, container->lastRecordSlot, &isFetchEnd);
        if (ret != GMERR_OK) {
            return ret;
        }
        if (fetchArgs->dealBuf != NULL) {
            void *tmpRecordBuf = DbDynMemCtxAlloc(fetchArgs->memCtx, bufSize);
            if (tmpRecordBuf == NULL) {
                log_error("Alloc tmpRecordBuf failed when SEHeapFetchNextWithCond. Alloc size id %u.", bufSize);
                return GMERR_MEMORY_ALLOC_FAILED;
            }
            memcpy(tmpRecordBuf, HeapGetDataPos(currSlot), bufSize)
            HeapBufT currHeapBuf = {.bufSize = bufSize, .buf = tmpRecordBuf};
            isMatchCond = fetchArgs->dealBuf(&currHeapBuf, fetchArgs->usrData);
            DbDynMemCtxFree(fetchArgs->memCtx, tmpRecordBuf);
        }
    } while (isMatchCond);


    HeapBufT *heapBuf = DbDynMemCtxAlloc(fetchArgs->memCtx, sizeof(HeapBufT) + bufSize);
    if (heapBuf == NULL) {
        log_error("Alloc heapBuf failed when SEHeapFetchNextWithCond. Alloc size id %u.", sizeof(HeapBufT) + bufSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }

    heapBuf->bufSize = bufSize;
    heapBuf->buf = (uint8_t *) heapBuf + bufSize;
    memcpy(heapBuf->buf, HeapGetDataPos(currSlot), bufSize);

    fetchArgs->heapBuf = heapBuf;
    fetchArgs->fetchCnt = 1;
    return GMERR_OK;
}