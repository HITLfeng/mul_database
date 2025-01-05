#ifndef __INTERFACE_COMMON_H__
#define __INTERFACE_COMMON_H__

#include <stdint.h>
#include "out_type_defs.h"
#include "db_memctx.h"

#ifdef __cplusplus
extern "C" {
#endif

// 存放所有模块公共结构体 但是不会对外暴露
typedef struct RunCtx {
    OperatorCode opCode;
    void *entry; // EE层根据opCode将runtime分发的信息解析并存入结构体中
    uint32_t entryLen;
    void *retEntry; // runtime层根据opcode解析EE层返回的结果 解析完后请手动释放
    uint32_t retEntryBufLen;
    uint32_t fetchCnt;        // 本次查询返回的记录条数
    uint32_t currDbId;        // 当前正在操作的数据库ID
    uint32_t currLabelId;     // 当前正在操作的labelID
    uint32_t currLabelFldCnt; // 当前正在操作的label字段个数
    DbMemCtxT *memCtx;        // 挂在 QryStmt 上，用于本次请求期间的内存申请
} RunCtxT;

typedef RunCtxT QryStmtT;

#ifdef __cplusplus
}
#endif

#endif // __INTERFACE_COMMON_H__