#include "include/spr_common.h"

SrDbCtrlManagerT *g_srDbCtrlManager = NULL;

// TODO: 改为原子操作
#define TABLE_ID_DIFF 400000
uint32_t g_dbGenNum = 100100;

uint32_t GenSrDbId(void)
{
    return g_dbGenNum++;
}

uint32_t GenSrTableId(void)
{
    return TABLE_ID_DIFF + g_dbGenNum++;
}

SrDbCtrlManagerT *GetDbCtrlManager(void)
{
    if (g_srDbCtrlManager != NULL)
    {
        return g_srDbCtrlManager;
    }
    // 创建并初始化
    g_srDbCtrlManager = KVMemAlloc(sizeof(SrDbCtrlManagerT));
    if (g_srDbCtrlManager == NULL)
    {
        log_error("alloc db_ctrl_manager failed.");
        return NULL;
    }

    Status ret = DbVectorInit(&g_srDbCtrlManager->dbCtrlList, sizeof(SrDbCtrlT));
    if (ret != GMERR_OK)
    {
        log_error("init g_srDbCtrlManager failed.");
        return NULL;
    }
    return g_srDbCtrlManager;
}

bool IsDbNameExist(const char *dbName)
{
    SrDbCtrlManagerT *dbCtrlManager = GetDbCtrlManager();
    if (dbCtrlManager == NULL)
    {
        // 理论上不会走到这里
        log_error("get db_ctrl_manager failed when judge IsDbNameExist.");
        return false;
    }
    // 遍历数据库列表
    for (uint32_t i = 0; i < DbVectorGetSize(&dbCtrlManager->dbCtrlList); i++)
    {
        SrDbCtrlT *dbCtrl = (SrDbCtrlT *)DbVectorGetItem(&dbCtrlManager->dbCtrlList, i);
        if (dbCtrl == NULL)
        {
            // 理论上不会走到这里
            log_error("get dbCtrl failed when judge IsDbNameExist.");
            return false;
        }
        if (strcmp(dbCtrl->dbName, dbName) == 0)
        {
            return true;
        }
    }
    return false;
}