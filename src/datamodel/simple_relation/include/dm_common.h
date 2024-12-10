#ifndef __DM_COMMON_H__
#define __DM_COMMON_H__

#include "../../../interface/include/outfunction.h"
#include "../../../common/include/vector_util.h"
#include "../../../common/include/common.h"
#include "db_memctx.h"
#include "interface_common.h"
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

Status DMSrCreateDb(QryStmtT *stmt);
Status DMSrDropDb(QryStmtT *stmt);
Status DMSrCreateTable(QryStmtT *stmt);
Status DMSrInsertData(QryStmtT *stmt);

// Status DMSrGetDbDesc(QryStmtT *stmt);
Status DMSrQueryTable(QryStmtT *stmt);

Status DMExecSysviewEdit(QryStmtT *stmt);



// ========================================
Status DMCreateTable(QryStmtT *stmt);
Status DMInsertData(QryStmtT *stmt);



#ifdef __cplusplus
}
#endif

#endif // __DM_COMMON_H__