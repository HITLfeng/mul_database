#ifndef SE_OUT_FUNCTION_H
#define SE_OUT_FUNCTION_H

#include <stdint.h>
#include "common.h"

// 对外接口 使用 SE 开头

#ifdef __cplusplus
extern "C" {
#endif

#define SE_HEAP_PAGE_SIZE 4096

typedef struct FixedHeap {
    void *pageBegin; // 页初始位置
    void *currPos; // 页当前位置
    void *pageEnd; // 页结束位置 当前不支持扩页
    uint32_t rowCnt; // 记录数
    uint32_t rowSize; // 一行记录长度
    uint32_t pageSize; // 页大小 默认4K
} FixedHeapT;

void SEFixedHeapInit(FixedHeapT *heap, uint32_t rowSize);

void SEFixedHeapInsertRow(FixedHeapT *heap, void *rowBuf);


/**
 * 建立一个 map 
 * key: labelId value: container
 */




/**
 * 服务器第一次拉起时调用 初始化表内存
 */
Status SeInitPageCtrl(void);

#ifdef __cplusplus
}
#endif

#endif // SE_OUT_FUNCTION_H