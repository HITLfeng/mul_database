#include "main_worker.h"

void *client_handler(void *arg)
{
    int clnt_sock = *((int *)arg);
    // 用户有效报文长度不超过1024
    char message[sizeof(MsgBufRequestT)];
    int str_len;
    normal_info("thread id: %d is deal with client %d\n", pthread_self(), clnt_sock);

    while (true)
    {
        memset(message, 0, sizeof(MsgBufRequestT));
        // 接收客户端发送的消息
        str_len = read(clnt_sock, message, sizeof(MsgBufRequestT));
        if (str_len == 0)
        {
            normal_info("thread id: %d msg -- client %d closed connection\n", pthread_self(), clnt_sock);
            // 客户端关闭连接
            close(clnt_sock);
            return NULL;
        }
        // message[str_len] = 0;

        // printf("收到客户端的消息：%s", message);

        // messgae即时输入也是输出哦
        RTProcessMain(message, str_len);

        // 发送消息给客户端
        // printf("请输入要发送的消息：");
        // fgets(message, sizeof(MsgBufRequestT), stdin);
        // str_len = strlen(message);
        int sendRet = write(clnt_sock, message, sizeof(MsgBufResponseT));
        if (sendRet == -1)
        {
            error_info("thread id: %d msg -- send msg to client %d failed\n", pthread_self(), clnt_sock);
            // 客户端关闭连接
            close(clnt_sock);
            return NULL;
        }
    }

    return NULL;
}

Status MainWorkerStart()
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    // 创建socket对象
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_info("MainWorkerStart, get socket error");
        return GMERR_SOCKET_FAILED;
    }

    // 绑定IP地址和端口
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(12345);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_info("MainWorkerStart, bind socket error");
        return GMERR_SOCKET_FAILED;
    }

    // 开始监听
    if (listen(serv_sock, 5) == -1)
    {
        error_info("MainWorkerStart, listen socket error");
        return GMERR_SOCKET_FAILED;
    }

    // 初始化内存池
    if (KVMemoryPoolInit() != GMERR_OK)
    {
        error_info("MainWorkerStart, KVMemoryPoolInit failed");
        return GMERR_STORAGE_MEMPOOL_INIT_FAILED;
    }

    while (1)
    {
        clnt_addr_size = sizeof(clnt_addr);
        // 接受客户端连接请求
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
        {
            error_info("MainWorkerStart, accept socket error");
            return GMERR_SOCKET_FAILED;
        }

        // 创建新线程处理客户端请求
        normal_info("MainWorkerStart, new client connected, client ip: %s, port: %d", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
        pthread_t t_id;
        pthread_create(&t_id, NULL, client_handler, (void *)&clnt_sock);
        pthread_detach(t_id); // 分离线程，使其结束后自动回收资源
    }

    // 释放内存池资源
    KVMemoryPoolUninit();
    // 关闭socket
    close(serv_sock);

    return GMERR_OK;
}
