#ifndef SE_OUT_FUNCTION_H
#define SE_OUT_FUNCTION_H

#include <stdint.h>
#include "common.h"
#include "db_memctx.h"
#include "kv_map.h"

// 对外接口 使用 SE 开头

#ifdef __cplusplus
extern "C" {
#endif

#define SE_HEAP_PAGE_SIZE 4096

// typedef struct FixedHeap {
//     void *pageBegin; // 页初始位置
//     void *currPos; // 页当前位置
//     void *pageEnd; // 页结束位置 当前不支持扩页
//     uint32_t rowCnt; // 记录数
//     uint32_t rowSize; // 一行记录长度
//     uint32_t pageSize; // 页大小 默认4K
// } FixedHeapT;

// void SEFixedHeapInit(FixedHeapT *heap, uint32_t rowSize);

// void SEFixedHeapInsertRow(FixedHeapT *heap, void *rowBuf);


/**
 * 建立一个 map 
 * key: labelId value: container
 */

typedef struct SERunCtx {
    DbHashMapT *containerMap; // 存储 labelId: container
    DbMemCtxT *memCtx;
} SERunCtxT;

typedef struct HeapAddr {
    uint32_t pageId;
    uint32_t slotId;
} HeapAddrT;

// SE 层初始化 container 需要使用的结构体，该结构体初始化由EE层使用label进行
typedef struct SeLabelInfo {
    uint32_t dbId;
    uint32_t labelId;
    uint32_t recordLen; // 记录长度
} SeLabelInfoT;

SERunCtxT *SEGetRunCtx();

/**
 * 服务器第一次拉起时调用 初始化表内存
 */
Status SeInitPageCtrl(void);

/**
 * 给表创建一个容器 用于存储数据
 * @param label 要创建容器的表
 * @return
 */
Status HeapContainerCreate(SeLabelInfoT *labelInfo);

/**
 * SE 层对其他模块提供的重要接口 往表中插入一条数据
 * @param labelId 【IN】要插入的表的ID
 * @param dataBuf 【IN】要插入的用户数据
 * @param addr    【OUT】出参，返回记录的地址 传入 [NULL] 则不做处理！
 * @return
 */
Status SEHeapInsertRow(uint32_t labelId, uint8_t *dataBuf, HeapAddrT *addr);

#ifdef __cplusplus
}
#endif

#endif // SE_OUT_FUNCTION_H