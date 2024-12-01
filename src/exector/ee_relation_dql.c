#include "ee_common.h"

// tmp
#include <stdio.h>
// tmp

void TraceSingleRecord(SrLabelT *labelCtrl, HeapBufT *heapBuf)
{
    DB_POINT2(labelCtrl, heapBuf);
    DB_ASSERT(labelCtrl->recordLen == heapBuf->bufSize);
    for (uint32_t i = 0; i < labelCtrl->fieldCnt; ++i) {
        SrPropertyT *property = &labelCtrl->properties[i];
        if (property->fieldType == SR_LABEL_FILED_TYPE_UINT32) {
            printf("| fld %u: %u | ", i, *(uint32_t *)((uint8_t *)heapBuf->buf + property->fldOffset));
        } else if (property->fieldType == SR_LABEL_FILED_TYPE_INT32) {
            printf("| fld %u: %d | ", i, *(int32_t *)((uint8_t *)heapBuf->buf + property->fldOffset));
        } else {
            printf("| fld %u: %s | ", i, (uint8_t *)((uint8_t *)heapBuf->buf + property->fldOffset));
        }
    }
    printf("\n");
}

Status EEQueryData(QryStmtT *stmt)
{
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;
    // 找 dbId 是否存在
    SrDbCtrlT *dbCtrl = DmGetDbCtrlByDbId(execCtx->dbId);
    if (dbCtrl == NULL) {
        log_error("EEQueryData: get dbCtrl failed.");
        return GMERR_DATAMODEL_SRDB_ID_NOT_EXISTED;
    }

    // 找 labelId 是否存在
    SrLabelT *labelCtrl = DmGetLabelCtrlByLabelId(dbCtrl, execCtx->labelId);
    if (labelCtrl == NULL) {
        log_error("EEQueryData: get labelCtrl failed.");
        return GMERR_DATAMODEL_SRLABEL_ID_NOT_EXISTED;
    }
    LabelCursorT labelCursor = (LabelCursorT) {0};
    Status ret = SEHeapOpenLabelCursor(labelCtrl->labelId, &labelCursor);
    if (ret != GMERR_OK) {
        log_error("query data: open label cursor failed.");
        return ret;
    }
    do {
        FetchArgsT fetchArgs = {
                .fetchCnt = 0,
                .memCtx = stmt->memCtx,
                .dealBuf = NULL,
                .heapBuf = NULL,
                .usrData = NULL
        };
        ret = SEHeapFetchNextWithCond(&labelCursor, &fetchArgs);
        if (ret == GMERR_OK) {
            TraceSingleRecord(labelCtrl, fetchArgs.heapBuf);
        }
        if (fetchArgs.heapBuf != NULL) {
            DbDynMemCtxFree(fetchArgs.memCtx, fetchArgs.heapBuf);
        }
    } while (ret != GMERR_NO_DATA);

}