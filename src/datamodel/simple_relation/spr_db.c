#include "include/spr_common.h"

void DmClearSingleDbCtrl(SrDbCtrlT *dbCtrl)
{
    if (dbCtrl->dbName != NULL)
    {
        KVMemFree(dbCtrl->dbName, strlen(dbCtrl->dbName) + 1);
    }
    // TODO:
    DmClearAllLabels(dbCtrl);
    DbVectorDestroy(&dbCtrl->labelCtrlList);
    dbCtrl->dbId = 0;
    dbCtrl->dbName = NULL;
}

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
    return GMERR_OK;
}

Status SrDmCreateDb(SimpleRelExecCtxT *execCtx)
{
    Status ret = SrCheckCreateDbArgs(execCtx);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    if (IsDbNameExist(execCtx->dbName))
    {
        log_error("SrDmCreateDb: dbName is exist.");
        return GMERR_DATAMODEL_SRDB_NAME_EXISTED;
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

Status SrDmDropDb(SimpleRelExecCtxT *execCtx)
{
    Status ret = SrCheckCreateDbArgs(execCtx);
    if (ret != GMERR_OK)
    {
        return ret;
    }
    if (!IsDbNameExist(execCtx->dbName))
    {
        warn_error("SrDmDropDb: dbName is not exist.");
        return GMERR_DATAMODEL_SRDB_NAME_NOT_EXISTED;
    }
    SrDbCtrlT *dbCtrl = DmGetDbCtrlByName(execCtx->dbName);
    if (dbCtrl == NULL) {
        log_error("SrDmDropDb: DmGetDbCtrlByName failed.");
        return GMERR_DATAMODEL_SRDB_LIST_EXCEPT_NULL;
    }
    DmClearSingleDbCtrl(dbCtrl);
    return RemoveDbCtrlByName(execCtx->dbName);
}