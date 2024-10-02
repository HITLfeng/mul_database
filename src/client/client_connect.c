#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

#include "client_common.h"

uint8_t *GetUsrDataPosition(uint8_t *usrMsgBuf) { return usrMsgBuf + sizeof(MsgBufResponseHeadT); }

void CltParseBaseMsgBuf(MsgBufResponseT *respBuf, UsrDataBaseT *result) {
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)respBuf;
    result->ret = respHead->status;
    return;
}

void *srvStartDetach(void *arg) { system("kvserver"); }

CliStatus KVCSrvStart(void) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, srvStartDetach, NULL) != 0) {
        log_error("server start thread create error.");
        return GMERR_CLIENT_START_SERVER_FAILED;
    }
    pthread_detach(thread);
}
CliStatus KVCSrvStop(void) { system("killall kvserver"); }

CliStatus KVCConnect(DbConnectT *conn) {
    DB_POINT(conn);
    int sock;
    char message[BUF_SIZE];
    int str_len;

    struct sockaddr_in serv_addr;

    // 创建socket对象
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        log_error("client socket error.");
        return GMERR_CLIENT_CONNECT_FAILED;
    }

    // 设置服务器地址和端口
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(12345);

    // 连接服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        log_error("client connect error.");
        return GMERR_CLIENT_CONNECT_FAILED;
    }
    conn->socketFd = sock;
    log_info("client connect success.");
    return GMERR_OK;
}

CliStatus KVCDisconnect(DbConnectT *conn) {
    DB_POINT(conn);
    close(conn->socketFd);
    return GMERR_OK;
}

CliStatus KVCSend(DbConnectT *conn, const MsgBufRequestT *msgBuf) {
    DB_POINT(conn);
    int sendRet = write(conn->socketFd, (const void *)msgBuf, sizeof(MsgBufRequestT));
    if (sendRet == -1) {
        log_error("client send msgBuf error.");
        return GMERR_CLIENT_SEND_FAILED;
    }
    return GMERR_OK;
}

CliStatus KVCRecv(DbConnectT *conn, MsgBufResponseT *msgBuf) {
    DB_POINT(conn);
    char message[sizeof(MsgBufResponseT)];
    int recvLen = read(conn->socketFd, &message, sizeof(MsgBufResponseT));
    if (recvLen == -1) {
        log_error("client recv msgBuf error.");
        return GMERR_CLIENT_RECV_FAILED;
    }
    // message[recvLen] = 0;
    memcpy(msgBuf, &message, sizeof(MsgBufResponseT));

    return GMERR_OK;
}

CliStatus KVCSendRequestAndRecvResponse(DbConnectT *conn, MsgBufRequestT *msgBuf, SRParseResponseCb cbExec,
                                        UsrDataBaseT *usrData) {
    DB_POINT2(conn, msgBuf);
    OperatorCode opCode = msgBuf->opCode;
    CliStatus ret = KVCSend(conn, msgBuf);
    if (ret != GMERR_OK) {
        return ret;
    }
    log_info("send request succ. type is %u.", opCode);

    // 读取服务器返回的消息
    MsgBufResponseT respBuf = {0};
    ret = KVCRecv(conn, &respBuf);
    if (ret != GMERR_OK) {
        return ret;
    }
    log_info("recv result succ. type is %u.", opCode);

    // 如果不指定回调函数
    if (cbExec == NULL) {
        // 这里解析下服务端返回值，返回值不区分服务端客户端
        UsrDataBaseT result = {0};
        CltParseBaseMsgBuf(&respBuf, &result);
        if (result.ret != GMERR_OK) {
            log_error("CltParseBaseMsgBuf fail, type is %u, ret is %u.", opCode, result.ret);
            return ret;
        }
        log_info("parse result succ, type is %u.", opCode);
        return GMERR_OK;
    }

    // 执行回调函数 拿到用户要解析的数据
    // 服务端返回值解析到usrData 与客户端区分开
    DB_POINT(usrData);
    uint8_t *bufCursor = (uint8_t *)&respBuf;
    cbExec(&bufCursor, usrData);
    return GMERR_OK;
}

