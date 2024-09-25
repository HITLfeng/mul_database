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

Status SrCheckCreateTableArgs(SimpleRelExecCtxT *execCtx)
{
    if (execCtx->labelJson == NULL || strcmp(execCtx->labelJson, "") == 0)
    {
        log_error("SrCheckCreateTableArgs: labelJson is null");
        return GMERR_DATAMODEL_SR_CREATE_LABEL_JSON_NULL;
    }
    return GMERR_OK;
}

Status SeSetPropertyType(SrPropertyT *property, const char *type)
{
    if (strcmp(type, "int") == 0)
    {
        property->fieldType = SR_LABEL_FILED_TYPE_INT32;
    }
    else if (strcmp(type, "uint32") == 0)
    {
        property->fieldType = SR_LABEL_FILED_TYPE_UINT32;
    }
    else if (strcmp(type, "string") == 0)
    {
        property->fieldType = SR_LABEL_FILED_TYPE_STRING;
    }
    else
    {
        log_error("SeSetPropertyType: type is not support.");
        return GMERR_KV_NOT_SUPPORT;
    }
    return GMERR_OK;
}

Status SrParseSingleField(json_t *field, SrPropertyT *property)
{
    json_t *fieldName = NULL;
    Status ret = KVJsonGetObject(field, "name", &fieldName);
    if (ret != GMERR_OK)
    {
        log_error("SrParseSingleField: name is null.");
        return ret;
    }
    ret = KVJsonParseStringObjToBuf(fieldName, property->fieldName, SR_FIELD_NAME_MAX_LENGTH);
    if (ret != GMERR_OK)
    {
        log_error("KVJsonParseStringObjToBuf failed.");
        return ret;
    }
    json_t *fieldType = NULL;
    ret = KVJsonGetObject(field, "type", &fieldType);
    if (ret != GMERR_OK)
    {
        log_error("SrParseSingleField: type is null.");
        return ret;
    }
    char *fieldTypeStr = NULL;
    ret = KVJsonParseStringObj(fieldType, &fieldTypeStr);
    if (ret != GMERR_OK)
    {
        log_error("KVJsonParseStringObj failed.");
        return ret;
    }
    ret = SeSetPropertyType(property, fieldTypeStr);
    if (ret != GMERR_OK)
    {
        log_error("SeSetPropertyType failed.");
        return ret;
    }
    // 解析字段长度
    json_t *fieldLen = NULL;
    ret = KVJsonGetObject(field, "size", &fieldLen);
    if (ret != GMERR_OK)
    {
        log_error("SrParseSingleField: size is null.");
        return ret;
    }
    int32_t fieldLength = 0;
    ret = KVJsonParseIntObj(fieldLen, &fieldLength);
    if (ret != GMERR_OK)
    {
        log_error("KVJsonParseIntObj failed.");
        return ret;
    }
    property->fieldSize = fieldLength;
    return GMERR_OK;
}

Status SrParseCreateLabelJson(const char *labelJson, SrCreateLabelCtxT *createLabelCtx)
{
    json_t *labelRoot = NULL;
    Status ret = KVStringToJSON(labelJson, &labelRoot);
    if (ret != GMERR_OK)
    {
        log_error("KVStringToJSON failed.");
        return ret;
    }
    // json_t *labelName = json_object_get(labelRoot, "labelName");

    // 解析 labelName 必不可少
    json_t *labelName = NULL;
    ret = KVJsonGetObject(labelRoot, "labelName", &labelName);
    if (ret != GMERR_OK)
    {
        log_error("LabelName maybe is null or label json has no labelName field.");
        return ret;
    }
    ret = KVJsonParseStringObjToBuf(labelName, createLabelCtx->labelName, SR_LABELNAME_MAX_LENGTH);
    if (ret != GMERR_OK)
    {
        log_error("KVJsonParseStringObjToBuf failed.");
        return ret;
    }
    // 解析 fields 数组，至少为 1 至多 30
    json_t *fields = NULL;
    ret = KVJsonGetObject(labelRoot, "fields", &fields);
    if (ret != GMERR_OK)
    {
        log_error("fields not exists.");
        return ret;
    }

    if (KVJsonIsArray(fields) == false)
    {
        log_error("fields is not array.");
        return ret;
    }

    uint32_t fieldsLen = KVJsonGetArraySize(fields);
    if (fieldsLen == 0 || fieldsLen > SR_LABEL_MAX_FILED_CNT)
    {
        log_error("fields len is not in range[1-30], len is %u.", fieldsLen);
        return GMERR_DATAMODEL_SR_CREATE_LABEL_FIELDS_OUTRANGE;
    }

    createLabelCtx->fieldCnt = fieldsLen;

    for (uint32_t i = 0; i < fieldsLen; i++)
    {
        json_t *field = NULL;
        ret = KVJsonArrayGetItem(fields, i, &field);
        if (ret != GMERR_OK)
        {
            return ret;
        }
        ret = SrParseSingleField(field, &createLabelCtx->properties[i]);
        if (ret != GMERR_OK)
        {
            return ret;
        }
    }

    return GMERR_OK;
}

Status SrDmCreateTable(SimpleRelExecCtxT *execCtx)
{
    Status ret = SrCheckCreateTableArgs(execCtx);
    if (ret != GMERR_OK)
    {
        return ret;
    }

    SrCreateLabelCtxT createLabelCtx = {0};
    ret = SrParseCreateLabelJson(execCtx->labelJson, &createLabelCtx);
    if (ret != GMERR_OK)
    {
        log_error("Parse label json failed.");
        return ret;
    }

    if (IsLabelNameExist(createLabelCtx.labelName))
    {
        log_error("SrDmCreateTable: labelName is exist.");
        return GMERR_DATAMODEL_SRLABEL_NAME_EXISTED;
    }


    SrLabelCtrlT labelCtrl = {0};

    labelCtrl.labelName = (char *)KVMemAlloc(strlen(execCtx->labelName) + 1);
    if (labelCtrl.labelName == NULL)
    {
        log_error("SrDmCreateDb: labelName alloc failed.");
        return GMERR_KV_MEMORY_ALLOC_FAILED;
    }
    labelCtrl.labelId = GenSrTableId();
    execCtx->labelId = labelCtrl.labelId;
    return GMERR_OK;
}

// Status SrDmInsertData(SimpleRelExecCtxT *execCtx){}