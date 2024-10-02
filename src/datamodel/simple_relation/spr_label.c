#include "include/spr_common.h"
#include "kv_json.h"

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

Status SrCheckCreateTableArgs(SimpleRelExecCtxT *execCtx) {
    if (execCtx->labelJson == NULL || strcmp(execCtx->labelJson, "") == 0) {
        log_error("SrCheckCreateTableArgs: labelJson is null");
        return GMERR_DATAMODEL_SR_CREATE_LABEL_JSON_NULL;
    }
    return GMERR_OK;
}

Status SeSetPropertyType(SrPropertyT *property, const char *type) {
    if (strcmp(type, "int") == 0) {
        property->fieldType = SR_LABEL_FILED_TYPE_INT32;
    } else if (strcmp(type, "uint32") == 0) {
        property->fieldType = SR_LABEL_FILED_TYPE_UINT32;
    } else if (strcmp(type, "string") == 0) {
        property->fieldType = SR_LABEL_FILED_TYPE_STRING;
    } else {
        log_error("SeSetPropertyType: type is not support.");
        return GMERR_KV_NOT_SUPPORT;
    }
    return GMERR_OK;
}

Status SrParseSingleField(json_t *field, SrPropertyT *property) {
    json_t *fieldName = NULL;
    Status ret = KVJsonGetObject(field, "name", &fieldName);
    if (ret != GMERR_OK) {
        log_error("SrParseSingleField: name is null.");
        return ret;
    }
    ret = KVJsonParseStringObjToBuf(fieldName, property->fieldName, SR_FIELD_NAME_MAX_LENGTH);
    if (ret != GMERR_OK) {
        log_error("KVJsonParseStringObjToBuf failed.");
        return ret;
    }
    json_t *fieldType = NULL;
    ret = KVJsonGetObject(field, "type", &fieldType);
    if (ret != GMERR_OK) {
        log_error("SrParseSingleField: type is null.");
        return ret;
    }
    const char *fieldTypeStr = NULL;
    ret = KVJsonParseStringObj(fieldType, &fieldTypeStr);
    if (ret != GMERR_OK) {
        log_error("KVJsonParseStringObj failed.");
        return ret;
    }
    ret = SeSetPropertyType(property, fieldTypeStr);
    if (ret != GMERR_OK) {
        log_error("SeSetPropertyType failed.");
        return ret;
    }
    // 解析字段长度
    json_t *fieldLen = NULL;
    ret = KVJsonGetObject(field, "size", &fieldLen);
    if (ret != GMERR_OK) {
        log_error("SrParseSingleField: size is null.");
        return ret;
    }
    int32_t fieldLength = 0;
    ret = KVJsonParseIntObj(fieldLen, &fieldLength);
    if (ret != GMERR_OK) {
        log_error("KVJsonParseIntObj failed.");
        return ret;
    }
    property->fieldSize = fieldLength;
    return GMERR_OK;
}

Status SrParseCreateLabelJson(const char *labelJson, SrCreateLabelCtxT *createLabelCtx) {
    json_t *labelRoot = NULL;
    Status ret = KVStringToJSON(labelJson, &labelRoot);
    if (ret != GMERR_OK) {
        log_error("KVStringToJSON failed.");
        return ret;
    }
    // json_t *labelName = json_object_get(labelRoot, "labelName");

    // 解析 labelName 必不可少
    json_t *labelName = NULL;
    ret = KVJsonGetObject(labelRoot, "labelName", &labelName);
    if (ret != GMERR_OK) {
        log_error("LabelName maybe is null or label json has no labelName field.");
        return ret;
    }
    ret = KVJsonParseStringObjToBuf(labelName, createLabelCtx->labelName, SR_LABEL_NAME_MAX_LENGTH);
    if (ret != GMERR_OK) {
        log_error("KVJsonParseStringObjToBuf failed.");
        return ret;
    }
    // 解析 fields 数组，至少为 1 至多 30
    json_t *fields = NULL;
    ret = KVJsonGetObject(labelRoot, "fields", &fields);
    if (ret != GMERR_OK) {
        log_error("fields not exists.");
        return ret;
    }

    if (KVJsonIsArray(fields) == false) {
        log_error("fields is not array.");
        return ret;
    }

    uint32_t fieldsLen = KVJsonGetArraySize(fields);
    if (fieldsLen == 0 || fieldsLen > SR_LABEL_MAX_FILED_CNT) {
        log_error("fields len is not in range[1-30], len is %u.", fieldsLen);
        return GMERR_DATAMODEL_SR_CREATE_LABEL_FIELDS_OUTRANGE;
    }

    createLabelCtx->fieldCnt = fieldsLen;

    for (uint32_t i = 0; i < fieldsLen; i++) {
        json_t *field = NULL;
        ret = KVJsonArrayGetItem(fields, i, &field);
        if (ret != GMERR_OK) {
            return ret;
        }
        ret = SrParseSingleField(field, &createLabelCtx->properties[i]);
        if (ret != GMERR_OK) {
            return ret;
        }
    }

    return GMERR_OK;
}