void SrParseTable(CliStmtT *stmt, uint8_t **bufCursor) {
    // 首先解析fieldNum
    uint32_t fldNum = DeseriUint32M(bufCursor);
    DB_ASSERT(fldNum > 0);

    // 申请内存
    CliTableSchemaT *tblSchema = (CliTableSchemaT *)malloc(sizeof(CliTableSchemaT));
    DB_ASSERT(tblSchema != NULL); // 将来统一整改
    memset(tblSchema, 0, sizeof(CliTableSchemaT));
    tblSchema->dbId = stmt->dbId;
    tblSchema->labelId = stmt->labelId;
    tblSchema->propertyCnt = fldNum;

    for (uint32_t i = 0; i < fldNum; ++i) {
        CliPropertyT *property = &tblSchema->properties[i];
        memset(property, 0, sizeof(CliPropertyT));
        DeseriStringM(bufCursor, property->fldName);
        property->type = DeseriUint32M(bufCursor);
        property->fldSize = DeseriUint32M(bufCursor);
    }
    stmt->tableSchema = tblSchema;
}

void SrParseQueryTblRspCb(uint8_t **respBuf, UsrDataBaseT *result) {
    DB_POINT2(respBuf, result);
    // 1.解析服务端返回值
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)*respBuf;
    if (respHead->status != GMERR_OK) {
        log_error("SrParseQueryTblRspCb error, server status = %d", respHead->status);
        return;
    }
    // 2.解析数据
    // 服务端返回格式 字段数 | 字段名长度 | 字段名 | 字段类型 | 字段长度 |
    uint8_t *bufCursor = *respBuf;
    bufCursor += sizeof(MsgBufResponseHeadT);
    UsrDataSimpleRelStmtT *srRes = (UsrDataSimpleRelStmtT *)result;
    srRes->ret = respHead->status;
    SrParseTable(srRes->stmt, &bufCursor);
}

CliStatus KVCPrepareStmt(DbConnectT *conn, CliStmtT **stmt, uint32_t dbId, uint32_t labelId) {
    if (*stmt != NULL) {
        return GMERR_OK;
    }
    *stmt = (CliStmtT *)malloc(sizeof(CliStmtT));
    if (*stmt == NULL) {
        log_error("malloc stmt fail. conn is %d.", conn->socketFd);
        return GMERR_CLIENT_MEMORY_ALLOC_FAILED;
    }
    memset(*stmt, 0, sizeof(CliStmtT));
    (*stmt)->conn = conn;
    (*stmt)->dbId = dbId;
    (*stmt)->labelId = labelId;
    (*stmt)->opCode = OP_SIMREL_QUERY_TABLE;

    // 获取当前表的定义缓存
    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_QUERY_TABLE);

    // len/dbname
    char *bufCursor = msgBuf.requestMsg;
    SeriUint32M((uint8_t **)&bufCursor, dbId);
    SeriUint32M((uint8_t **)&bufCursor, labelId);

    UsrDataSimpleRelStmtT queryTblRes = {.ret = GMERR_OK, .stmt = *stmt};

    CliStatus ret = KVCSendRequestAndRecvResponse(conn, &msgBuf, SrParseQueryTblRspCb, (UsrDataBaseT *)&queryTblRes);
    if (ret != GMERR_OK) {
        log_error("CLIENT: create stmt fail.");
        return ret;
    }
    if (queryTblRes.ret != GMERR_OK) {
        log_error("SERVER: create stmt fail, ret is %u.", queryTblRes.ret);
        return GMERR_OK;
    }
    return GMERR_OK;
}

CliStatus KVCReleaseStmt(CliStmtT *stmt) {
    if (stmt == NULL) {
        return GMERR_OK;
    }
    if (stmt->tableSchema != NULL) {
        free(stmt->tableSchema);
    }
    stmt->tableSchema = NULL;
    stmt->conn = NULL;
    stmt->dbId = 0;
    stmt->labelId = 0;
    free(stmt);
}
