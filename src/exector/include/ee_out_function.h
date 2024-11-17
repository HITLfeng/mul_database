#ifndef EE_OUT_FUNCTION_H
#define EE_OUT_FUNCTION_H

#include "ee_common.h"
#include "out_type_defs.h"
#include "interface_common.h"

#ifdef __cplusplus
extern "C" {
#endif

Status EEProcessRuntimeOpCode(QryStmtT *stmt);

#ifdef __cplusplus
}
#endif

#endif // EE_OUT_FUNCTION_H