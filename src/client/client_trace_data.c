#include "client_common.h"
#include <stdio.h>

void TraceOneLine(uint32_t cnt) {
    for (uint32_t i = 0; i < cnt; ++i) {
        printf("----------");
    }
    printf("\n");
}

void TraceSchemaHead(CliTableSchemaT *tableSchema) {
    TraceOneLine(tableSchema->propertyCnt);
    for (uint32_t i = 0; i < tableSchema->propertyCnt; ++i) {
        printf("| %-10s", tableSchema->properties[i].fldName);
    }
    printf("| \n");
}

void TraceCondition(CliTableSchemaT *tableSchema, const char *conditionStr) {
    if (conditionStr == NULL) {
        return;
    }
    TraceOneLine(tableSchema->propertyCnt);
    printf("| %-10s", conditionStr);
    TraceOneLine(tableSchema->propertyCnt);
}

void TraceAllRecord(CliTableSchemaT *tableSchema, UsrDataSimpleRelQueryT *queryData) {
    uint8_t *bufCursor = queryData->fetchBuf;
    for (uint32_t i = 0; i < queryData->fetchCnt; ++i) {
        // 记录单挑记录
        for (uint32_t j = 0; j < tableSchema->propertyCnt; ++j) {
            printf("| %-10s", bufCursor);
            bufCursor += tableSchema->properties[j].type;

            CliPropertyT *properties = &tableSchema->properties[j];
            if (properties->type == SR_LABEL_FILED_TYPE_UINT32) {
                printf("| %10u  ", i, *(uint32_t *)(bufCursor + properties->fldOffset));
            } else if (properties->type == SR_LABEL_FILED_TYPE_INT32) {
                printf("| %10d  ", i, *(int32_t *)((uint8_t *)bufCursor + properties->fldOffset));
            } else {
                printf("| %s    ", i, (uint8_t *)((uint8_t *)bufCursor + properties->fldOffset));
            }
            printf("| \n");
        }
    }
    TraceOneLine(tableSchema->propertyCnt);
}

void CliTraceQueryData(CliTableSchemaT *tableSchema, UsrDataSimpleRelQueryT *queryData, const char *conditionStr) {
    TraceSchemaHead(tableSchema);
    TraceCondition(tableSchema, conditionStr);
    // 记录每条数据
    TraceAllRecord(tableSchema, queryData);
}