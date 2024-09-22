#include "main_worker.h"


static uint8_t *GetUsrMsgBufPostion(char *message)
{
    return (uint8_t *)message + sizeof(MsgBufRequestHeadT);
}

void RTSetRequestHead(char *msgRequest, Status status, const char *responseBuf, uint32_t responseBuflen)
{
    memset(msgRequest, 0, sizeof(MsgBufRequestT));
    MsgBufResponseHeadT *msgResponseHead = (MsgBufResponseHeadT *)msgRequest;
    msgResponseHead->status = status;
    if (status != GMERR_OK || responseBuf == NULL)
    {
        msgResponseHead->responseBufLen = 0;
        return;
    }
    DB_ASSERT(responseBuflen <= BUF_SIZE);
    msgResponseHead->responseBufLen = responseBuflen;
    memcpy(GetUsrMsgBufPostion((uint8_t *)msgResponseHead), responseBuf, responseBuflen);
}

void RTProcessMain(char *message, uint32_t str_len)
{
    DB_POINT(message);
    Status ret = GMERR_OK;
    // 1. 解析相关信息
    MsgBufRequestHeadT *msg_head = (MsgBufRequestHeadT *)message;
    if (msg_head->opCode >= OP_BUTT)
    {
        // 未知的opCode
        error_info("parse request msg buffer failed. unknown opCode. type: %d", msg_head->opCode);
        ret = GMERR_RUNTIME_UNKNOWN_OPCODE;
        RTSetRequestHead(message, ret, NULL, 0);
        return;
    }
    // 2. 调用对应的处理函数
    uint8_t *reslutBuf = (uint8_t *)malloc(BUF_SIZE);
    if (reslutBuf == NULL)
    {
        error_info("parse request msg buffer failed. malloc failed.");
        ret = GMERR_MEMORY_ALLOC_FAILED;
        RTSetRequestHead(message, ret, NULL, 0);
        return;
    }
    memset(reslutBuf, BUF_SIZE, 0);
    ret = RTProcessOpcode(msg_head->opCode, GetUsrMsgBufPostion(message), reslutBuf, BUF_SIZE);

    // 3. 设置返回报文信息
    RTSetRequestHead(message, ret, reslutBuf, BUF_SIZE);
    free(reslutBuf);
    return;
}