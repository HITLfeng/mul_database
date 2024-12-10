#ifndef __DM_OUT_FUNCTION_H__
#define __DM_OUT_FUNCTION_H__

#include "vector_util.h"
#include "out_type_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DMVALUE 相关接口
 */

typedef DbValueT DmValueT;
typedef DbValueTypeT DmValueTypeT;
void DmSetValue(DmValueT *dmValue, const void *value, uint32_t valueLen, DmValueTypeT type);
/**
 * 比较两个DMVALUE 类型不一致会出错
 * @param dmValueLeft 左值
 * @param dmValueRight 右值
 * @return 1 代表 >  0 代表 =  -1 代表 <
 */
int32_t DmCmpValue(DmValueT *dmValueLeft, DmValueT *dmValueRight);


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
    // FixedHeapT heapRow; // 存储记录数
    uint32_t recordLen; // 记录长度 TODO: 还没有赋值
} SrLabelT;

typedef struct SrCreateLabelCtx
{
    char labelName[SR_LABEL_NAME_MAX_LENGTH];
    uint32_t fieldCnt;                              // feild 个数
    SrPropertyT properties[SR_LABEL_MAX_FILED_CNT]; // 属性类型数组
} SrCreateLabelCtxT;

/**
 * 根据name 获取DBCtrl
 */
SrDbCtrlT *DmGetDbCtrlByName(const char *dbName);

/**
 * 根据dbId 获取DBCtrl
 */
SrDbCtrlT *DmGetDbCtrlByDbId(uint32_t dbId);

uint32_t GenSrDbId(void);

uint32_t GenSrTableId(void);

/**
 * 根据labelId 获取labelCtrl
 */
SrLabelT *DmGetLabelCtrlByLabelId(SrDbCtrlT *dbCtrl, uint32_t labelId);


#ifdef __cplusplus
}
#endif

#endif // __DM_OUT_FUNCTION_H__