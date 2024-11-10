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




