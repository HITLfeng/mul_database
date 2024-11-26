#ifndef SE_COMMON_H
#define SE_COMMON_H

#include <stdint.h>
#include "common.h"
#include "db_memctx.h"
#include "common.h"
#include "kv_map.h"
#include "se_out_function.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SePageInfo {
    void *nextFreeSlot; // 指向本页下一处free slot

    uint32_t recordSize; // 用户存储记录长度
    uint32_t slotSize; // 每个槽位的大小

    uint32_t slotTotalCnt; // 总槽位数量
    uint32_t slotUsedCnt; // 使用了的槽位数量
    uint32_t slotFreeCnt; // 空闲的槽位数量
} SePageInfoT;

// ROW 结构
// | nextAddr | preAddr | data Buf | isDelete(预留) |
//

// TODO: 不会存在空页，空页理论上会被马上回收
typedef struct SePage {
    void *nextPage;
    void *pageAddr;
    uint32_t pageId; // 页 ID

    SePageInfoT pageInfo; // 被使用后 初始化该值
} SePageT;

typedef struct HeapLabelInfo {
    uint32_t dbId;
    uint32_t labelId;
    uint32_t recordLen; // 表中所有字段总长度
} HeapLabelInfoT;

typedef struct HeapContainer {
    SePageT *pageList; // 当前container中申请到的页 链表形式报错
    uint32_t pageCnt; // 申请页的数量
//    void *freeSlotList;
    void *useSlotList; // 记录链表
    uint32_t recordCnt; // 记录数
    HeapLabelInfoT labelInfo; // 打开该容器的表的信息
} HeapContainerT;

//void *SeGetPageCtrlMng(void);

typedef struct LabelCursor {
    uint32_t labelId;
    HeapContainerT *container;
} LabelCursorT;





/**
 * 暂不对外提供 在外部接口 SeInitPageCtrl 内部调用
* 初始化 全局 存储运行上下文
* 使用 dataMemCtx
*/
Status SEInitRunCtx(void);

/**
 *
 * @param container 为当前容器申请新的 page
 * @return
 */
Status HeapAllocAndInitNewPage(HeapContainerT *container, SePageT **outPage);


void HeapSetSlotNextAddr(void *slot, void *addr);
void HeapSetSlotPrevAddr(void *slot, void *addr);
void HeapSetSlotId(void *slot, uint32_t slotId);
void *HeapGetSlotNextAddr(void *slot);
void *HeapGetSlotPrevAddr(void *slot);
void *HeapGetDataPos(void *slot);
uint32_t HeapGetSlotId(void *slot);

#ifdef __cplusplus
}
#endif

#endif // SE_COMMON_H