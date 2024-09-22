#ifndef __SPR_COMMON_H__
#define __SPR_COMMON_H__

#include "../../../interface/include/outfunction.h"
#include "../../../common/include/vector_util.h"
#include "../../../common/include/common.h"
#include "../../../common/include/kv_memory.h"

typedef struct SrLabelCtrl {
    const char *labelName;
    const uint32_t labelId;
} SrLabelCtrlT;

typedef struct SrDbCtrl {
    char *dbName;
    uint32_t dbId;
    DbVectorT labelCtrlList; // 存放 SrLabelCtrlT
} SrDbCtrlT;

typedef struct SrDbCtrlManager {
    // TODO:lock
    DbVectorT dbCtrlList; // 存放 SrDbCtrlT
} SrDbCtrlManagerT;


typedef struct SrLabel {
    const char *labelName;
    const uint32_t labelId;
    uint32_t fieldCnt; // feild 个数
    SrLabelFiledTypeT *fieldType; // 属性类型数组
} SrLabelT;

Status DmProcessSimpleRelOpCode(OperatorCode opCode, SimpleRelExecCtxT *execCtx);

uint32_t GenSrDbId(void);
uint32_t GenSrTableId(void);


SrDbCtrlManagerT *GetDbCtrlManager(void);
bool IsDbNameExist(const char *dbName);
Status RemoveDbCtrlByName(const char *dbName);

void DmClearSingleDbCtrl(SrDbCtrlT *dbCtrl);
void DmClearAllLabels(const char *dbName);


#endif // __SPR_COMMON_H__