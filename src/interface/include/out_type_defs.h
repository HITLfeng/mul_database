#ifndef __OUT_TYPE_DEFS__
#define __OUT_TYPE_DEFS__

#include <stdint.h>

#define BUF_SIZE 1024

#ifdef __cplusplus
extern "C" {
#endif

// SR 字段名长度
#define SR_FIELD_NAME_MAX_LENGTH 64
// SR 字段个数
#define SR_LABEL_MAX_FILED_CNT 30
#define SR_DB_NAME_MAX_LENGTH 128
#define SR_LABEL_NAME_MAX_LENGTH 128
#define SR_FIELD_VALUE_MAX_LENGTH 64
// json报文最大允许长度2M
#define SR_LABEL_JSON_MAX_LENGTH 2048

typedef enum OperatorCode {
    OP_ADD_TEST = 1,
    // SIMPLE RELATION
    OP_SIMREL_CREATE_DB = 10,
    OP_SIMREL_DROP_DB,
    OP_SIMREL_CREATE_TABLE,
    OP_SIMREL_DROP_TABLE,
    OP_SIMREL_INSERT_DATA,
    OP_SIMREL_DELETE_DATA,
    OP_SIMREL_QUERY_DATA,
    OP_SIMREL_DFX_DB_DESC,
    OP_SIMREL_QUERY_TABLE, // 内部调用
    OP_SIMREL_BUTT,

    // END
    OP_BUTT,
} OperatorCode;

typedef struct MsgBufRequestHead {
    OperatorCode opCode;
    uint32_t requestBufLen;
} MsgBufRequestHeadT;

typedef struct MsgBufResponseHead {
    uint32_t status; // 服务端返回值
    uint32_t responseBufLen;
} MsgBufResponseHeadT;

typedef struct MsgBufRequest {
    OperatorCode opCode;
    uint32_t requestBufLen;
    char requestMsg[BUF_SIZE];
} MsgBufRequestT;

typedef struct MsgBufResponse {
    uint32_t status; // 服务端返回值
    uint32_t responseBufLen;
    char requestMsg[BUF_SIZE];
} MsgBufResponseT;

typedef struct KVConnect {
    int socketFd;
    // int fd;
    // char ip[16];
    // uint16_t port;
} DbConnectT;

// ************************************
// SIMPLERELATION 相关类型定义 start
// ************************************

typedef enum FiledType {
    SR_LABEL_FILED_TYPE_INT32 = 0,
    SR_LABEL_FILED_TYPE_UINT32 = 0,
    // SR_LABEL_FILED_TYPE_FLOAT,
    SR_LABEL_FILED_TYPE_STRING,
    SR_LABEL_FILED_TYPE_BUTT,
} FiledTypeT;

// 这个客户端创建的时候填写
typedef struct SrDbCreateLabelCtx {
    uint32_t fieldCnt;                                   // feild 个数
    FiledTypeT fieldType[SR_LABEL_MAX_FILED_CNT]; // 属性类型数组
} SrDbCreateLabelCtxT;

// simple rel 相关传递机构体
typedef struct SimpleRelExecCtx {
    uint32_t dbId;
    uint32_t labelId;
    char dbName[SR_DB_NAME_MAX_LENGTH];
    char labelName[SR_LABEL_NAME_MAX_LENGTH];
    char labelJson[SR_LABEL_JSON_MAX_LENGTH];
} SimpleRelExecCtxT;

typedef struct RunCtx {
    OperatorCode opCode;
    void *entry;    // EE层根据opCode将runtime分发的信息解析并存入结构体中
    void *retEntry; // runtime层根据opcode解析EE层返回的结果 解析完后请手动释放
    uint32_t retEntryBufLen;
    uint32_t entryLen;
    uint32_t currDbId;    // 当前正在操作的数据库ID
    uint32_t currLabelId; // 当前正在操作的labelID
    uint32_t currLabelFldCnt; // 当前正在操作的label字段个数
} RunCtxT;

typedef RunCtxT QryStmtT;

// 客户端使用，用来辅助插入数据
// 建表后自动填充并返回改结构体
// 可调用函数接口返回改结构体 TODO
typedef struct CliProperty {
    FiledTypeT type;
    uint32_t fldSize; // 字段长度 只支持定长
    uint8_t value[SR_FIELD_VALUE_MAX_LENGTH]; // 字段值
    uint8_t fldName[SR_FIELD_NAME_MAX_LENGTH]; // 字段名
} CliPropertyT;
typedef struct CliTableSchema {
    uint32_t dbId;
    uint32_t labelId;
    CliPropertyT properties[SR_LABEL_MAX_FILED_CNT];
    uint32_t propertyCnt;
} CliTableSchemaT;

typedef struct CliStmt {
    OperatorCode opCode;
    uint32_t dbId;
    uint32_t labelId;
    DbConnectT *conn;
    CliTableSchemaT *tableSchema; // 缓存某次操作的schema信息 后续考虑优化为共享内存方案，可以减少通信开销
} CliStmtT; // 一个stmt只能操作一个DB内的一张表


// ************************************
// SIMPLERELATION 相关类型定义 end
// ************************************

#ifdef __cplusplus
}
#endif

#endif // __OUT_TYPE_DEFS__