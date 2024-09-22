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
#define __DEBUG

#ifdef __DEBUG
#define normal_info(format, ...) printf("["__FILE__                                        \
                                        "][Line: %d][%s]: \033[32m" format "\033[32;0m\n", \
                                        __LINE__, __func__, ##__VA_ARGS__)
#define warning_info(format, ...) printf("["__FILE__                                        \
                                         "][Line: %d][%s]: \033[33m" format "\033[32;0m\n", \
                                         __LINE__, __func__, ##__VA_ARGS__)
#define error_info(format, ...) printf("["__FILE__                                        \
                                       "][Line: %d][%s]: \033[31m" format "\033[32;0m\n", \
                                       __LINE__, __func__, ##__VA_ARGS__)
#else
#define normal_info(format, ...)
#define warn_info(format, ...)
#define error_info(format, ...)
#endif

/*
 * error no
 */
#define true 1
#define false 0
typedef uint32_t Status;

// common 错误码
#define GMERR_OK 0
#define GMERR_MEMORY_ALLOC_FAILED 1000001
#define GMERR_SOCKET_FAILED 1000002

// 测试错误码
#define GMERR_ADD_TEST_INVAILD_OPTION 1001003


// RUNTIME 模块错误码
#define GMERR_RUNTIME_UNKNOWN_OPCODE 1002000
#define GMERR_RUNTIME_INVAILD_BUFLENGTH 1002001

// CLIENT 模块错误码
#define GMERR_CLIENT_SOCKET_FAILED 2001001
#define GMERR_CLIENT_CONNECT_FAILED 2001002
#define GMERR_CLIENT_SEND_FAILED 2001003
#define GMERR_CLIENT_RECV_FAILED 2001004
#define GMERR_CLIENT_START_SERVER_FAILED 2001005
#define GMERR_CLIENT_STOP_SERVER_FAILED 2001006

// STORAGE 模块错误码
#define GMERR_STORAGE_MEMPOOL_INIT_FAILED 3001001


/*
 * 断言非空函数
 */
#define DB_ASSERT(pointer) assert(pointer)

inline static void DB_POINT(void *pointer1)
{
    assert(pointer1 != NULL);
}

inline static void DB_POINT2(void *pointer1, void *pointer2)
{
    assert(pointer1 != NULL);
    assert(pointer2 != NULL);
}

#ifdef __cplusplus
}
#endif

#endif // __COMMON_H__
