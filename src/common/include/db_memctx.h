#ifndef __DB_MEMCTX_H__
#define __DB_MEMCTX_H__

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct DbMemCtx DbMemCtxT;
/*
 * 启动服务器时自动调用 初始化memctx管理结构
 */
Status DbInitMemManager();
void *DbDynMemCtxAlloc(DbMemCtxT *memCtx, uint32_t allocSize);
void DbDynMemCtxFree(DbMemCtxT *memCtx, void *ptr);
Status DbCreateMemCtx(DbMemCtxT *memCtx, const char *name, DbMemCtxT **childMemCtx);
void DbMemCtxReset(DbMemCtxT *memCtx);
Status DbMemCtxDelete(DbMemCtxT *memCtx);

// 依次是 获取顶层memctx 元数据memctx 存储数据memctx
DbMemCtxT *DbGetTopMemCtx();
DbMemCtxT *DbGetMetaMemCtx();
DbMemCtxT *DbGetDataMemCtx();

#ifdef __cplusplus
}
#endif

#endif // __DB_MEMCTX_H__
