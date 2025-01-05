#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

#include "../common/include/seri_utils.h"
// #include "seri_utils.h"

#include "include/client_common.h"

// ********************************************
// *******  简单关系表 相关接口 对外提供    *******
// ********************************************

void SRCSeriRequsetBuf(uint8_t **bufCursor, const uint8_t *buf, uint32_t bufLen) {
    memcpy(*bufCursor, buf, bufLen);
    *bufCursor += bufLen;
}

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

// 当前支持的数据类型 int32_t uint32_t string(char *)

static uint32_t GetRecordTotalLen(CliTableSchemaT *tableSchema) {
    uint32_t totalLen = 0;
    for (uint32_t i = 0; i < tableSchema->propertyCnt; ++i) {
        totalLen += tableSchema->properties[i].fldSize;
    }
    return totalLen;
}

void SRCSeriInsertRequsetBuf(uint8_t **bufCursor, CliStmtT *stmt) {
    // 序列化DBID LABELID 记录总长度
    SeriUint32M(bufCursor, stmt->dbId);
    SeriUint32M(bufCursor, stmt->labelId);
    uint32_t recordTotalLen = GetRecordTotalLen(stmt->tableSchema);
    SeriUint32M(bufCursor, recordTotalLen);
    CliTableSchemaT *tableSchema = stmt->tableSchema;
    for (uint32_t i = 0; i < tableSchema->propertyCnt; ++i) {
        if (tableSchema->properties[i].type == SR_LABEL_FILED_TYPE_INT32) {
            // SeriInt32M(bufCursor, tableSchema->properties[i].value);
            SeriFixedStringM(bufCursor, tableSchema->properties[i].value, tableSchema->properties[i].fldSize);
        } else if (tableSchema->properties[i].type == SR_LABEL_FILED_TYPE_UINT32) {
            // SeriUint32M(bufCursor, tableSchema->properties[i].value);
            SeriFixedStringM(bufCursor, tableSchema->properties[i].value, tableSchema->properties[i].fldSize);
        } else if (tableSchema->properties[i].type == SR_LABEL_FILED_TYPE_STRING) {
            // SeriStringM((uint8_t **)bufCursor, tableSchema->properties[i].value); // xiugai 根据固定长度
            // 为定长数据定制，不序列化长度
            SeriFixedStringM(bufCursor, tableSchema->properties[i].value, tableSchema->properties[i].fldSize);
        } else {
            log_error("CLIENT: SRCSeriInsertRequsetBuf:not support type. type = %d", tableSchema->properties[i].type);
            return;
        }
    }
}

// 这个函数难度较高
CliStatus SRCInsertData(CliStmtT *stmt, ...) {
    DB_POINT2(stmt, stmt->tableSchema);
    va_list args;
    va_start(args, stmt); // 从可变参数列表中获取第一个参数 也就是stmt之后
    CliTableSchemaT *tableSchema = stmt->tableSchema;
    for (uint32_t i = 0; i < tableSchema->propertyCnt; ++i) {
        // 用户自己保证传入的内容对的上，否则后果不可预期
        if (tableSchema->properties[i].type == SR_LABEL_FILED_TYPE_INT32) {
            int32_t value = va_arg(args, int32_t);
            char *pIntBuf = tableSchema->properties[i].value;
            SeriInt32((uint8_t **)&pIntBuf, value);
        } else if (tableSchema->properties[i].type == SR_LABEL_FILED_TYPE_UINT32) {
            uint32_t value = va_arg(args, uint32_t);
            char *pUintBuf = tableSchema->properties[i].value;
            SeriUint32((uint8_t **)&pUintBuf, value);
        } else if (tableSchema->properties[i].type == SR_LABEL_FILED_TYPE_STRING) {
            const char *value = va_arg(args, char *);
            if (STRLEN(value) > SR_FIELD_VALUE_MAX_LENGTH) {
                log_error("CLIENT: SRCInsertData: string value too long. value = %s, fieldName = %s", value,
                          tableSchema->properties[i].fldName);
                return GMERR_OK;
            }
            strcpy(tableSchema->properties[i].value, value);
        } else {
            log_error("CLIENT: SRCInsertData:not support type. type = %d", tableSchema->properties[i].type);
            return GMERR_OK;
        }
    }
    va_end(args);

    // 初始化 requestHeader
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_INSERT_DATA);

    // 序列化 msgBuf.requestMsg
    char *bufCursor = msgBuf.requestMsg;
    SRCSeriInsertRequsetBuf((uint8_t **)&bufCursor, stmt);

    // 客户端服务端错误码混合返回
    return KVCSendRequestAndRecvResponse(stmt->conn, &msgBuf, NULL, NULL);
}

