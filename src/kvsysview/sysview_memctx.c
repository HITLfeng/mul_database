#include "common.h"
#include "out_type_defs.h"
#include "seri_utils.c"
#include "client_common.h"
#include "outfunction.h"
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

void DbSysFillRequestMsgBuf(SysviewQueryCtxBaseT *ctxBase, char **bufCursor)
{
    MemOperatorT op = ctxBase->op;

}

Status DbSysQureyExec(DbConnectT *conn, SysviewQueryCtxBaseT *ctxBase) {
    DB_POINT2(conn, ctxBase);


    // 申请栈内存
    MsgBufRequestT msgBuf = {0};
    SysInitMsgBuf(&msgBuf, OP_SYSVIEW_EDIT);

    // len/dbname
    char *bufCursor = msgBuf.requestMsg;

    SeriUint32(msgBuf.requestMsg, (uint32_t)op);

    UsrDataBase SysviewRes = {0};
    Status ret = KVCSendRequestAndRecvResponse(conn, &msgBuf, SrParseCreateDbRspCb, (UsrDataBaseT *)&createDbRes);
    if (ret != GMERR_OK) {
        log_error("CLIENT: DbSysQureyExec fail.");
        return ret;
    }
    if (SysviewRes.ret != GMERR_OK) {
        log_error("SERVER: DbSysQureyExec fail, ret is %u.", createDbRes.ret);
        return GMERR_OK;
    }
    return GMERR_OK;
}

typedef struct SysviewMemCtxAlloc {
    uint32_t allocSize;
    uint32_t allocTime; // 申请多少次
} SysviewMemCtxAllocT;

Status SysviewMain()
{
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

        fgets(inputMsg, sizeof(input), stdin); // 使用fgets可以接受空格

        // 去掉换行符
        inputMsg[strcspn(inputMsg, "\n")] = 0;

        if (strcmp(inputMsg, "q") == 0) {
            printf("quit now.\n");
            break;
        } else if (strcmp(input, "1") == 0) {
            SysviewMemCtxAllocT sca = {0};
            printf(">> please input allocSize: ");
            scanf("%u", &sca.allocSize);
            if (sca.allocSize == 0) {
                printf("allocsize should not eq 0.\n");
                system("clear");
                continue;
            }
            printf(">> please input allocTime: ");
            scanf("%u", &sca.allocTime);
            if (sca.allocTime == 0) {
                printf("allocTime should not eq 0.\n");
                system("clear");
                continue;
            }

        } else if (strcmp(input, "2") == 0) {
            printf("DbDymMemCtxFree is not support now!\n");
        } else {
        }
        system("clear");
    }


    KVCDisconnect(conn);
}