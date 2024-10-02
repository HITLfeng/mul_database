#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

#include "../common/include/seri_utils.h"
// #include "seri_utils.h"

#include "include/client_common.h"

void SetCalcUsrMsgBuf(char *usrMsgBuf, int x, int y, CalcOptionT opt)
{
    uint8_t *bufCursor = (uint8_t *)usrMsgBuf;
    SeriInt32M(&bufCursor, x);
    SeriInt32M(&bufCursor, y);
    SeriCharM(&bufCursor, opt);
}



// CliStatus CltParseCalcMsgBuf(MsgBufResponseT *respBuf, UsrDataBaseT *result)
// {
//     DB_POINT2(respBuf, result);
//     MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)respBuf;
//     UsrDataCalcT *calcRes = (UsrDataCalcT *)result;
//     calcRes->ret = respHead->status;
//     uint8_t *bufCursor = GetUsrDataPosition((uint8_t *)respBuf);
//     calcRes->calcAns = DeseriInt(&bufCursor);
//     return GMERR_OK;
// }

void CltParseCalcRspBuf(uint8_t **respBuf, UsrDataBaseT *result)
{
    DB_POINT2(respBuf, result);
    // 1.解析服务端返回值
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)*respBuf;
    if (respHead->status != GMERR_OK)
    {
        log_error("calc error, server status = %d", respHead->status);
        return;
    }
    // 2.解析数据
    *respBuf += sizeof(MsgBufResponseHeadT);
    UsrDataCalcT *calcRes = (UsrDataCalcT *)result;
    calcRes->ret = respHead->status;
    calcRes->calcAns = DeseriInt(respBuf);
}

CliStatus KVCCalcTwoNumber(DbConnectT *conn, int x, int y, CalcOptionT opt, int *result)
{
    DB_POINT2(conn, result);
    OperatorCode opCode = OP_ADD_TEST;
    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    msgBuf.opCode = opCode;
    msgBuf.requestBufLen = BUF_SIZE;

    SetCalcUsrMsgBuf(msgBuf.requestMsg, x, y, opt);

    UsrDataCalcT calcRes = {0};
    CliStatus ret = KVCSendRequestAndRecvResponse(conn, &msgBuf, CltParseCalcRspBuf, (UsrDataBaseT *)&calcRes);
    if (calcRes.ret != GMERR_OK)
    {
        log_error("calc error and can not get result, server status = %d", calcRes.ret);
        return GMERR_OK; // 客户端返回成功
    }
    *result = calcRes.calcAns;
    return GMERR_OK;
}