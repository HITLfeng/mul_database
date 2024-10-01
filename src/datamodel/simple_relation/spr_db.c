#include "include/spr_common.h"

// void DmClearSingleDbCtrl(QryStmtT *stmt)
// {
//     if (dbCtrl->dbName != NULL)
//     {
//         KVMemFree(dbCtrl->dbName, strlen(dbCtrl->dbName) + 1);
//     }
//     // TODO:
//     DmClearAllLabels(dbCtrl);
//     DbVectorDestroy(&dbCtrl->labelCtrlList);
//     dbCtrl->dbId = 0;
//     dbCtrl->dbName = NULL;
// }

Status SrCheckCreateDbArgs(SimpleRelExecCtxT *execCtx) {
    if (execCtx->dbName == NULL || strcmp(execCtx->dbName, "") == 0) {
        log_error("SrCheckCreateDbArgs: dbName is null");
        return GMERR_DATAMODEL_SRDB_NAME_NULL;
    }
    if (strlen(execCtx->dbName) > SR_DB_NAME_MAX_LENGTH) {
        log_error("SrCheckCreateDbArgs: dbName is too long");
        return GMERR_DATAMODEL_SRDB_NAME_TOO_LONG;
    }
    return GMERR_OK;
}

Status DMSrCreateDb(QryStmtT *stmt) {
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;
    Status ret = SrCheckCreateDbArgs(execCtx);
    if (ret != GMERR_OK) {
        return ret;
    }
    if (IsDbNameExist(execCtx->dbName)) {
        log_error("DMSrCreateDb: dbName is exist.");
        return GMERR_DATAMODEL_SRDB_NAME_EXISTED;
    }
    SrDbCtrlT dbCtrl = {0};
    dbCtrl.dbName = (char *)KVMemAlloc(strlen(execCtx->dbName) + 1);
    if (dbCtrl.dbName == NULL) {
        log_error("DMSrCreateDb: dbName alloc failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    memset(dbCtrl.dbName, 0, strlen(execCtx->dbName) + 1);
    strcpy(dbCtrl.dbName, execCtx->dbName);
    ret = DbVectorInit(&dbCtrl.labelCtrlList, sizeof(SrLabelT));
    if (ret != GMERR_OK) {
        KVMemFree(dbCtrl.dbName, sizeof(SrLabelT));
        log_error("DMSrCreateDb: DbVectorInit labelCtrlList failed.");
        return ret;
    }
    dbCtrl.dbId = GenSrDbId();

    SrDbCtrlManagerT *dbCtrlMgr = GetDbCtrlManager();
    ret = DbVectorAppendItem(&dbCtrlMgr->dbCtrlList, &dbCtrl);
    if (ret != GMERR_OK) {
        KVMemFree(dbCtrl.dbName, strlen(execCtx->dbName) + 1);
        DbVectorDestroy(&dbCtrl.labelCtrlList);
        log_error("DMSrCreateDb: DbVectorAppendItem labelCtrlList failed.");
        return ret;
    }
    // 设置返回结果
    uint32_t retEntryBufLen = sizeof(uint32_t);
    void *retEntry = (void *)KVMemAlloc(retEntryBufLen);
    if (retEntry == NULL) {
        KVMemFree(dbCtrl.dbName, strlen(execCtx->dbName) + 1);
        DbVectorDestroy(&dbCtrl.labelCtrlList);
        log_error("DMSrCreateDb: KVMemAlloc retEntry failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    memset(retEntry, 0, retEntryBufLen);
    *((uint32_t *)retEntry) = dbCtrl.dbId;
    stmt->retEntry = retEntry;
    stmt->retEntryBufLen = retEntryBufLen;
    return GMERR_OK;
}

Status DMSrDropDb(QryStmtT *stmt) {
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;
    Status ret = SrCheckCreateDbArgs(execCtx);
    if (ret != GMERR_OK) {
        return ret;
    }
    if (!IsDbNameExist(execCtx->dbName)) {
        log_warn("DMSrDropDb: dbName is not exist.");
        return GMERR_DATAMODEL_SRDB_NAME_NOT_EXISTED;
    }
    SrDbCtrlT *dbCtrl = DmGetDbCtrlByName(execCtx->dbName);
    if (dbCtrl == NULL) {
        log_error("DMSrDropDb: DmGetDbCtrlByName failed.");
        return GMERR_DATAMODEL_SRDB_LIST_EXCEPT_NULL;
    }
    // DmClearSingleDbCtrl(dbCtrl);
    return RemoveDbCtrlByName(execCtx->dbName);
}