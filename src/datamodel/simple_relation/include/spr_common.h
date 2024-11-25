#ifndef __SPR_COMMON_H__
#define __SPR_COMMON_H__

#include "../../../interface/include/outfunction.h"
#include "../../../common/include/vector_util.h"
#include "../../../common/include/common.h"
#include "../../../common/include/kv_memory.h"
#include "db_memctx.h"
#include "se_out_function.h"
#include "interface_common.h"
#include "ee_out_function.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SrDbCtrl
{
    char *dbName;
    uint32_t dbId;
    DbVectorT labelCtrlList; // 存放 SrLabelT
    DbMemCtxT *memCtx;
} SrDbCtrlT;

typedef struct SrDbCtrlManager
{
    // TODO:lock
    DbVectorT dbCtrlList; // 存放 SrDbCtrlT
    DbMemCtxT *memCtx; // db_ctrl_manager memctx 父节点是 meta memctx
} SrDbCtrlManagerT;

typedef struct SrProperty
{
    char fieldName[SR_FIELD_NAME_MAX_LENGTH];
    FiledTypeT fieldType;
    uint32_t fieldSize; // 字段长度 只支持定长
    uint32_t fldOffset; // 字段偏移量，用于快速读取buf
} SrPropertyT;

typedef struct SrLabel
{
    uint32_t dbId;
    const char *labelName;
    uint32_t labelId;
    uint32_t fieldCnt;       // feild 个数
    SrPropertyT *properties; // 属性数组
    FixedHeapT heapRow; // 存储记录数
    uint32_t recordLen; // 记录长度 TODO: 还没有赋值
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

SrLabelT *DmGetLabelCtrlByLabelId(SrDbCtrlT *dbCtrl, uint32_t labelId);

// void DmClearSingleDbCtrl(SrDbCtrlT *dbCtrl);
void DmClearAllLabels(const char *dbName);

Status DMSrCreateDb(QryStmtT *stmt);
Status DMSrDropDb(QryStmtT *stmt);
Status DMSrCreateTable(QryStmtT *stmt);
Status DMSrInsertData(QryStmtT *stmt);

Status DMSrGetDbDesc(QryStmtT *stmt);
Status DMSrQueryTable(QryStmtT *stmt);

Status DMExecSysviewEdit(QryStmtT *stmt);



// ========================================
Status DMCreateTable(QryStmtT *stmt);
Status DMInsertData(QryStmtT *stmt);



#ifdef __cplusplus
}
#endif

#endif // __SPR_COMMON_H__