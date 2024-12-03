#ifndef EE_COMMON_H
#define EE_COMMON_H

#include "common.h"
#include "interface_common.h"
#include "dm_out_function.h"
#include "ee_out_function.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 调用接口创建container前初始化labelInfo
 * @param label
 * @return
 */
void InitLabelInfo(SrLabelT *label, SeLabelInfoT *labelInfo);

/**
 * 查询数据
 */
Status EEQueryData(QryStmtT *stmt);

#ifdef __cplusplus
}
#endif




#endif // EE_COMMON_H