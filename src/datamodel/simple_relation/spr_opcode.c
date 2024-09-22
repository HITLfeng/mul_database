#include "include/spr_common.h"

Status SrCheckCreateDbArgs(SimpleRelExecCtxT *execCtx)
{
    if (execCtx->dbName == NULL || strcmp(execCtx->dbName, "") == 0)
    {
        log_error("SrCheckCreateDbArgs: dbName is null");
        return GMERR_DATAMODEL_SRDB_NAME_NULL;
    }
    if (strlen(execCtx->dbName) > SR_DBNAME_MAX_LENGTH)
    {
        log_error("SrCheckCreateDbArgs: dbName is too long");
        return GMERR_DATAMODEL_SRDB_NAME_TOO_LONG;
    }

    if (IsDbNameExist(execCtx->dbName))
    {
        log_error("SrCheckCreateDbArgs: dbName is exist.");
        return GMERR_DATAMODEL_SRDB_NAME_EXISTED;
    }
    return GMERR_OK;
}

Status SrDmCreateDb(SimpleRelExecCtxT *execCtx)
{
    Status ret = SrCheckCreateDbArgs(execCtx);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    SrDbCtrlT dbCtrl = {0};
    dbCtrl.dbName = (char *)KVMemAlloc(strlen(execCtx->dbName) + 1);
    if (dbCtrl.dbName == NULL)
    {
        log_error("SrDmCreateDb: dbName alloc failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    ret = DbVectorInit(&dbCtrl.labelCtrlList, sizeof(SrLabelCtrlT));
    if (ret != GMERR_OK)
    {
        KVMemFree(dbCtrl.dbName, sizeof(SrLabelCtrlT));
        log_error("SrDmCreateDb: DbVectorInit labelCtrlList failed.");
        return ret;
    }
    dbCtrl.dbId = GenSrDbId();
    execCtx->dbId = dbCtrl.dbId;
    return GMERR_OK;
}

Status DmProcessSimpleRelOpCode(OperatorCode opCode, SimpleRelExecCtxT *execCtx)
{
    switch (opCode)
    {
    case OP_SIMREL_CREATE_DB:
        return SrDmCreateDb(execCtx);
    case OP_SIMREL_DROP_DB:
        break;
    case OP_SIMREL_CREATE_TABLE:
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
