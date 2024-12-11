#include "main_worker.h"
#include "seri_utils.h"
#include "outfunction.h"
#include "ee_out_function.h"

Status RtHandleAddTest(char *usrMsg, char *resultBuf, uint32_t bufLen) {
    // 序列化格式 int32_t int32_t char
    uint8_t **bufCursor = (uint8_t **)&usrMsg;
    int32_t argL = DeseriIntM(bufCursor);
    int32_t argR = DeseriIntM(bufCursor);
    CalcOptionT argOp = DeseriCharM(bufCursor);
    switch (argOp) {
    case CALC_ADD:
        SeriInt32((uint8_t **)&resultBuf, argL + argR);
        break;
    case CALC_SUB:
        SeriInt32((uint8_t **)&resultBuf, argL - argR);
        break;
    case CALC_MUL:
        SeriInt32((uint8_t **)&resultBuf, argL * argR);
        break;
    case CALC_DIV:
        if (argR == 0) {
            log_error("add test invaild option, option is \\ and argR is 0.");
            return GMERR_ADD_TEST_INVAILD_OPTION;
        }
        SeriInt32((uint8_t **)&resultBuf, argL / argR);
        break;
    default:
        log_error("add test invaild option, option is %c", argOp);
        return GMERR_ADD_TEST_INVAILD_OPTION;
    }
    return GMERR_OK;
}

bool IsSimpleRelOpCode(OperatorCode opCode) { return opCode >= OP_SIMREL_CREATE_DB && opCode < OP_SIMREL_BUTT; }
bool IsSysviewOpCode(OperatorCode opCode) { return opCode >= OP_SYSVIEW_EDIT && opCode < OP_SYSVIEW_END; }
void RtSysviewInitExecCtxByOpCode(OperatorCode opCode, char *usrMsg, SysviewEditCtxT *execCtx) {
    uint8_t *bufCursor = (uint8_t *)usrMsg;
    switch (opCode) {
    case OP_SYSVIEW_EDIT:
        execCtx->op = (MemOperatorT)DeseriUint32M(&bufCursor);
        execCtx->allocSize = DeseriUint32M(&bufCursor);
        execCtx->allocTime = DeseriUint32M(&bufCursor);
        break;
    default:
        break;
    }
}

void RtSRInitExecCtxByOpCode(QryStmtT *stmt, char *usrMsg, SimpleRelExecCtxT *execCtx) {
    OperatorCode opCode = stmt->opCode;
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
        execCtx->dbId = DeseriUint32M((uint8_t **)&bufCursor);
        execCtx->labelId = DeseriUint32M((uint8_t **)&bufCursor);
        execCtx->totalFldSize = DeseriUint32M((uint8_t **)&bufCursor);
        execCtx->insertData = DbDynMemCtxAlloc(stmt->memCtx, execCtx->totalFldSize);
        DB_ASSERT(execCtx->insertData != NULL);
        memset(execCtx->insertData, 0x00, execCtx->totalFldSize);
        DeseriFixedStringM((uint8_t **)&bufCursor, execCtx->insertData, execCtx->totalFldSize);
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
    case OP_SIMREL_QUERY:
        execCtx->cond = *(SRCondT *)bufCursor;
        execCtx->dbId = execCtx->cond.dbId;
        execCtx->labelId = execCtx->cond.labelId;
    default:
        break;
    }
    stmt->entry = execCtx;
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

void RtSysviewSetResultBufByOpCode(char *resultBuf, QryStmtT *stmt) {
    char *bufCursor = resultBuf;
    switch (stmt->opCode) {
    case OP_SYSVIEW_EDIT:
        break;
    default:
        break;
    }
    if (stmt->retEntry != NULL) {
        KVMemFree(stmt->retEntry, stmt->retEntryBufLen);
    }
}

void RtSRSetResultBufByOpCode(char *resultBuf, QryStmtT *stmt) {
    char *bufCursor = resultBuf;
    switch (stmt->opCode) {
    case OP_SIMREL_CREATE_DB:
        SeriInt32((uint8_t **)&bufCursor, *(uint32_t *)stmt->retEntry);
        break;
    case OP_SIMREL_DROP_DB:
        break;
    case OP_SIMREL_CREATE_TABLE:
        // labelId
        SeriInt32((uint8_t **)&bufCursor, *(uint32_t *)stmt->retEntry);
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
        DbDynMemCtxFree(stmt->memCtx, stmt->retEntry);
        stmt->retEntry = NULL;
    }
}

