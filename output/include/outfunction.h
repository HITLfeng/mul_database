#include "out_type_defs.h"

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

// 收发报文函数
CliStatus KVCSend(DbConnectT *conn, const MsgBufRequestT *msgBuf);
CliStatus KVCRecv(DbConnectT *conn, MsgBufResponseT *msgBuf);

// 对外接口测试函数
CliStatus KVCCalcTwoNumber(DbConnectT *conn, int x, int y, CalcOptionT opt, UsrResultBaseT *result);



// *****************************************
// * 简单关系表相关函数 SIMPLE RELATION TABLE
// *****************************************

// 创建 关系表 所在 database // namespace
CliStatus SRCCreateDb(DbConnectT *conn, const char *dbName, uint32_t *dbId);

// 删除 关系表 所在 database // namespace
CliStatus SRCDeleteDb(DbConnectT *conn, const char *dbName);

// 创建 关系表
CliStatus SRCCreateLabel(DbConnectT *conn, const char *labelName, SrDbCreateLabelCtxT *createLabelCtx);

// 删除 关系表
CliStatus SRCDeleteLabel(DbConnectT *conn, const char *labelName);

// 插入数据
CliStatus SRCInsertData(DbConnectT *conn, const char *labelName, const char *key, const char *value);
// 查询数据
CliStatus SRCQueryData(DbConnectT *conn, const char *labelName, const char *key, char *value, uint32_t valueLen);
// 删除数据
CliStatus SRCDeleteData(DbConnectT *conn, const char *labelName, const char *key);





#ifdef __cplusplus
}
#endif