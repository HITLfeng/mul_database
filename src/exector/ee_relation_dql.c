#include "ee_common.h"

// tmp
#include <stdio.h>
// tmp

void TraceSingleRecord(SrLabelT *labelCtrl, HeapBufT *heapBuf) {
    if (!IsDebugInfoOn()) {
        return;
    }
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

typedef struct HeapCmpUserData {
    SrPropertyT *properties;
    SRCondT *cond;
} HeapCmpUserDataT;

// SR_LABEL_FILED_TYPE_INT32 = 0,
//         SR_LABEL_FILED_TYPE_UINT32 = 1,
//// SR_LABEL_FILED_TYPE_FLOAT,
// SR_LABEL_FILED_TYPE_STRING,
const void *GetBufByOffset(const void *buf, uint32_t offset) { return (const uint8_t *)buf + offset; }

bool IsCondMatch(SRCondCmpT cmpType, int32_t result) {
    switch (cmpType) {
    case OP_CMP_LARGE:
        return result > 0;
    case OP_CMP_EQUAL:
        return result == 0;
    case OP_CMP_LESS:
        return result < 0;
    }
}

bool EEQueryDataMatchCond(const HeapBufT *heapBuf, void *usrData) {
    HeapCmpUserDataT *cmpData = (HeapCmpUserDataT *)usrData;
    SRCondT *cond = cmpData->cond;

    if (cond->cmpType == OP_CMP_NULL) {
        // 此种比较类型 全部匹配
        return true;
    }

    SrPropertyT *properties = cmpData->properties;
    SrPropertyT *property = &properties[cond->fldIdx];
    // TODO: GetDmValue
    //    if (property->fieldType == SR_LABEL_FILED_TYPE_INT32) {
    //
    //    }

    // 1.获取 buf 中的value
    DmValueT dmValue = {0};
    DmSetValue(&dmValue, GetBufByOffset(heapBuf->buf, property->fldOffset), property->fieldSize, property->fieldType);

    // 2. 比较两个 value
    int32_t result = DmCmpValue(&dmValue, &cond->dbValue);

    // 3. 判断是否 match
    bool isMatch = IsCondMatch(cond->cmpType, result);
    return isMatch;
}

void EEInitFetchArgs(FetchArgsT *fetchArgs, HeapCmpUserDataT *cmpData, DbMemCtxT *memCtx) {
    DB_ASSERT(fetchArgs != NULL);
    fetchArgs->fetchCnt = 0;
    fetchArgs->memCtx = memCtx;
    fetchArgs->matchCond = EEQueryDataMatchCond;
    fetchArgs->heapBuf = NULL;
    fetchArgs->usrData = cmpData;
    DbVectorInit(&(fetchArgs->dataVector), sizeof(HeapBufT), memCtx);
}

void EEReleaseFetchArgs(FetchArgsT *fetchArgs) { DbVectorDestroy(&(fetchArgs->dataVector)); }

Status EEQueryData(QryStmtT *stmt) {
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
    LabelCursorT labelCursor = (LabelCursorT){0};
    Status ret = SEHeapOpenLabelCursor(labelCtrl->labelId, &labelCursor);
    if (ret != GMERR_OK) {
        log_error("query data: open label cursor failed.");
        return ret;
    }
    HeapCmpUserDataT cmpData = {.properties = labelCtrl->properties, .cond = &execCtx->cond};
    FetchArgsT fetchArgs = {0};
    EEInitFetchArgs(&fetchArgs, &cmpData, stmt->memCtx);
    do {
        ret = SEHeapFetchNextWithCond(&labelCursor, &fetchArgs);
        if (ret == GMERR_OK) {
            TraceSingleRecord(labelCtrl, fetchArgs.heapBuf);
        }
        if (fetchArgs.heapBuf != NULL) {
            ret = DbVectorAppendItem(&(fetchArgs.dataVector), fetchArgs.heapBuf);
            if (ret != GMERR_OK) {
                log_error("EEQueryData: append data to vector failed.");
                return ret;
            }
        }
    } while (ret != GMERR_NO_DATA && !labelCursor.isFetchEnd);

    // 设置返回信息 retEntry
    // 统一申请内存
    if (fetchArgs.fetchCnt > 0) {
        void *retBuf = DbDynMemCtxAlloc(stmt->memCtx, labelCtrl->recordLen * fetchArgs.fetchCnt);
        if (retBuf == NULL) {
            log_error("EEQueryData: alloc retEntry failed.");
            return GMERR_MEMORY_ALLOC_FAILED;
        }
        uint8_t *retCursor = retBuf;
        // TODO: 后续改为数据分批返回 现在先不处理，超过部分自动截断
        for (uint32_t i = 0; i < fetchArgs.fetchCnt; ++i) {
            HeapBufT *heapBuf = (HeapBufT *)DbVectorGetItem(&(fetchArgs.dataVector), i);
            memcpy(retCursor, heapBuf->buf, heapBuf->bufSize);
            retCursor += heapBuf->bufSize;
            DbDynMemCtxFree(fetchArgs.memCtx, heapBuf->buf); // 由memCtx delete 统一释放
        }
        stmt->fetchCnt = fetchArgs.fetchCnt;
        stmt->retEntry = retBuf;
        stmt->retEntryBufLen = labelCtrl->recordLen * fetchArgs.fetchCnt;
    }
    EEReleaseFetchArgs(&fetchArgs);
    return ret == GMERR_NO_DATA ? GMERR_OK : ret;
}

Status EEDeleteData(QryStmtT *stmt) {
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;
    // 找 dbId 是否存在
    SrDbCtrlT *dbCtrl = DmGetDbCtrlByDbId(execCtx->dbId);
    if (dbCtrl == NULL) {
        log_error("EEDeleteData: get dbCtrl failed.");
        return GMERR_DATAMODEL_SRDB_ID_NOT_EXISTED;
    }

    // 找 labelId 是否存在
    SrLabelT *labelCtrl = DmGetLabelCtrlByLabelId(dbCtrl, execCtx->labelId);
    if (labelCtrl == NULL) {
        log_error("EEDeleteData: get labelCtrl failed.");
        return GMERR_DATAMODEL_SRLABEL_ID_NOT_EXISTED;
    }
    LabelCursorT labelCursor = (LabelCursorT){0};
    Status ret = SEHeapOpenLabelCursor(labelCtrl->labelId, &labelCursor);
    if (ret != GMERR_OK) {
        log_error("query data: open label cursor failed.");
        return ret;
    }
    HeapCmpUserDataT cmpData = {.properties = labelCtrl->properties, .cond = &execCtx->cond};
    do {
        FetchArgsT fetchArgs = {.fetchCnt = 0,
                                .memCtx = stmt->memCtx,
                                .matchCond = EEQueryDataMatchCond,
                                .heapBuf = NULL,
                                .usrData = &cmpData};
        ret = SEHeapFetchAndDeleteWithCond(&labelCursor, &fetchArgs);
        if (ret != GMERR_OK && ret != GMERR_NO_DATA) {
            log_error("heap fetch and delete data failed.");
            return ret;
        }
    } while (ret != GMERR_NO_DATA && !labelCursor.isFetchEnd);
    return ret == GMERR_NO_DATA ? GMERR_OK : ret;
}