void RtInitStmt(QryStmtT *stmt, OperatorCode opCode, void *execCtx) {
    stmt->opCode = opCode;
    stmt->entry = execCtx;
}

Status RtInitQryStmt(OperatorCode opCode, QryStmtT **outStmt) {
    // 从顶层memCtx上申请stmt内存
    DB_POINT(outStmt);
    // 申请新memCtx
    DbMemCtxT *stmtMemCtx = NULL;
    Status ret = DbCreateMemCtx(NULL, "qry_stmt_memctx", &stmtMemCtx);
    if (ret != GMERR_OK) {
        return ret;
    }

    QryStmtT *stmt = (QryStmtT *)DbDynMemCtxAlloc(stmtMemCtx, sizeof(QryStmtT));
    if (stmt == NULL) {
        log_error("alloc qry stmt failed.");
        DbMemCtxDelete(stmtMemCtx);
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }

    stmt->memCtx = stmtMemCtx;
    stmt->opCode = opCode;
    stmt->entry = NULL;
    stmt->retEntry = NULL;
    *outStmt = stmt;
    return GMERR_OK;
}

void RtUninitQryStmt(QryStmtT *stmt) {
    DB_POINT(stmt);
    DbMemCtxT *memCtx = stmt->memCtx;
    DbMemCtxDelete(memCtx);
}

Status RtHandleSimpleRelOpCode(OperatorCode opCode, char *usrMsg, char *resultBuf, uint32_t bufLen) {
    if (!IsSimpleRelOpCode(opCode)) {
        log_error("simple rel op code invaild, opCode is %d", opCode);
        return GMERR_SRDB_OP_CODE_INVAILD;
    }

    // 初始化stmt
    QryStmtT *stmt = NULL;
    Status ret = RtInitQryStmt(opCode, &stmt);

    // simple rel 通用结构体
    SimpleRelExecCtxT execCtx = {0};
    // 根据opCode 解析execCtx
    RtSRInitExecCtxByOpCode(stmt, usrMsg, &execCtx);

    // EE RUNTIME 转向 ee 层处理
    ret = EEProcessRuntimeOpCode(stmt);
    if (ret != GMERR_OK) {
        log_error("process simple rel op code failed, opCode is %d", opCode);
        return ret;
    }

    // Status ret = DmProcessSimpleRelOpCode(opCode, &execCtx);
    // 根据opCode 填写返回结果
    RtSRSetResultBufByOpCode(resultBuf, stmt);
    if (execCtx.insertData != NULL) {
        DbDynMemCtxFree(stmt->memCtx, execCtx.insertData);
    }
    // 释放stmt
    RtUninitQryStmt(stmt);
    return GMERR_OK;
}

Status RtHandleSysViewEdit(OperatorCode opCode, char *usrMsg, char *resultBuf, uint32_t bufLen) {
    if (!IsSysviewOpCode(opCode)) {
        log_error("sys view edit op code invaild, opCode is %d", opCode);
        return GMERR_SRDB_OP_CODE_INVAILD; // TODO: 修改错误码
    }

    QryStmtT *stmt = (QryStmtT *)KVMemAlloc(sizeof(QryStmtT));
    if (stmt == NULL) {
        log_error("alloc qry stmt failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    memset(stmt, 0, sizeof(QryStmtT));

    // sysview 通用结构体
    SysviewEditCtxT execCtx = {0};

    // 根据opCode 解析execCtx
    RtSysviewInitExecCtxByOpCode(opCode, usrMsg, &execCtx);

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
        // 改版后的查询！
    case OP_SIMREL_QUERY:
        return RtHandleSimpleRelOpCode(opCode, usrMsg, resultBuf, bufLen);
    case OP_SYSVIEW_EDIT:
        return RtHandleSysViewEdit(opCode, usrMsg, resultBuf, bufLen);
    // case OP_SYSVIEW_END:
    //     return RtHandleSysViewEnd(usrMsg, resultBuf, bufLen);
    default:
        break;
    }
    return GMERR_OK;
}