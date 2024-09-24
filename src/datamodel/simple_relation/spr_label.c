// #include "include/spr_common.h"

// void DmClearSingleDbCtrl(SrDbCtrlT *dbCtrl)
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

// Status SrCheckCreateTableArgs(SimpleRelExecCtxT *execCtx)
// {
//     if (execCtx->labelName == NULL || strcmp(execCtx->labelName, "") == 0)
//     {
//         log_error("SrCheckCreateTableArgs: labelName is null");
//         return GMERR_DATAMODEL_SRDB_NAME_NULL;
//     }
//     if (strlen(execCtx->labelName) > SR_LABELNAME_MAX_LENGTH)
//     {
//         log_error("SrCheckCreateTableArgs: labelName is too long");
//         return GMERR_DATAMODEL_SRDB_NAME_TOO_LONG;
//     }
//     return GMERR_OK;
// }

// Status SrDmCreateTable(SimpleRelExecCtxT *execCtx)
// {
//     Status ret = SrCheckCreateTableArgs(execCtx);
//     if (ret != GMERR_OK)
//     {
//         return ret;
//     }
//     if (IsLabelNameExist(execCtx->labelName))
//     {
//         log_error("SrDmCreateTable: labelName is exist.");
//         return GMERR_DATAMODEL_SRLABEL_NAME_EXISTED;
//     }
//     SrLabelCtrlT labelCtrl = {0};

//     labelCtrl.labelName = (char *)KVMemAlloc(strlen(execCtx->labelName) + 1);
//     if (labelCtrl.labelName == NULL)
//     {
//         log_error("SrDmCreateDb: labelName alloc failed.");
//         return GMERR_KV_MEMORY_ALLOC_FAILED;
//     }
//     labelCtrl.labelId = GenSrTableId();
//     execCtx->labelId = labelCtrl.labelId;
//     return GMERR_OK;
// }

// Status SrDmInsertData(SimpleRelExecCtxT *execCtx){}