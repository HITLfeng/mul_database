#include "common.h"
#include "out_type_defs.h"
#include "seri_utils.h"
#include "client_common.h"
#include "outfunction.h"
#include "kvsysview_common.h"
#include <string.h>
#include <stdlib.h>

void SysInitMsgBuf(MsgBufRequestT *msgBuf, OperatorCode opCode) {
    DB_POINT(msgBuf);
    memset(msgBuf, 0, sizeof(MsgBufRequestT));
    msgBuf->opCode = opCode;
    msgBuf->requestBufLen = BUF_SIZE;
}

// create db 函数
void SysParseQueryCb(uint8_t **respBuf, UsrDataBaseT *result) {
    DB_POINT2(respBuf, result);
    // 1.解析服务端返回值
    MsgBufResponseHeadT *respHead = (MsgBufResponseHeadT *)*respBuf;
    if (respHead->status != GMERR_OK) {
        log_error("SrParseCreateDbRspCb error, server status = %d", respHead->status);
        return;
    }
    // 2.解析数据
    *respBuf += sizeof(MsgBufResponseHeadT);
    result->ret = respHead->status;
}

typedef struct SysviewQueryCtxBase {
    MemOperatorT op;
} SysviewQueryCtxBaseT;

typedef struct SysviewQueryCtxAllocMemctx {
    MemOperatorT op;
    uint32_t allocSize;
    uint32_t allocTime; // 申请多少次
} SysviewQueryCtxAllocMemctxT;

void DbSysFillRequestMsgBuf(SysviewQueryCtxBaseT *ctxBase, uint8_t **bufCursor) {
    MemOperatorT op = ctxBase->op;
    SeriUint32M(bufCursor, (uint32_t)op);
    switch (op) {
    case MEM_OP_ALLOC: {
        SysviewQueryCtxAllocMemctxT *ctx = (SysviewQueryCtxAllocMemctxT *)(ctxBase);
        SeriUint32M(bufCursor, ctx->allocSize);
        SeriUint32M(bufCursor, ctx->allocTime);
        break;
    }
    case MEM_OP_FREE:
        break;
    case MEM_OP_CREATE_MEMCTX:
        break;
    case MEM_OP_RESET_MEMCTX:
        break;
    case MEM_OP_DELETE_MEMCTX:
        break;
    default:
        break;
    }
}

Status DbSysQureyExec(DbConnectT *conn, SysviewQueryCtxBaseT *ctxBase) {
    DB_POINT2(conn, ctxBase);

    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    SysInitMsgBuf(&msgBuf, OP_SYSVIEW_EDIT);

    // len/dbname
    uint8_t *bufCursor = (uint8_t *)msgBuf.requestMsg;
    DbSysFillRequestMsgBuf(ctxBase, &bufCursor);

    UsrDataBaseT SysviewRes = {0};
    Status ret = KVCSendRequestAndRecvResponse(conn, &msgBuf, SysParseQueryCb, (UsrDataBaseT *)&SysviewRes);
    if (ret != GMERR_OK) {
        log_error("CLIENT: DbSysQureyExec fail.");
        return ret;
    }
    if (SysviewRes.ret != GMERR_OK) {
        log_error("SERVER: DbSysQureyExec fail, ret is %u.", SysviewRes.ret);
        return GMERR_OK;
    }
    return GMERR_OK;
}

// typedef struct SysviewMemCtxAlloc {
//     uint32_t allocSize;
//     uint32_t allocTime; // 申请多少次
// } SysviewMemCtxAllocT;

Status SysviewMain() {
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    DB_ASSERT(conn != NULL);
    memset(conn, 0, sizeof(DbConnectT));

    Status ret = KVCConnect(conn);
    if (ret != GMERR_OK) {
        return ret;
    }

    char inputMsg[64];
    while (true) {
        printf(">> please input you choice: \n");

        fgets(inputMsg, sizeof(inputMsg), stdin); // 使用fgets可以接受空格

        // 去掉换行符
        inputMsg[strcspn(inputMsg, "\n")] = 0;

        if (strcmp(inputMsg, "q") == 0) {
            printf("quit now.\n");
            break;
        } else if (strcmp(inputMsg, "1") == 0) {
            SysviewQueryCtxAllocMemctxT sca = {0};
            sca.op = MEM_OP_ALLOC;
            printf(">> please input allocSize: ");
            scanf("%u", &sca.allocSize);
            if (sca.allocSize == 0) {
                printf("allocsize should not eq 0.\n");
                // system("clear");
                continue;
            }
            printf(">> please input allocTime: ");
            scanf("%u", &sca.allocTime);
            if (sca.allocTime == 0) {
                printf("allocTime should not eq 0.\n");
                // system("clear");
                continue;
            }
            ret = DbSysQureyExec(conn, (SysviewQueryCtxBaseT *)&sca);
            if (ret != GMERR_OK) {
                printf("DbSysQureyExec fail and ret is %u.\n", ret);
                // system("clear");
                continue;
            }
            printf("DbSysQureyExec success.\n");
        } else if (strcmp(inputMsg, "2") == 0) {
            printf("DbDymMemCtxFree is not support now!\n");
        } else {
            printf("Not support now!\n");
        }
        // system("clear");
    }

    KVCDisconnect(conn);
}