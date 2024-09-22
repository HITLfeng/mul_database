#ifndef __MAIN_WORKER_H__
#define __MAIN_WORKER_H__

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include "out_type_defs.h"
#include "../../common/include/common.h"
#include "../../common/include/kv_memory.h"

#ifdef __cplusplus
extern "C"
{
#endif

Status MainWorkerStart(void);

// RT模块处理函数入口，根据opCode调用不同的处理函数
// message 输入输出函数
void RTProcessMain(char *message, uint32_t str_len);

// 根据opCode调用不同的处理函数 resultBuf 输出型参数
Status RTProcessOpcode(OperatorCode opCode, char *usrMsg, char *resultBuf, uint32_t bufLen);


#ifdef __cplusplus
}
#endif

#endif // __MAIN_WORKER_H__