// TODO:异常分支资源清理
Status DMSrCreateTable(QryStmtT *stmt) {
    // 首先获取DbCtrl
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;

    SrDbCtrlT *dbCtrl = DmGetDbCtrlByDbId(execCtx->dbId);
    if (dbCtrl == NULL) {
        log_error("DMSrCreateTable: get dbCtrl failed.");
        return GMERR_DATAMODEL_SRDB_ID_NOT_EXISTED;
    }

    Status ret = SrCheckCreateTableArgs(execCtx);
    if (ret != GMERR_OK) {
        return ret;
    }

    SrCreateLabelCtxT createLabelCtx = {0};
    ret = SrParseCreateLabelJson(execCtx->labelJson, &createLabelCtx);
    if (ret != GMERR_OK) {
        log_error("Parse label json failed.");
        return ret;
    }

    if (IsLabelNameExist(dbCtrl, createLabelCtx.labelName)) {
        log_error("DMSrCreateTable: labelName is exist.");
        return GMERR_DATAMODEL_SRLABEL_NAME_EXISTED;
    }

    SrLabelT labelCtrl = {0};
    char *labelName = (char *)KVMemAlloc(strlen(createLabelCtx.labelName) + 1);
    if (labelName == NULL) {
        log_error("DMSrCreateDb: labelName alloc failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    strcpy(labelName, createLabelCtx.labelName);

    uint32_t memSize = createLabelCtx.fieldCnt * sizeof(SrPropertyT);
    SrPropertyT *properties = (SrPropertyT *)KVMemAlloc(memSize);
    if (properties == NULL) {
        log_error("DMSrCreateDb: properties alloc failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    memset(properties, 0x00, memSize);
    uint32_t fldOffset = 0;
    for (uint32_t i = 0; i < createLabelCtx.fieldCnt; i++) {
        SrPropertyT *property = &properties[i];
        // char *fieldName = (char *)KVMemAlloc(strlen(createLabelCtx.properties[i].fieldName) + 1);
        // if (fieldName == NULL) {
        //     log_error("DMSrCreateDb: fieldName alloc failed.");
        //     return GMERR_KV_MEMORY_ALLOC_FAILED;
        // }
        strcpy(property->fieldName, createLabelCtx.properties[i].fieldName);
        property->fieldSize = createLabelCtx.properties[i].fieldSize;
        property->fieldType = createLabelCtx.properties[i].fieldType;
        property->fldOffset = fldOffset;
        fldOffset += property->fieldSize;
    }
    labelCtrl.properties = properties;
    labelCtrl.fieldCnt = createLabelCtx.fieldCnt;
    labelCtrl.labelId = GenSrTableId();
    labelCtrl.dbId = execCtx->dbId;
    labelCtrl.labelName = labelName;

    // 申请页存储空间
    SEFixedHeapInit(&labelCtrl.heapRow, fldOffset);
    log_debug("debug: server fld total len is %u.", fldOffset);

    ret = DbVectorAppendItem(&dbCtrl->labelCtrlList, &labelCtrl);
    if (ret != GMERR_OK) {
        log_error("DbVectorAppend labelCtrl failed.");
        return ret;
    }

    // 设置返回结果
    uint32_t retEntryBufLen = sizeof(uint32_t);
    void *retEntry = (void *)KVMemAlloc(retEntryBufLen);
    if (retEntry == NULL) {
        log_error("DMSrCreateTable: KVMemAlloc retEntry failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    memset(retEntry, 0, retEntryBufLen);
    *((uint32_t *)retEntry) = labelCtrl.labelId;
    stmt->retEntry = retEntry;
    stmt->retEntryBufLen = retEntryBufLen;

    return GMERR_OK;
}

// Status DMSrInsertData(QryStmtT *stmt){}

// 打印--------------------------------------
void DmPrintInterval(uint32_t printCnt, const uint32_t printLen) {
    for (uint32_t i = 0; i < printCnt + 1; ++i) {
        for (uint32_t j = 0; j < printLen + 1; ++j) {
            printf("-");
        }
    }
    printf("-\n");
}

void DmPrintTitle(SrLabelT *labelCtrl) {
    const char *titleOrder = "order";
    printf("|%20s", titleOrder);
    for (uint32_t i = 0; i < labelCtrl->fieldCnt; ++i) {
        printf("|%20s", labelCtrl->properties[i].fieldName);
    }
    printf("|\n");
}

void DmTraceTableRow(SrLabelT *labelCtrl) {
    DB_POINT(labelCtrl);

    const uint32_t printLen = 20; // 每格20
    // 打印标题栏
    DmPrintInterval(labelCtrl->fieldCnt, printLen);
    DmPrintTitle(labelCtrl);
    DmPrintInterval(labelCtrl->fieldCnt, printLen);
    SrPropertyT *properties = labelCtrl->properties;
    uint8_t *traceCursor = labelCtrl->heapRow.pageBegin;
    for (uint32_t i = 0; i < labelCtrl->heapRow.rowCnt; ++i) {
        printf("|%20u", i + 1);
        for (uint32_t j = 0; j < labelCtrl->fieldCnt; ++j) {
            if (properties[j].fieldType == SR_LABEL_FILED_TYPE_INT32) {
                printf("|%20d", *(int32_t *)(traceCursor + properties[j].fldOffset));
            } else if (properties[j].fieldType == SR_LABEL_FILED_TYPE_UINT32) {
                printf("|%20u", *(uint32_t *)(traceCursor + properties[j].fldOffset));
            } else if (properties[j].fieldType == SR_LABEL_FILED_TYPE_STRING) {
                printf("|%20s", (char *)traceCursor + properties[j].fldOffset);
            }
        }
        printf("|\n");
        DmPrintInterval(labelCtrl->fieldCnt, printLen);
        traceCursor += labelCtrl->heapRow.rowSize;
    }
}

Status DMSrGetDbDesc(QryStmtT *stmt) {
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;
    SrDbCtrlT *dbCtrl = DmGetDbCtrlByDbId(execCtx->dbId);
    if (dbCtrl == NULL) {
        log_error("DMSrCreateTable: get dbCtrl failed.");
        return GMERR_DATAMODEL_SRDB_ID_NOT_EXISTED;
    }

    // 打印表的个数
    printf("查询数据库ID为:%u ", execCtx->dbId);
    printf("数据库中表的数量为:%u\n", DbVectorGetSize(&dbCtrl->labelCtrlList));
    for (uint32_t i = 0; i < DbVectorGetSize(&dbCtrl->labelCtrlList); i++) {
        SrLabelT *labelCtrl = (SrLabelT *)DbVectorGetItem(&dbCtrl->labelCtrlList, i);
        printf("第%d个表的信息为:\n", i);
        printf("表ID为:%u ", labelCtrl->labelId);
        printf("表名称为:%s ", labelCtrl->labelName);
        printf("表中字段的数量为:%u\n ", labelCtrl->fieldCnt);
        for (uint32_t j = 0; j < labelCtrl->fieldCnt; j++) {
            SrPropertyT *property = &labelCtrl->properties[j];
            printf("%d:", j + 1);
            printf("名称:%s ", property->fieldName);
            printf("类型:%u ", property->fieldType);
            printf("大小:%u\n", property->fieldSize);
        }
        printf("*******************************\n");
        DmTraceTableRow(labelCtrl);
        printf("*******************************\n");
    }
    return GMERR_OK;
}

Status DMSrQueryTable(QryStmtT *stmt) {
    SimpleRelExecCtxT *execCtx = (SimpleRelExecCtxT *)stmt->entry;
    // 找 dbId 是否存在
    SrDbCtrlT *dbCtrl = DmGetDbCtrlByDbId(execCtx->dbId);
    if (dbCtrl == NULL) {
        log_error("DMSrCreateTable: get dbCtrl failed.");
        return GMERR_DATAMODEL_SRDB_ID_NOT_EXISTED;
    }

    // 找 labelId 是否存在
    SrLabelT *labelCtrl = DmGetLabelCtrlByLabelId(dbCtrl, execCtx->labelId);
    if (labelCtrl == NULL) {
        log_error("DMSrCreateTable: get labelCtrl failed.");
        return GMERR_DATAMODEL_SRLABEL_ID_NOT_EXISTED;
    }
    SrPropertyT *properties = (SrPropertyT *)KVMemAlloc(labelCtrl->fieldCnt * sizeof(SrPropertyT));
    if (properties == NULL) {
        log_error("DMSrCreateTable: properties alloc failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    memset(properties, 0x00, labelCtrl->fieldCnt * sizeof(SrPropertyT));
    // 慎用，这里能直接浅拷贝是因为内部没有指针，将来添加指针的话请同步修改
    memcpy(properties, labelCtrl->properties, labelCtrl->fieldCnt * sizeof(SrPropertyT));
    stmt->retEntryBufLen = labelCtrl->fieldCnt * sizeof(SrPropertyT);
    stmt->retEntry = (void *)properties;
    stmt->currLabelFldCnt = labelCtrl->fieldCnt;
    return GMERR_OK;
}

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

    SEFixedHeapInsertRow(&labelCtrl->heapRow, execCtx->insertData);
    return GMERR_OK;
}
