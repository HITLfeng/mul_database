#include "se_common.h"

// typedef struct SeRunCtx {} SeRunCtxT;

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

#define HEAP_INVAILD_ID 0xffffffef
#define INVAILD_HEAPADDR ((HeapAddrT){HEAP_INVAILD_ID, HEAP_INVAILD_ID})

HeapContainerT *GetHeapContainerByLabelId(uint32_t labelId) {
    SERunCtxT *runCtx = SEGetRunCtx();
    uint32_t targetLabelId = labelId;
    // 根据表ID获取容器
    HeapContainerT *container = (HeapContainerT *)DbHashMapFind(runCtx->containerMap, &targetLabelId);
    if (container == NULL) {
        return NULL;
    }
    return container;
}

Status SEHeapOpenLabelCursor(uint32_t labelId, LabelCursorT *labelCursor) {
    DB_POINT(labelCursor);
    if (labelCursor->labelId != 0) {
        log_error("error when init labelCursor. label id is %u and not equal to 0.", labelCursor->labelId);
        return GMERR_STORAGE_LABELCURSOR_USED;
    }
    HeapContainerT *container = GetHeapContainerByLabelId(labelId);
    if (container == NULL) {
        log_error("container is not exist, label id is %u.", labelId);
        return GMERR_STORAGE_CONTAINER_NOT_EXIST;
    }
    labelCursor->labelId = labelId;
    labelCursor->container = container;
    labelCursor->heapAddr = INVAILD_HEAPADDR;
    labelCursor->isFetchEnd = false;
    return GMERR_OK;
}

inline static bool IsHeapAddrInvaild(HeapAddrT *addr) {
    return addr->pageId == HEAP_INVAILD_ID && addr->slotId == HEAP_INVAILD_ID;
}

Status HeapGetNextSlot(HeapAddrT *addr, void **slot, void *endSlot, bool *isFetchEnd) {
    if (*isFetchEnd == true) {
        return GMERR_NO_DATA;
    }
    if (IsHeapAddrInvaild(addr)) {
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
    // 走到这说明能捞到数据
    *slot = currSlot;

    if (currSlot == endSlot) {
        *addr = INVAILD_HEAPADDR;
        *isFetchEnd = true;
        return GMERR_OK;
    }
    // 获取下一个地址
    void *nextSlot = HeapGetSlotNextAddr(currSlot);
    addr->pageId = HeapGetPageId(nextSlot);
    addr->slotId = HeapGetSlotId(nextSlot);
    return GMERR_OK;
}

// 查找下一条符合条件的slot
static Status HeapFetchNextSlotWithCond(LabelCursorT *labelCursor, FetchArgsT *fetchArgs, void **slot) {
    *slot = NULL;
    HeapContainerT *container = labelCursor->container;
    uint32_t bufSize = container->labelInfo.recordLen;
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
            // 捞不到数据后这里可以正常返回 非出错
            return ret;
        }
        if (isFetchEnd) {
            labelCursor->isFetchEnd = true; // 表示已经捞到最后一条数据
        }
        DB_ASSERT(HeapGetSlotFlag(currSlot) != SE_SLOT_FREE);
        if (HeapGetSlotFlag(currSlot) == SE_SLOT_DELETE) {
            continue;
        }
        if (fetchArgs->matchCond != NULL) {
            void *tmpRecordBuf = DbDynMemCtxAlloc(fetchArgs->memCtx, bufSize);
            if (tmpRecordBuf == NULL) {
                log_error("Alloc tmpRecordBuf failed when SEHeapFetchNextWithCond. Alloc size id %u.", bufSize);
                return GMERR_MEMORY_ALLOC_FAILED;
            }
            memcpy(tmpRecordBuf, HeapGetDataPos(currSlot), bufSize);
            HeapBufT currHeapBuf = {.bufSize = bufSize, .buf = tmpRecordBuf};
            isMatchCond = fetchArgs->matchCond(&currHeapBuf, fetchArgs->usrData);
            DbDynMemCtxFree(fetchArgs->memCtx, tmpRecordBuf);
        } else {
            isMatchCond = true;
        }
    } while (!isMatchCond && !labelCursor->isFetchEnd);

    if (labelCursor->isFetchEnd && !isMatchCond) {
        // 说明已经捞到最后一条数据了
        // fetchArgs->fetchCnt = 0;
        fetchArgs->heapBuf = NULL;
        return GMERR_NO_DATA;
    }
    *slot = currSlot;
    return GMERR_OK;
}

Status SEHeapFetchAndDeleteWithCond(LabelCursorT *labelCursor, FetchArgsT *fetchArgs) {
    void *currSlot = NULL;
    Status ret = HeapFetchNextSlotWithCond(labelCursor, fetchArgs, &currSlot);
    if (ret != GMERR_OK) {
        // 可能返回无数据 为正常情况
        return ret;
    }
    DB_ASSERT(currSlot != NULL);
    HeapSetDeleteFlag(currSlot);
    return GMERR_OK;
}

Status SEHeapFetchNextWithCond(LabelCursorT *labelCursor, FetchArgsT *fetchArgs) {
    void *currSlot = NULL;
    Status ret = HeapFetchNextSlotWithCond(labelCursor, fetchArgs, &currSlot);
    if (ret != GMERR_OK) {
        // 可能返回无数据 为正常情况
        return ret;
    }
    DB_ASSERT(currSlot != NULL);
    uint32_t bufSize = ((HeapContainerT *)(labelCursor->container))->labelInfo.recordLen;
    HeapBufT *heapBuf = DbDynMemCtxAlloc(fetchArgs->memCtx, sizeof(HeapBufT) + bufSize);
    if (heapBuf == NULL) {
        log_error("Alloc heapBuf failed when SEHeapFetchNextWithCond. Alloc size id %u.", sizeof(HeapBufT) + bufSize);
        return GMERR_MEMORY_ALLOC_FAILED;
    }

    heapBuf->bufSize = bufSize;
    heapBuf->buf = (uint8_t *)heapBuf + sizeof(HeapBufT);
    memcpy(heapBuf->buf, HeapGetDataPos(currSlot), bufSize);

    fetchArgs->heapBuf = heapBuf;
    fetchArgs->fetchCnt++;
    return GMERR_OK;
}