#include "../../interface/include/outfunction.h"
#include "../../common/include/db_vector.h"

typedef struct SrLabelCtrl {
    const char *labelName;
    const uint32_t labelId;
} SrLabelCtrlT;

typedef struct SrDbCtrl {
    const char *dbName;
    const uint32_t dbId;
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
