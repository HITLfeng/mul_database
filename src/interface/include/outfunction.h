#ifndef __OUT_FUNCTION_H__
#define __OUT_FUNCTION_H__

#include "out_type_defs.h"
#include <stdarg.h>

typedef uint32_t CliStatus;

typedef enum CalcOption {
    CALC_ADD = 0,
    CALC_SUB,
    CALC_MUL,
    CALC_DIV,
    CALC_BUTT
} CalcOptionT;

#ifdef __cplusplus
extern "C"
{
#endif


// 启动服务
CliStatus KVCSrvStart(void);
CliStatus KVCSrvStop(void);

// 建联 断联函数

CliStatus KVCConnect(DbConnectT *conn);
CliStatus KVCDisconnect(DbConnectT *conn);

// 准备操作句柄函数 配套使用
CliStatus KVCPrepareStmt(DbConnectT *conn, CliStmtT **stmt, uint32_t dbId, uint32_t labelId);
CliStatus KVCReleaseStmt(CliStmtT **stmt);

// 收发报文函数
CliStatus KVCSend(DbConnectT *conn, const MsgBufRequestT *msgBuf);
CliStatus KVCRecv(DbConnectT *conn, MsgBufResponseT *msgBuf);

// 对外接口测试函数
CliStatus KVCCalcTwoNumber(DbConnectT *conn, int x, int y, CalcOptionT opt, int *result);



// *****************************************
// * 简单关系表相关函数 SIMPLE RELATION TABLE
// *****************************************

// 创建 关系表 所在 database // namespace
CliStatus SRCCreateDb(DbConnectT *conn, const char *dbName, uint32_t *dbId);

// 删除 关系表 所在 database // namespace
CliStatus SRCDeleteDb(DbConnectT *conn, const char *dbName);

// 创建 关系表
CliStatus SRCCreateLabelWithJson(DbConnectT *conn, uint32_t dbId, const char *labelJson, uint32_t *labelId);
// 暂时不支持
// CliStatus SRCCreateLabel(DbConnectT *conn, const char *labelName, SrDbCreateLabelCtxT *createLabelCtx);

// 删除 关系表
CliStatus SRCDeleteLabel(DbConnectT *conn, const char *labelName);

// 插入数据
CliStatus SRCInsertData(CliStmtT *stmt, ...);


// 查询数据
CliStatus SRCQueryData(DbConnectT *conn, const char *labelName, const char *key, char *value, uint32_t valueLen);
// 删除数据
CliStatus SRCDeleteData(DbConnectT *conn, const char *labelName, const char *key);

// DFX 关系表
CliStatus SRCTraceDbDesc(DbConnectT *conn, uint32_t dbId);





#ifdef __cplusplus
}
#endif

#endif  // __OUT_FUNCTION_H__