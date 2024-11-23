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
        if (currPage->slotFreeCnt != 0) {
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

Status HeapInsert(HeapContainerT *container)
{
    Status ret = GMERR_OK;
    // 判断是否需要 申请新的页下来
    SePageT *page = HeapContainerGetPage(container);
    if (page == NULL) {
        // 申请新的页
        ret = HeapAllocAndInitNewPage(container, &page);
        if (ret != GMERR_OK) {
            return ret;
        }
    }
    DB_ASSERT(page);
 // TODO: HERE


    return GMERR_OK;
}