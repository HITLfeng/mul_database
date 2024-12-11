#ifndef __DM_COMMON_H__
#define __DM_COMMON_H__

#include "outfunction.h"
#include "vector_util.h"
#include "common.h"
#include "db_memctx.h"
#include "dm_out_function.h"

#ifdef __cplusplus
extern "C" {
#endif





Status DmProcessSimpleRelOpCode(OperatorCode opCode, SimpleRelExecCtxT *execCtx);



SrDbCtrlManagerT *GetDbCtrlManager(void);
bool IsDbNameExist(const char *dbName);
Status RemoveDbCtrlByName(const char *dbName);


bool IsLabelNameExist(SrDbCtrlT *dbCtrl, const char *labelName);


// void DmClearSingleDbCtrl(SrDbCtrlT *dbCtrl);
void DmClearAllLabels(const char *dbName);





// ========================================
Status DMCreateTable(QryStmtT *stmt);
Status DMInsertData(QryStmtT *stmt);



#ifdef __cplusplus
}
#endif

#endif // __DM_COMMON_H__