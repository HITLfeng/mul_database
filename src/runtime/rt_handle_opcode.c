#include "main_worker.h"
#include "seri_utils.h"
#include "outfunction.h"
// #include "spr_common.h"
#include "ee_out_function.h"

Status RtHandleAddTest(char *usrMsg, char *resultBuf, uint32_t bufLen) {
    // 序列化格式 int32_t int32_t char
    uint8_t **bufCursor = (uint8_t **)&usrMsg;
    int32_t argL = DeseriIntM(bufCursor);
    int32_t argR = DeseriIntM(bufCursor);
    CalcOptionT argOp = DeseriCharM(bufCursor);
    switch (argOp) {
    case CALC_ADD:
        SeriInt((uint8_t **)&resultBuf, argL + argR);
        break;
    case CALC_SUB:
        SeriInt((uint8_t **)&resultBuf, argL - argR);
        break;
    case CALC_MUL:
        SeriInt((uint8_t **)&resultBuf, argL * argR);
        break;
    case CALC_DIV:
        if (argR == 0) {
            log_error("add test invaild option, option is \\ and argR is 0.");
            return GMERR_ADD_TEST_INVAILD_OPTION;
        }
        SeriInt((uint8_t **)&resultBuf, argL / argR);
        break;
    default:
        log_error("add test invaild option, option is %c", argOp);
        return GMERR_ADD_TEST_INVAILD_OPTION;
    }
    return GMERR_OK;
}

bool IsSimpleRelOpCode(OperatorCode opCode) { return opCode >= OP_SIMREL_CREATE_DB && opCode < OP_SIMREL_BUTT; }

void RtSRInitExecCtxByOpCode(OperatorCode opCode, char *usrMsg, SimpleRelExecCtxT *execCtx) {
    char *bufCursor = usrMsg;
    switch (opCode) {
    case OP_SIMREL_CREATE_DB:
        DeseriString((uint8_t **)&usrMsg, execCtx->dbName);
        break;
    case OP_SIMREL_DROP_DB:
        DeseriString((uint8_t **)&usrMsg, execCtx->dbName);
        break;
    case OP_SIMREL_CREATE_TABLE:
        // 填充 labelJson
        execCtx->dbId = DeseriUint32M((uint8_t **)&bufCursor);
        DeseriString((uint8_t **)&bufCursor, execCtx->labelJson);
        break;
    case OP_SIMREL_DROP_TABLE:
        break;
    case OP_SIMREL_INSERT_DATA:
        break;
    case OP_SIMREL_DELETE_DATA:
        break;
    case OP_SIMREL_QUERY_DATA:
        break;
    case OP_SIMREL_QUERY_TABLE:
        execCtx->dbId = DeseriUint32M((uint8_t **)&bufCursor);
        execCtx->labelId = DeseriUint32M((uint8_t **)&bufCursor);
        break;
    case OP_SIMREL_DFX_DB_DESC:
        execCtx->dbId = DeseriUint32M((uint8_t **)&bufCursor);
        break;
    default:
        break;
    }
}

void RtSeriTable(uint8_t **bufCursor, QryStmtT *stmt) {
    SeriUint32M(bufCursor, stmt->currLabelFldCnt);
    // printf("label field count is %d\n", stmt->currLabelFldCnt);
    SrPropertyT *properties = (SrPropertyT *)stmt->retEntry;
    for (int i = 0; i < stmt->currLabelFldCnt; i++) {
        SrPropertyT *property = &properties[i];
        SeriStringM((char **)bufCursor, property->fieldName);
        // printf("field name is %s\n", property->fieldName);
        SeriUint32M(bufCursor, (uint32_t)property->fieldType);
        // printf("field type is %d\n", (uint32_t)property->fieldType);
        SeriUint32M(bufCursor, property->fieldSize);
        // printf("field size is %d\n", property->fieldSize);
    }
}

void RtSRSetResultBufByOpCode(char *resultBuf, QryStmtT *stmt) {
    char *bufCursor = resultBuf;
    switch (stmt->opCode) {
    case OP_SIMREL_CREATE_DB:
        SeriInt((uint8_t **)&bufCursor, *(uint32_t *)stmt->retEntry);
        break;
    case OP_SIMREL_DROP_DB:
        break;
    case OP_SIMREL_CREATE_TABLE:
        // labelId
        SeriInt((uint8_t **)&bufCursor, *(uint32_t *)stmt->retEntry);
        // 填充 labelJson
        break;
    case OP_SIMREL_DROP_TABLE:
        break;
    case OP_SIMREL_INSERT_DATA:
        break;
    case OP_SIMREL_DELETE_DATA:
        break;
    case OP_SIMREL_QUERY_DATA:
        break;
    case OP_SIMREL_QUERY_TABLE:
        RtSeriTable((uint8_t **)&bufCursor, stmt);
        break;
    case OP_SIMREL_DFX_DB_DESC:
        break;
    default:
        break;
    }
    if (stmt->retEntry != NULL) {
        KVMemFree(stmt->retEntry, stmt->retEntryBufLen);
    }
}

void RtInitStmt(QryStmtT *stmt, OperatorCode opCode, void *execCtx) {
    stmt->opCode = opCode;
    stmt->entry = execCtx;
}

Status RtHandleSimpleRelOpCode(OperatorCode opCode, char *usrMsg, char *resultBuf, uint32_t bufLen) {
    if (!IsSimpleRelOpCode(opCode)) {
        log_error("simple rel op code invaild, opCode is %d", opCode);
        return GMERR_SRDB_OP_CODE_INVAILD;
    }

    QryStmtT *stmt = (QryStmtT *)KVMemAlloc(sizeof(QryStmtT));
    if (stmt == NULL) {
        log_error("alloc qry stmt failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    memset(stmt, 0, sizeof(QryStmtT));

    // simple rel 通用结构体
    SimpleRelExecCtxT execCtx = {0};

    // 根据opCode 解析execCtx
    RtSRInitExecCtxByOpCode(opCode, usrMsg, &execCtx);

    RtInitStmt(stmt, opCode, (void *)&execCtx);

    Status ret = EEProcessRuntimeOpCode(stmt);
    if (ret != GMERR_OK) {
        log_error("process simple rel op code failed, opCode is %d", opCode);
        return ret;
    }

    // Status ret = DmProcessSimpleRelOpCode(opCode, &execCtx);
    // 根据opCode 填写返回结果
    RtSRSetResultBufByOpCode(resultBuf, stmt);
    KVMemFree(stmt, sizeof(QryStmtT));
    return GMERR_OK;
}

// RUNTIME 模块完成 报文解析 与 报文回填

Status RTProcessOpcode(OperatorCode opCode, char *usrMsg, char *resultBuf, uint32_t bufLen) {
    switch (opCode) {
    case OP_ADD_TEST:
        return RtHandleAddTest(usrMsg, resultBuf, bufLen);
    // SIMPLERELATION 入口函数，内部申请stmt
    case OP_SIMREL_CREATE_DB:
    case OP_SIMREL_DROP_DB:
    case OP_SIMREL_CREATE_TABLE:
    case OP_SIMREL_DROP_TABLE:
    case OP_SIMREL_INSERT_DATA:
    case OP_SIMREL_DELETE_DATA:
    case OP_SIMREL_QUERY_DATA:
    case OP_SIMREL_QUERY_TABLE:
    case OP_SIMREL_DFX_DB_DESC:
        return RtHandleSimpleRelOpCode(opCode, usrMsg, resultBuf, bufLen);
    default:
        break;
    }
    return GMERR_OK;
}