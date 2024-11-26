#include "ee_common.h"

Status DMSrInsertData(QryStmtT *stmt) {
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;
    // 找 dbId 是否存在
    SrDbCtrlT *dbCtrl = DmGetDbCtrlByDbId(execCtx->dbId);
    if (dbCtrl == NULL) {
        log_error("DMSrInsertData: get dbCtrl failed.");
        return GMERR_DATAMODEL_SRDB_ID_NOT_EXISTED;
    }

    // 找 labelId 是否存在
    SrLabelT *labelCtrl = DmGetLabelCtrlByLabelId(dbCtrl, execCtx->labelId);
    if (labelCtrl == NULL) {
        log_error("DMSrInsertData: get labelCtrl failed.");
        return GMERR_DATAMODEL_SRLABEL_ID_NOT_EXISTED;
    }
    Status ret = SEHeapInsertRow(labelCtrl->labelId, execCtx->insertData, NULL);
    if (ret != GMERR_OK) {
        return ret;
    }
    return GMERR_OK;
}