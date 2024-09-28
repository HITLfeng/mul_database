#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

#include "../common/include/seri_utils.h"
// #include "seri_utils.h"

#include "include/client_common.h"

void SetSRSetDbUsrMsgBuf(char *usrMsgBuf, const char *buf)
{
    DB_POINT2(usrMsgBuf, buf);
    char *bufCursor = usrMsgBuf;
    SeriStringM(&bufCursor, buf);
}

void SetSRSetCreateTableMsgBuf(char *usrMsgBuf, uint32_t dbId, const char *buf)
{
    DB_POINT2(usrMsgBuf, buf);
    char *bufCursor = usrMsgBuf;
    SeriUint32M((uint8_t **)&bufCursor, dbId);
    SeriStringM(&bufCursor, buf);
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
    SetSRSetDbUsrMsgBuf(msgBuf.requestMsg, dbName);

    CliStatus ret = KVCSend(conn, &msgBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    log_info("send calc request succ SRCCreateDb.");

    // 读取服务器返回的消息
    MsgBufResponseT respBuf = {0};
    ret = KVCRecv(conn, &respBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    log_info("recv calc result succ SRCCreateDb.");
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
    log_info("parse calc result succ");
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
    SetSRSetDbUsrMsgBuf(msgBuf.requestMsg, dbName);

    CliStatus ret = KVCSend(conn, &msgBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    log_info("send SRCDeleteDb request succ SRCDeleteDb.");

    // 读取服务器返回的消息
    MsgBufResponseT respBuf = {0};
    ret = KVCRecv(conn, &respBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    log_info("recv SRCDeleteDb result succ SRCDeleteDb.");
    // 解析服务器返回的消息
    UsrResultBaseT result = {0};
    CltParseBaseMsgBuf(&respBuf, &result);
    if (result.ret != GMERR_OK)
    {
        log_error("drop db fail, ret is %u.", result.ret);
        return ret;
    }
    log_info("parse SRCDeleteDb result succ");
    return GMERR_OK;
}

CliStatus SRCCreateLabelWithJson(DbConnectT *conn, uint32_t dbId, const char *labelJson)
{
    DB_POINT2(conn, labelJson);
    OperatorCode opCode = OP_SIMREL_CREATE_TABLE;

    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    msgBuf.opCode = opCode;
    msgBuf.requestBufLen = BUF_SIZE;

    // len/dbname
    SetSRSetCreateTableMsgBuf(msgBuf.requestMsg, dbId, labelJson);

    CliStatus ret = KVCSend(conn, &msgBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    log_info("send SRCCreateLabelWithJson request succ.");

    // 读取服务器返回的消息
    MsgBufResponseT respBuf = {0};
    ret = KVCRecv(conn, &respBuf);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    log_info("recv SRCCreateLabelWithJson result succ.");
    // 解析服务器返回的消息
    UsrResultBaseT result = {0};
    CltParseBaseMsgBuf(&respBuf, &result);
    if (result.ret != GMERR_OK)
    {
        log_error("SRCCreateLabelWithJson fail, ret is %u.", result.ret);
        return ret;
    }
    log_info("parse SRCCreateLabelWithJson result succ");
    return GMERR_OK;
}
