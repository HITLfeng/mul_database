#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "../interface/include/outfunction.h"
// #include "outfunction.h"

#include "../common/include/common.h"
// #include "common.h"

uint8_t *GetUsrDataPosition(uint8_t *usrMsgBuf)
{
    return usrMsgBuf + sizeof(MsgBufResponseHeadT);
}

void *srvStartDetach(void *arg)
{
    system("kvserver");
}

CliStatus KVCSrvStart(void)
{
    pthread_t thread;
    if (pthread_create(&thread, NULL, srvStartDetach, NULL) != 0)
    {
        log_error("server start thread create error.");
        return GMERR_CLIENT_START_SERVER_FAILED;
    }
    pthread_detach(thread);
}
CliStatus KVCSrvStop(void)
{
    system("killall kvserver");
}

CliStatus KVCConnect(DbConnectT *conn)
{
    DB_POINT(conn);
    int sock;
    char message[BUF_SIZE];
    int str_len;

    struct sockaddr_in serv_addr;

    // 创建socket对象
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        log_error("client socket error.");
        return GMERR_CLIENT_CONNECT_FAILED;
    }

    // 设置服务器地址和端口
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(12345);

    // 连接服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        log_error("client connect error.");
        return GMERR_CLIENT_CONNECT_FAILED;
    }
    conn->socketFd = sock;
    log_info("client connect success.");
    return GMERR_OK;
}

CliStatus KVCDisconnect(DbConnectT *conn)
{
    DB_POINT(conn);
    close(conn->socketFd);
    return GMERR_OK;
}

CliStatus KVCSend(DbConnectT *conn, const MsgBufRequestT *msgBuf)
{
    DB_POINT(conn);
    int sendRet = write(conn->socketFd, (const void *)msgBuf, sizeof(MsgBufRequestT));
    if (sendRet == -1)
    {
        log_error("client send msgBuf error.");
        return GMERR_CLIENT_SEND_FAILED;
    }
    return GMERR_OK;
}

CliStatus KVCRecv(DbConnectT *conn, MsgBufResponseT *msgBuf)
{
    DB_POINT(conn);
    char message[sizeof(MsgBufResponseT)];
    int recvLen = read(conn->socketFd, &message, sizeof(MsgBufResponseT));
    if (recvLen == -1)
    {
        log_error("client recv msgBuf error.");
        return GMERR_CLIENT_RECV_FAILED;
    }
    // message[recvLen] = 0;
    memcpy(msgBuf, &message, sizeof(MsgBufResponseT));

    return GMERR_OK;
}