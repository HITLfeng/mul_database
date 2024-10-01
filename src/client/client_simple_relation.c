#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

#include "../common/include/seri_utils.h"
// #include "seri_utils.h"

#include "include/client_common.h"

// 序列化单段字符串用此接口
void SetSRSetDbUsrMsgBuf(char *usrMsgBuf, const char *buf) {
    DB_POINT2(usrMsgBuf, buf);
    char *bufCursor = usrMsgBuf;
    SeriStringM(&bufCursor, buf);
}

void SetSRSetCreateTableMsgBuf(char *usrMsgBuf, uint32_t dbId, const char *buf) {
    DB_POINT2(usrMsgBuf, buf);
    char *bufCursor = usrMsgBuf;
    SeriUint32M((uint8_t **)&bufCursor, dbId);
    SeriStringM(&bufCursor, buf);
}

CliStatus CltParseCreateDbMsgBuf(MsgBufResponseT *respBuf, UsrDataBaseT *result) {
    DB_POINT2(respBuf, result);
    CltParseBaseMsgBuf(respBuf, result);
    // 解析数据
    UsrDataSimpleRelT *createDbRes = (UsrDataSimpleRelT *)result;
    uint8_t *bufCursor = GetUsrDataPosition((uint8_t *)respBuf);
    createDbRes->srData.dbId = DeseriInt(&bufCursor);
    return GMERR_OK;
}

void SRCInitMsgBuf(MsgBufRequestT *msgBuf, OperatorCode opCode) {
    DB_POINT(msgBuf);
    memset(msgBuf, 0, sizeof(MsgBufRequestT));
    msgBuf->opCode = opCode;
    msgBuf->requestBufLen = BUF_SIZE;
}

// create db 函数

void SrParseCreateDbRspCb(uint8_t **respBuf, UsrDataBaseT *result) {
    DB_POINT2(respBuf, result);
    // 1.解析服务端返回值
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)*respBuf;
    if (respHead->status != GMERR_OK) {
        log_error("SrParseCreateDbRspCb error, server status = %d", respHead->status);
        return;
    }
    // 2.解析数据
    *respBuf += sizeof(MsgBufResponseHeadT);
    UsrDataSimpleRelT *srRes = (UsrDataSimpleRelT *)result;
    srRes->ret = respHead->status;
    srRes->srData.dbId = DeseriInt(respBuf);
}

CliStatus SRCCreateDb(DbConnectT *conn, const char *dbName, uint32_t *dbId) {
    DB_POINT3(conn, dbName, dbId);

    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_CREATE_DB);

    // len/dbname
    SetSRSetDbUsrMsgBuf(msgBuf.requestMsg, dbName);

    UsrDataSimpleRelT createDbRes = {0};

    CliStatus ret = KVCSendRequestAndRecvResponse(conn, &msgBuf, SrParseCreateDbRspCb, (UsrDataBaseT *)&createDbRes);
    if (ret != GMERR_OK) {
        log_error("CLIENT: create db fail.");
        return ret;
    }
    if (createDbRes.ret != GMERR_OK) {
        log_error("SERVER: create db fail, ret is %u.", createDbRes.ret);
        return GMERR_OK;
    }
    *dbId = createDbRes.srData.dbId;
    return GMERR_OK;
}

// drop db 函数 TODO 改为根据DBID删除DB

CliStatus SRCDeleteDb(DbConnectT *conn, const char *dbName) {
    DB_POINT2(conn, dbName);

    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_DROP_DB);

    // len/dbname
    SetSRSetDbUsrMsgBuf(msgBuf.requestMsg, dbName);

    return KVCSendRequestAndRecvResponse(conn, &msgBuf, NULL, NULL);
}

void SrParseCreateTableRspCb(uint8_t **respBuf, UsrDataBaseT *result) {
    DB_POINT2(respBuf, result);
    // 1.解析服务端返回值
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)*respBuf;
    if (respHead->status != GMERR_OK) {
        log_error("SrParseCreateTableRspCb error, server status = %d", respHead->status);
        return;
    }
    // 2.解析数据
    *respBuf += sizeof(MsgBufResponseHeadT);
    UsrDataSimpleRelT *srRes = (UsrDataSimpleRelT *)result;
    srRes->ret = respHead->status;
    srRes->srData.labelId = DeseriInt(respBuf);
}

CliStatus SRCCreateLabelWithJson(DbConnectT *conn, uint32_t dbId, const char *labelJson, uint32_t *labelId) {
    DB_POINT2(conn, labelJson);

    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_CREATE_TABLE);

    // len/dbname
    SetSRSetCreateTableMsgBuf(msgBuf.requestMsg, dbId, labelJson);

    UsrDataSimpleRelT createTableRes = {0};
    CliStatus ret =
        KVCSendRequestAndRecvResponse(conn, &msgBuf, SrParseCreateTableRspCb, (UsrDataBaseT *)&createTableRes);
    if (ret != GMERR_OK) {
        log_error("CLIENT: create table fail.");
        return ret;
    }
    if (createTableRes.ret != GMERR_OK) {
        log_error("SERVER: create table fail, ret is %u.", createTableRes.ret);
        return GMERR_OK;
    }
    *labelId = createTableRes.srData.labelId;
    return GMERR_OK;
}

// DFX 关系表
CliStatus SRCTraceDbDesc(DbConnectT *conn, uint32_t dbId) {
    DB_POINT(conn);

    // 初始化 requestHeader
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_DFX_DB_DESC);

    // 序列化 msgBuf.requestMsg
    char *bufCursor = msgBuf.requestMsg;
    SeriUint32((uint8_t **)&bufCursor, dbId);

    // 客户端服务端错误码混合返回
    return KVCSendRequestAndRecvResponse(conn, &msgBuf, NULL, NULL);
}

// 2024/9/30
// 客户端函数统一重构整改
// 1.初始化requestHeader
// 2.序列化msgBuf
// 3.发送并且等待响应 调用回调函数解析响应