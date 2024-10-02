#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__

#include "../../interface/include/out_type_defs.h"
#include "../../common/include/seri_utils.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct UsrDataBase {
    uint32_t ret;
} UsrDataBaseT;

typedef struct UsrDataCalc {
    uint32_t ret;
    uint32_t calcAns;
} UsrDataCalcT;

// ************************************
// SIMPLERELATION 相关类型定义 start
// ************************************

typedef struct UsrDataSimpleRel {
    uint32_t ret;
    union srData {
        uint32_t dbId;
        uint32_t labelId;
        /* data */
    } srData;
} UsrDataSimpleRelT;

typedef struct UsrDataSimpleRelStmt {
    uint32_t ret;
    CliStmtT *stmt;
} UsrDataSimpleRelStmtT;

uint8_t *GetUsrDataPosition(uint8_t *usrMsgBuf);

typedef void (*SRParseResponseCb)(uint8_t **,
                                  UsrDataBaseT *); // 回调函数， 两个入参分别为 服务端返回报文、用户数据结构体

// ************************************
// 核心函数
// ************************************
CliStatus KVCSendRequestAndRecvResponse(DbConnectT *conn, MsgBufRequestT *msgBuf, SRParseResponseCb cbExec,
                                        UsrDataBaseT *usrData);

void CltParseBaseMsgBuf(MsgBufResponseT *respBuf, UsrDataBaseT *result);

void SRCInitMsgBuf(MsgBufRequestT *msgBuf, OperatorCode opCode);

#ifdef __cplusplus
}
#endif

#endif // __CLIENT_COMMON_H__
