#include "include/spr_common.h"

Status DmProcessSimpleRelOpCode(OperatorCode opCode, SimpleRelExecCtxT *execCtx)
{
    switch (opCode)
    {
    case OP_SIMREL_CREATE_DB:
        return SrDmCreateDb(execCtx);
    case OP_SIMREL_DROP_DB:
        return SrDmDropDb(execCtx);
    case OP_SIMREL_CREATE_TABLE:
        // return SrDmCreateTable(execCtx);
    case OP_SIMREL_DROP_TABLE:
        break;
    case OP_SIMREL_INSERT_DATA:
        // return SrDmInsertData(execCtx);
    case OP_SIMREL_DELETE_DATA:
        break;
    case OP_SIMREL_QUERY_DATA:
        break;
    default:
        break;
    }
}
