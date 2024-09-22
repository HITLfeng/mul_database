#ifndef __OUT_TYPE_DEFS__
#define __OUT_TYPE_DEFS__

#include <stdint.h>

#define BUF_SIZE 1024

#ifdef __cplusplus
extern "C"
{
#endif


#define SR_LABEL_MAX_FILED_CNT 30
#define SR_DBNAME_MAX_LENGTH 128


typedef enum OperatorCode
{
    OP_ADD_TEST = 0,
    // SIMPLE RELATION 
    OP_SIMREL_CREATE_DB = 10,
    OP_SIMREL_DROP_DB,
    OP_SIMREL_CREATE_TABLE,
    OP_SIMREL_DROP_TABLE,
    OP_SIMREL_INSERT_DATA,
    OP_SIMREL_DELETE_DATA,
    OP_SIMREL_QUERY_DATA,

    // END
    OP_BUTT,
} OperatorCode;


typedef struct MsgBufRequestHead
{
    OperatorCode opCode;
    uint32_t requestBufLen;
} MsgBufRequestHeadT;

typedef struct MsgBufResponseHead
{
    uint32_t status; // 服务端返回值
    uint32_t responseBufLen;
} MsgBufResponseHeadT;

typedef struct MsgBufRequest
{
    OperatorCode opCode;
    uint32_t requestBufLen;
    char requestMsg[BUF_SIZE];
} MsgBufRequestT;

typedef struct MsgBufResponse
{
    uint32_t status; // 服务端返回值
    uint32_t responseBufLen;
    char requestMsg[BUF_SIZE];
} MsgBufResponseT;

typedef struct KVConnect
{
    int socketFd;
    // int fd;
    // char ip[16];
    // uint16_t port;
} DbConnectT;

typedef struct UsrResultBase {
    uint32_t ret;
} UsrResultBaseT;

typedef struct UsrResultCalc {
    uint32_t ret;
    uint32_t calcAns;
} UsrResultCalcT;


// ************************************
// SIMPLERELATION 相关类型定义 start
// ************************************

typedef struct UsrResultCreateDb {
    uint32_t ret;
    uint32_t dbId;
} UsrResultCreateDbT;

typedef enum SrLabelFiledType
{
    SR_LABEL_FILED_TYPE_INT = 0,
    // SR_LABEL_FILED_TYPE_FLOAT,
    SR_LABEL_FILED_TYPE_STRING,
    SR_LABEL_FILED_TYPE_BUTT,
} SrLabelFiledTypeT;

// 这个客户端创建的时候填写
typedef struct SrDbCreateLabelCtx {
    uint32_t fieldCnt; // feild 个数
    SrLabelFiledTypeT fieldType[SR_LABEL_MAX_FILED_CNT]; // 属性类型数组
} SrDbCreateLabelCtxT;

// ************************************
// SIMPLERELATION 相关类型定义 end
// ************************************

#ifdef __cplusplus
}
#endif



#endif  // __OUT_TYPE_DEFS__