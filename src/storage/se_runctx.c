#include "se_common.h"

#define SE_RUNCTX_NAME "se_runctx_name"

SERunCtxT *g_seRunCtx; // 全局唯一 SE层运行上下文

SERunCtxT *SEGetRunCtx() {
    DB_ASSERT(g_seRunCtx);
    return g_seRunCtx;
}


Status SEInitRunCtxInner(SERunCtxT *seRunCtx, DbMemCtxT *memCtx) {
//    seRunCtx->containerMap;
    DbHashMapT *map = NULL;
    Status ret = DbHashMapCreate(&map, DbHashUInt32, DbCmpUInt32, memCtx)
    if (ret != GMERR_OK) {
        return ret;
    }
    seRunCtx->memCtx = memCtx;
    return GMERR_OK;
}

/**
* 初始化 全局 存储运行上下文
* 使用 dataMemCtx
*/
Status SEInitRunCtx()
{
    DbMemCtxT *dataMemCtx = DbGetDataMemCtx();
    DB_ASSERT(dataMemCtx != NULL);
    // 创建 seRunCtx memCtx
    DbMemCtxT *seRunCtxMemCtx = NULL;
    Status ret = DbCreateMemCtx(dataMemCtx, SE_RUNCTX_NAME, &seRunCtxMemCtx);
    if (ret != GMERR_OK) {
        return ret;
    }
    SERunCtxT *seRunCtx = DbDynMemCtxAlloc(dataMemCtx, sizeof(SERunCtxT));
    if (seRunCtx == NULL) {
        log_error("alloc se run ctx memory error. alloc size is %u.", sizeof(SERunCtxT));
        return GMERR_MEMORY_ALLOC_FAILED;
    }
    ret = SEInitRunCtxInner(seRunCtx, seRunCtxMemCtx);
    if (ret != GMERR_OK) {
        return ret;
    }
    g_seRunCtx = seRunCtx;
    return GMERR_OK;
}