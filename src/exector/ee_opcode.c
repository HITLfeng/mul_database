#include "ee_out_function.h"
#include "spr_common.h"

static bool EEIsSimpleRelOpCode(OperatorCode opCode) {
    return opCode >= OP_SIMREL_CREATE_DB && opCode < OP_SIMREL_BUTT;
}

Status EEProcessSimpleRelationOpcode(QryStmtT *stmt) {
    switch (stmt->opCode) {
    case OP_SIMREL_CREATE_DB:
        // 返回报文包含DBId
        return DMSrCreateDb(stmt);
    case OP_SIMREL_DROP_DB:
        return DMSrDropDb(stmt);
    case OP_SIMREL_CREATE_TABLE:
        // 返回报文包含labelId
        return DMSrCreateTable(stmt);
    case OP_SIMREL_DROP_TABLE:
        break;
    case OP_SIMREL_INSERT_DATA:
        // return DMSrInsertData(stmt);
    case OP_SIMREL_DELETE_DATA:
        break;
    case OP_SIMREL_QUERY_DATA:
        break;
    case OP_SIMREL_QUERY_TABLE:
        return DMSrQueryTable(stmt);
        break;
    case OP_SIMREL_DFX_DB_DESC:
        return DMSrGetDbDesc(stmt);
    default:
        break;
    }
}

Status EEProcessRuntimeOpCode(QryStmtT *stmt) {
    OperatorCode opCode = stmt->opCode;
    if (EEIsSimpleRelOpCode(opCode)) {
        return EEProcessSimpleRelationOpcode(stmt);
    } else {
        // 待拓展
    }
    log_error("EE process runtime opcode %d error.", opCode);
    return GMERR_EE_UNKNOWN_OPCODE;
}