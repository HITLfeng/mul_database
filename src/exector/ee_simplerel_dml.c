#include "ee_common.h"
#include "spr_common.h"
#include "se_out_function.h"

// 创建DB 表容器
Status EECreateLabelContainer(SrLabelT *label)
{
    Status ret = HeapContainerCreate(label);
    if (ret != GMERR_OK) {
        return ret;
    }
    return GMERR_OK;
}