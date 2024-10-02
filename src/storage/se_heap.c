#include "se_out_function.h"
#include "kv_memory.h"



void SEFixedHeapInit(FixedHeapT *heap, uint32_t rowSize) {
    memset(heap, 0x00, sizeof(FixedHeapT));
    heap->rowCnt = 0;
    heap->rowSize = rowSize;
    heap->pageSize = SE_HEAP_PAGE_SIZE;

    void *page = KVMemAlloc(heap->pageSize);
    DB_ASSERT(page != NULL);
    heap->pageBegin = page;
    heap->currPos = page;
    heap->pageEnd = page + heap->pageSize;
}

void SEFixedHeapInsertRow(FixedHeapT *heap, void *rowBuf) {
    if (heap->currPos + heap->rowSize > heap->pageEnd) {
        // TODO 申请新页
        // 先报错
        DB_ASSERT(false);
    }
    memcpy(heap->currPos, rowBuf, heap->rowSize);
    heap->currPos += heap->rowSize;
    heap->rowCnt++;
}