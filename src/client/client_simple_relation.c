#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

#include "../common/include/seri_utils.h"
// #include "seri_utils.h"

#include "include/client_common.h"

void SetSRCreateOrDropDbUsrMsgBuf(char *usrMsgBuf, const char *dbName)
{
    SeriStringM(&usrMsgBuf, dbName);
}

void CltParseBaseMsgBuf(MsgBufResponseT *respBuf, UsrResultBaseT *result)
{
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)respBuf;
    result->ret = respHead->status;
    return;
}

CliStatus CltParseCreateDbMsgBuf(MsgBufResponseT *respBuf, UsrResultBaseT *result)
{
    DB_POINT2(respBuf, result);
    CltParseBaseMsgBuf(respBuf, result);
    // 解析数据
    UsrResultCreateDbT *createDbRes = (UsrResultCreateDbT *)result;
    uint8_t *bufCursor = GetUsrDataPosition((uint8_t *)respBuf);
    createDbRes->dbId = DeseriInt(&bufCursor);
    return GMERR_OK;
}

CliStatus SRCCreateDb(DbConnectT *conn, const char *dbName, uint32_t *dbId)
{
    DB_POINT3(conn, dbName, dbId);
    OperatorCode opCode = OP_SIMREL_CREATE_DB;

    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    msgBuf.opCode = opCode;
    msgBuf.requestBufLen = BUF_SIZE;

    // len/dbname
    SetSRCreateOrDropDbUsrMsgBuf(msgBuf.requestMsg, dbName);

    CliStatus ret = KVCSend(conn, &msgBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    normal_info("send calc request succ SRCCreateDb.");

    // 读取服务器返回的消息
    MsgBufResponseT respBuf = {0};
    ret = KVCRecv(conn, &respBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    normal_info("recv calc result succ SRCCreateDb.");
    // 解析服务器返回的消息
    UsrResultCreateDbT createDbRes = {0};
    UsrResultBaseT *result = (UsrResultBaseT *)&createDbRes;
    ret = CltParseCreateDbMsgBuf(&respBuf, result);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    if (createDbRes.ret != GMERR_OK)
    {
        log_error("create db fail, ret is %u.", createDbRes.ret);
        return ret;
    }
    normal_info("parse calc result succ");
    *dbId = createDbRes.dbId;
    return GMERR_OK;
}

CliStatus SRCDeleteDb(DbConnectT *conn, const char *dbName)
{
    DB_POINT2(conn, dbName);
    OperatorCode opCode = OP_SIMREL_DROP_DB;

    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    msgBuf.opCode = opCode;
    msgBuf.requestBufLen = BUF_SIZE;

    // len/dbname
    SetSRCreateOrDropDbUsrMsgBuf(msgBuf.requestMsg, dbName);

    CliStatus ret = KVCSend(conn, &msgBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    normal_info("send calc request succ SRCDeleteDb.");

    // 读取服务器返回的消息
    MsgBufResponseT respBuf = {0};
    ret = KVCRecv(conn, &respBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    normal_info("recv calc result succ SRCDeleteDb.");
    // 解析服务器返回的消息
    UsrResultBaseT result = {0};
    CltParseBaseMsgBuf(&respBuf, &result);
    if (result.ret != GMERR_OK)
    {
        log_error("drop db fail, ret is %u.", result.ret);
        return ret;
    }
    normal_info("parse calc result succ");
    return GMERR_OK;
}