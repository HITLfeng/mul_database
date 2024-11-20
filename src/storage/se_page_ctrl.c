#include "se_common.h"


// se层page使用页管理 一页 4096 bytes
#define SE_SINGLE_PAGE_SIZE 4096

// se层初始申请的页的个数 可动态扩容，不会缩容
#define SE_INIT_PAGE_COUNT 5


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

typedef struct HeapAddr {
    uint32_t pageId;
    uint32_t slotId;
} HeapAddrT;


// TODO: 不会存在空页，空页理论上会被马上回收
typedef struct SePage {
    void *nextPage;
    void *pageAddr;
    void *freeSlotList;
    void *useSlotList; // 本页的，当找到尽头后，寻找下一页
    uint32_t slotSize; // 每个槽位的大小
} SePageT;

typedef struct SePageCtrl {
    uint32_t allPageCnt; // 当前的 page 总数 used + free
    uint32_t usedPageCnt; // 当前的 used page 数
    uint32_t freePageCnt; // 当前的 free page 数

} SePageCtrlT;




