#include "ee_common.h"
#include "dm_common.h"
#include "se_out_function.h"

// 创建DB 表容器
Status EECreateLabelContainer(SeLabelInfoT *labelInfo)
{
    Status ret = HeapContainerCreate(labelInfo);
    if (ret != GMERR_OK) {
        return ret;
    }
    return GMERR_OK;
}