/**
 * 所有对外接口统一重构！
 */
/*
Status SrcOutFunc(DbConnectT *conn, ...) {
    // 1. 设置opcode
    // 2. 根据opCode 填充基础信息
    // 3. 定制填充msg特殊信息
    // 4. 发送和接受消息
    // 5. 触发自动回调解析 服务端 status
    // 6. 解析服务端返回的其他特殊信息
    // 7. 返回结果
 }
 */

#define SR_CURR_OPERATION_COUNT 3

#define SR_KEY_MAX_LENGTH 128
#define SR_KEY_VALUE_LENGTH 1024

/**
 * @brief Sets the value of a DbValue structure based on the provided type and value.
 *
 * This function takes a pointer to a DbValue structure, a FieldTypeT indicating the type of the value,
 * and a char pointer to the value itself. It then assigns the appropriate value to the DbValue structure
 * based on the type. If an unsupported type is provided, it asserts false.
 *
 * @param DbValue Pointer to the DbValue structure to be modified.
 * @param type FieldTypeT indicating the type of the value.
 * @param value Char pointer to the value to be set.
 */
void SetDbValue(DbValueT *DbValue, FiledTypeT type, char *value) {
    switch (type) {
    case SR_LABEL_FILED_TYPE_INT32:
        DbValue->type = SR_LABEL_FILED_TYPE_INT32;
        DbValue->value.int32 = atoi(value);
        break;
    case SR_LABEL_FILED_TYPE_UINT32:
        DbValue->type = SR_LABEL_FILED_TYPE_UINT32;
        DbValue->value.uint32 = atoi(value);
        break;
    case SR_LABEL_FILED_TYPE_STRING:
        DbValue->type = SR_LABEL_FILED_TYPE_STRING;
        strcpy(DbValue->value.strings.str, value);
        break;
    default:
        DB_ASSERT(false);
    }
}

void SplitWithoutSpace(const char *str, char *key, char *value, const char *op) {
    char *sign = strstr(str, op); // 查找分隔符位置
    if (sign == NULL) {
        DB_ASSERT(false);
    }
    // name >= abc123
    //      sign
    uint32_t leftLen = sign - str;
    uint32_t strCursor = 0;
    for (uint32_t i = 0; i < leftLen; ++i) {
        if (str[i] != ' ') {
            key[strCursor++] = str[i];
        }
    }
    key[strCursor] = '\0';
    strCursor = 0;
    for (uint32_t i = leftLen + strlen(op); i < strlen(str); ++i) {
        if (str[i] != ' ') {
            value[strCursor++] = str[i];
        }
    }
    value[strCursor] = '\0';
}

void SetCondition(CliStmtT *stmt, SRCondT *cond, char *key, char *value, SRCondCmpT condCmp) {
    CliTableSchemaT *tableSchema = stmt->tableSchema;
    // 根据 key 寻找对应的 property 下标
    uint32_t propertyCnt = tableSchema->propertyCnt;
    for (uint32_t i = 0; i < propertyCnt; ++i) {
        if (strcmp(tableSchema->properties[i].fldName, key) == 0) {
            cond->fldIdx = i;
            cond->cmpType = condCmp;
            SetDbValue(&cond->dbValue, tableSchema->properties[i].type, value);
            cond->dbId = stmt->dbId;
            cond->labelId = stmt->labelId;
            return;
        }
    }
    DB_ASSERT(false);
    // 这里应该报错返回
}

