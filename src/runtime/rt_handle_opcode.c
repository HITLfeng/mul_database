#include "main_worker.h"
#include "seri_utils.h"
#include "outfunction.h"
// #include "spr_common.h"

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
            error_info("add test invaild option, option is \\ and argR is 0.");
            return GMERR_ADD_TEST_INVAILD_OPTION;
        }
        SeriInt((uint8_t **)&resultBuf, argL / argR);
        break;
    default:
        error_info("add test invaild option, option is %c", argOp);
        return GMERR_ADD_TEST_INVAILD_OPTION;
    }
    return GMERR_OK;
}

bool IsSimpleRelOpCode(OperatorCode opCode) {
    return opCode >= OP_SIMREL_CREATE_DB && opCode <= OP_SIMREL_QUERY_DATA;
}

void RtSRInitExecCtxByOpCode(OperatorCode opCode, char *usrMsg,
                             SimpleRelExecCtxT *execCtx) {
    switch (opCode) {
    case OP_SIMREL_CREATE_DB:
        execCtx->opCode = OP_SIMREL_CREATE_DB;
        DeseriString((uint8_t **)&usrMsg, execCtx->dbName);
        break;
    case OP_SIMREL_DROP_DB:
        execCtx->opCode = OP_SIMREL_DROP_DB;
        DeseriString((uint8_t **)&usrMsg, execCtx->dbName);
        break;
    case OP_SIMREL_CREATE_TABLE:
        // 填充 labelJson
        execCtx->opCode = OP_SIMREL_CREATE_TABLE;
        char *bufCursor = usrMsg;
        execCtx->dbId = DeseriUint32M((uint8_t **)&bufCursor);
        DeseriString((uint8_t **)&bufCursor, execCtx->labelJson);
        break;
    case OP_SIMREL_DROP_TABLE:
        execCtx->opCode = OP_SIMREL_DROP_TABLE;
        break;
    case OP_SIMREL_INSERT_DATA:
        execCtx->opCode = OP_SIMREL_INSERT_DATA;
        break;
    case OP_SIMREL_DELETE_DATA:
        execCtx->opCode = OP_SIMREL_DELETE_DATA;
        break;
    case OP_SIMREL_QUERY_DATA:
        execCtx->opCode = OP_SIMREL_QUERY_DATA;
        break;
    default:
        break;
    }
}

void RtSRSetResultBufByOpCode(OperatorCode opCode, char *resultBuf,
                              SimpleRelExecCtxT *execCtx) {
    switch (opCode) {
    case OP_SIMREL_CREATE_DB:
        SeriInt((uint8_t **)&resultBuf, execCtx->dbId);
        break;
    case OP_SIMREL_DROP_DB:
        break;
    case OP_SIMREL_CREATE_TABLE:
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
    default:
        break;
    }
}

Status RtHandleSimpleRelOpCode(OperatorCode opCode, char *usrMsg,
                               char *resultBuf, uint32_t bufLen) {
    if (!IsSimpleRelOpCode(opCode)) {
        error_info("simple rel op code invaild, opCode is %d", opCode);
        return GMERR_SRDB_OP_CODE_INVAILD;
    }
    SimpleRelExecCtxT execCtx = {0};

    // 根据opCode 解析execCtx
    RtSRInitExecCtxByOpCode(opCode, usrMsg, &execCtx);
    Status ret = DmProcessSimpleRelOpCode(opCode, &execCtx);
    if (ret != GMERR_OK) {
        error_info("process simple rel op code failed, opCode is %d", opCode);
        return ret;
    }
    // 根据opCode 填写返回结果
    RtSRSetResultBufByOpCode(opCode, resultBuf, &execCtx);
    return GMERR_OK;
}

Status RTProcessOpcode(OperatorCode opCode, char *usrMsg, char *resultBuf,
                       uint32_t bufLen) {
    switch (opCode) {
    case OP_ADD_TEST:
        return RtHandleAddTest(usrMsg, resultBuf, bufLen);
    case OP_SIMREL_CREATE_DB:
    case OP_SIMREL_DROP_DB:
    case OP_SIMREL_CREATE_TABLE:
    case OP_SIMREL_DROP_TABLE:
    case OP_SIMREL_INSERT_DATA:
    case OP_SIMREL_DELETE_DATA:
    case OP_SIMREL_QUERY_DATA:
        // 只需要DbName的走这个分支

        return RtHandleSimpleRelOpCode(opCode, usrMsg, resultBuf, bufLen);
    default:
        break;
    }
    return GMERR_OK;
}