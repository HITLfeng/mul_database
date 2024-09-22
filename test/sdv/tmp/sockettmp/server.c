#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024

void *client_handler(void *arg);

typedef enum OperatorCode
{
    OP_ADD_TEST = 0,
    OP_BUTT,
} OperatorCode;

typedef struct MsgBuf
{
    OperatorCode opCode;
    uint32_t bufLen;
    char msg[BUF_SIZE];
} MsgBufT;

int main()
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    // 创建socket对象
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    // 绑定IP地址和端口
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(12345);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("bind() error");
        exit(1);
    }

    // 开始监听
    if (listen(serv_sock, 5) == -1)
    {
        perror("listen() error");
        exit(1);
    }

    while (1)
    {
        clnt_addr_size = sizeof(clnt_addr);
        // 接受客户端连接请求
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
        {
            perror("accept() error");
            continue;
        }

        // 创建新线程处理客户端请求
        pthread_t t_id;
        pthread_create(&t_id, NULL, client_handler, (void *)&clnt_sock);
        pthread_detach(t_id); // 分离线程，使其结束后自动回收资源
    }

    // 关闭socket
    close(serv_sock);

    return 0;
}

void *client_handler(void *arg)
{
    int clnt_sock = *((int *)arg);
    char message[BUF_SIZE];
    int str_len;

    // 接收客户端发送的消息
    str_len = read(clnt_sock, message, BUF_SIZE - 1);
    if (str_len == 0)
    {
        // 客户端关闭连接
        close(clnt_sock);
        return;
    }
    message[str_len] = 0;
    // TODO: 解析一下报文
    printf("收到客户端的消息：%s", message);

    // 发送消息给客户端
    printf("请输入要发送的消息：");
    fgets(message, BUF_SIZE, stdin);
    str_len = strlen(message);
    write(clnt_sock, message, str_len);

    return NULL;
}