void PrepareCondition(CliStmtT *stmt, const char *condition, SRCondT *cond) {
    if (condition == NULL || strcmp(condition, "") == 0) {
        cond->cmpType = OP_CMP_NULL;
        cond->dbId = stmt->dbId;
        cond->labelId = stmt->labelId;
        return;
    }
    const char *op[SR_CURR_OPERATION_COUNT] = {">", "=", "<"};
    for (uint32_t i = 0; i < SR_CURR_OPERATION_COUNT; ++i) {
        char *sign = strstr(condition, op[i]);
        if (sign != NULL) {
            // cond->cmpType = i;
            char key[SR_KEY_MAX_LENGTH] = {0};
            char value[SR_KEY_VALUE_LENGTH] = {0};
            SplitWithoutSpace(condition, key, value, op[i]);
            SetCondition(stmt, cond, key, value, i);
            return;
        }
    }
    DB_ASSERT(false);
}

// 20241127
// 目前只支持一个查询条件 > = <
void SRParseQueryData(uint8_t **respBuf, UsrDataBaseT *result) {
    DB_POINT2(respBuf, result);
    // 1.解析服务端返回值
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)*respBuf;
    if (respHead->status != GMERR_OK) {
        log_error("SrParseCreateTableRspCb error, server status = %d", respHead->status);
        return;
    }
    // 2.解析数据
    *respBuf += sizeof(MsgBufResponseHeadT);
    UsrDataSimpleRelQueryT *srRes = (UsrDataSimpleRelQueryT *)result;

    srRes->ret = respHead->status;
    srRes->fetchCnt = DeseriUint32M(respBuf);
    srRes->fetchBufLen = DeseriUint32M(respBuf);
    DB_ASSERT(srRes->fetchBufLen <= BUF_SIZE);
    memcpy(srRes->fetchBuf, *respBuf, srRes->fetchBufLen);
}

CliStatus SRCQueryDataWithCond(CliStmtT *stmt, const char *conditionStr) {
    DB_POINT(stmt);
    SRCondT cond = {0};
    PrepareCondition(stmt, conditionStr, &cond);
    // 初始化 requestHeader
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_QUERY);

    // 序列化 msgBuf.requestMsg
    char *bufCursor = msgBuf.requestMsg;
    SRCSeriRequsetBuf((uint8_t **)&bufCursor, (void *)&cond, sizeof(cond));
    UsrDataSimpleRelQueryT queryData = {0};

    // 客户端服务端错误码混合返回
    Status ret = KVCSendRequestAndRecvResponse(stmt->conn, &msgBuf, SRParseQueryData, (UsrDataBaseT *)&queryData);
    if (ret != GMERR_OK) {
        log_error("CLIENT: send request and get response fail.");
        return ret;
    }
    if (queryData.ret != GMERR_OK) {
        log_error("SERVER: query data fail, ret is %u.", queryData.ret);
        return GMERR_OK;
    }
    // 客户端根据buf打印数据
    CliTraceQueryData(stmt->tableSchema, &queryData, conditionStr);
    return GMERR_OK;
}

// 202412141000
CliStatus SRCDeleteDataWithCond(CliStmtT *stmt, const char *conditionStr) {
    DB_POINT(stmt);
    SRCondT cond = {0};
    PrepareCondition(stmt, conditionStr, &cond);
    // 初始化 requestHeader
    MsgBufRequestT msgBuf = {0};
    SRCInitMsgBuf(&msgBuf, OP_SIMREL_DELETE);

    // 序列化 msgBuf.requestMsg
    char *bufCursor = msgBuf.requestMsg;
    SRCSeriRequsetBuf((uint8_t **)&bufCursor, (void *)&cond, sizeof(cond));

    // 客户端服务端错误码混合返回
    return KVCSendRequestAndRecvResponse(stmt->conn, &msgBuf, NULL, NULL);
}