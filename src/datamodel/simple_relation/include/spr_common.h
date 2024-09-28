#ifndef __SPR_COMMON_H__
#define __SPR_COMMON_H__

#include "../../../interface/include/outfunction.h"
#include "../../../common/include/vector_util.h"
#include "../../../common/include/common.h"
#include "../../../common/include/kv_memory.h"


typedef struct SrDbCtrl
{
    char *dbName;
    uint32_t dbId;
    DbVectorT labelCtrlList; // 存放 SrLabelT
} SrDbCtrlT;

typedef struct SrDbCtrlManager
{
    // TODO:lock
    DbVectorT dbCtrlList; // 存放 SrDbCtrlT
} SrDbCtrlManagerT;

typedef struct SrProperty
{
    char fieldName[SR_FIELD_NAME_MAX_LENGTH];
    SrLabelFiledTypeT fieldType;
    uint32_t fieldSize; // 字段长度 只支持定长
} SrPropertyT;

typedef struct SrLabel
{
    uint32_t dbId;
    const char *labelName;
    uint32_t labelId;
    uint32_t fieldCnt;       // feild 个数
    SrPropertyT *properties; // 属性数组
} SrLabelT;


typedef struct SrCreateLabelCtx
{
    char labelName[SR_LABEL_NAME_MAX_LENGTH];
    uint32_t fieldCnt;                              // feild 个数
    SrPropertyT properties[SR_LABEL_MAX_FILED_CNT]; // 属性类型数组
} SrCreateLabelCtxT;



Status DmProcessSimpleRelOpCode(OperatorCode opCode, SimpleRelExecCtxT *execCtx);

uint32_t GenSrDbId(void);
uint32_t GenSrTableId(void);

SrDbCtrlManagerT *GetDbCtrlManager(void);
bool IsDbNameExist(const char *dbName);
Status RemoveDbCtrlByName(const char *dbName);
SrDbCtrlT *DmGetDbCtrlByName(const char *dbName);
SrDbCtrlT *DmGetDbCtrlByDbId(uint32_t dbId);

bool IsLabelNameExist(SrDbCtrlT *dbCtrl, const char *labelName);

// void DmClearSingleDbCtrl(SrDbCtrlT *dbCtrl);
void DmClearAllLabels(const char *dbName);

Status SrDmCreateDb(SimpleRelExecCtxT *execCtx);
Status SrDmDropDb(SimpleRelExecCtxT *execCtx);
Status SrDmCreateTable(SimpleRelExecCtxT *execCtx);
Status SrDmInsertData(SimpleRelExecCtxT *execCtx);

#endif // __SPR_COMMON_H__