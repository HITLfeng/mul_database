#ifndef __DB_MEMCTX_H__
#define __DB_MEMCTX_H__

#ifdef __cplusplus
extern "C" {
#endif
    
/*
 * 启动服务器时自动调用 初始化memctx管理结构
 */
Status DbInitMemManager();

#ifdef __cplusplus
}
#endif

#endif // __DB_MEMCTX_H__
