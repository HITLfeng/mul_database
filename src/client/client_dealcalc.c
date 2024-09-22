#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

#include "../common/include/seri_utils.h"
// #include "seri_utils.h"

uint8_t *GetUsrDataPosition(uint8_t *usrMsgBuf)
{
    return usrMsgBuf + sizeof(MsgBufResponseHeadT);
}

void SetCalcUsrMsgBuf(char *usrMsgBuf, int x, int y, CalcOptionT opt)
{
    uint8_t *bufCursor = (uint8_t *)usrMsgBuf;
    SeriIntM(&bufCursor, x);
    SeriIntM(&bufCursor, y);
    SeriCharM(&bufCursor, opt);
}



CliStatus CltParseCalcMsgBuf(MsgBufResponseT *respBuf, UsrResultBaseT *result)
{
    DB_POINT2(respBuf, result);
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)respBuf;
    UsrResultCalcT *calcRes = (UsrResultCalcT *)result;
    calcRes->ret = respHead->status;
    uint8_t *bufCursor = GetUsrDataPosition((uint8_t *)respBuf);
    calcRes->calcAns = DeseriInt(&bufCursor);
    return GMERR_OK;
}

CliStatus KVCCalcTwoNumber(KVConnectT *conn, int x, int y, CalcOptionT opt, UsrResultBaseT *result)
{
    DB_POINT2(conn, result);
    OperatorCode opCode = OP_ADD_TEST;
    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    msgBuf.opCode = opCode;
    msgBuf.requestBufLen = BUF_SIZE;

    SetCalcUsrMsgBuf(msgBuf.requestMsg, x, y, opt);

    CliStatus ret = KVCSend(conn, &msgBuf);
    if (ret != GMERR_OK) {
        return ret;
    }
    normal_info("send calc request succ");

    // 读取服务器返回的消息
    MsgBufResponseT respBuf = {0};
    ret = KVCRecv(conn, &respBuf);
    if (ret != GMERR_OK) {
        return ret;
    }
    normal_info("recv calc result succ");
    // 解析服务器返回的消息
    ret = CltParseCalcMsgBuf(&respBuf, result);
    if (ret != GMERR_OK) {
        return ret;
    }
    normal_info("parse calc result succ");

    return GMERR_OK;
}