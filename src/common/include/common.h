#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

// 调试打印开关
// #define __DEBUG
/*
// #ifdef __DEBUG
// #define normal_info(format, ...) printf("["__FILE__                                        \
//                                         "][Line: %d][%s]: \033[32m" format "\033[32;0m\n", \
//                                         __LINE__, __func__, ##__VA_ARGS__)
// #define warning_info(format, ...) printf("["__FILE__                                        \
//                                          "][Line: %d][%s]: \033[33m" format "\033[32;0m\n", \
//                                          __LINE__, __func__, ##__VA_ARGS__)
// #define error_info(format, ...) printf("["__FILE__                                        \
//                                        "][Line: %d][%s]: \033[31m" format "\033[32;0m\n", \
//                                        __LINE__, __func__, ##__VA_ARGS__)
// #else
// #define normal_info(format, ...)
// #define warn_info(format, ...)
// #define error_info(format, ...)
// #endif

*/
// 规格约束


#define STRLEN(str) (strlen(str) + 1)
/*
 * error no
 */
#define true 1
#define false 0
typedef uint32_t Status;

// common 错误码
#define GMERR_OK 0
#define GMERR_SOCKET_FAILED 1000001
#define GMERR_MEMORY_ALLOC_FAILED 1000002
#define GMERR_KV_MEMORY_ALLOC_FAILED 1000003


// 第三方库出错
#define GMERR_JSON_LIB_ERROR 1000004

// memCtx 出错
// 没有页可以分配了
#define GMERR_MEMCTX_ERROR_NO_PAGE_ALLOC 1000500
// 获取fix size level失败
#define GMERR_MEMCTX_ERROR_INVAILD_LEVEL_INDEX 1000501
#define GMERR_MEMCTX_ERROR_UNEXCEPT_NULL 1000502

#define GMERR_KV_NOT_SUPPORT 1000999
// 测试错误码
#define GMERR_ADD_TEST_INVAILD_OPTION 1001003


// RUNTIME 模块错误码
#define GMERR_RUNTIME_UNKNOWN_OPCODE 1002000
#define GMERR_RUNTIME_INVAILD_BUFLENGTH 1002001

#define GMERR_SRDB_OP_CODE_INVAILD 1002101



// CLIENT 模块错误码
#define GMERR_CLIENT_SOCKET_FAILED 2001001
#define GMERR_CLIENT_CONNECT_FAILED 2001002
#define GMERR_CLIENT_SEND_FAILED 2001003
#define GMERR_CLIENT_RECV_FAILED 2001004
#define GMERR_CLIENT_START_SERVER_FAILED 2001005
#define GMERR_CLIENT_STOP_SERVER_FAILED 2001006
#define GMERR_CLIENT_MEMORY_ALLOC_FAILED 2001007


// STORAGE 模块错误码
#define GMERR_STORAGE_MEMPOOL_INIT_FAILED 3001001
#define GMERR_STORAGE_MEMCTX_INIT_FAILED 3001002

// ****************************************
// DATAMODEL 模块错误码
// ****************************************
#define GMERR_DATAMODEL_SRDB_NAME_NULL 4001001
#define GMERR_DATAMODEL_SRDB_NAME_TOO_LONG 4001002
#define GMERR_DATAMODEL_SRDB_GET_GMANGER_FAILED 4001003
#define GMERR_DATAMODEL_SRDB_NAME_EXISTED 4001004
// DB 名字不存在
#define GMERR_DATAMODEL_SRDB_NAME_NOT_EXISTED 4001005
#define GMERR_DATAMODEL_SRDB_LIST_EXCEPT_NULL 4001006
#define GMERR_DATAMODEL_SRDB_ID_NOT_EXISTED 4001007
 
#define GMERR_DATAMODEL_SRLABEL_NAME_EXISTED 4001506
#define GMERR_DATAMODEL_SRLABEL_ID_NOT_EXISTED 4001507
#define GMERR_DATAMODEL_SR_CREATE_LABEL_JSON_NULL 4001508
#define GMERR_DATAMODEL_SR_CREATE_LABEL_JSON_INVAILD 4001509
#define GMERR_DATAMODEL_SR_CREATE_LABEL_FIELDS_OUTRANGE 4001510

// ****************************************
// EE 模块错误码
// ****************************************
#define GMERR_EE_UNKNOWN_OPCODE 5001001
#define GMERR_EE_INVAILD_BUFLENGTH 5001002
#define GMERR_EE_INVAILD_OPCODE 5001003
#define GMERR_EE_INVAILD_PARAM 5001004
#define GMERR_EE_INVAILD_PARAM2 5001005
#define GMERR_EE_INVAILD_PARAM3 5001006
#define GMERR_EE_INVAILD_PARAM4 5001007
#define GMERR_EE_INVAILD_PARAM5 5001008
#define GMERR_EE_INVAILD_PARAM6 5001009
#define GMERR_EE_INVAILD_PARAM7 5001010
#define GMERR_EE_INVAILD_PARAM8 5001011
#define GMERR_EE_INVAILD_PARAM9 5001012
#define GMERR_EE_INVAILD_PARAM10 5001013


/*
 * 断言非空函数
 */
#define DB_ASSERT(pointer) assert(pointer)

inline static void DB_POINT(const void *pointer1)
{
    assert(pointer1 != NULL);
}

inline static void DB_POINT2(const void *pointer1, const void *pointer2)
{
    assert(pointer1 != NULL);
    assert(pointer2 != NULL);
}

inline static void DB_POINT3(const void *pointer1, const void *pointer2, const void *pointer3)
{
    assert(pointer1 != NULL);
    assert(pointer2 != NULL);
    assert(pointer3 != NULL);
}


#ifdef __cplusplus
}
#endif

#endif // __COMMON_H__
