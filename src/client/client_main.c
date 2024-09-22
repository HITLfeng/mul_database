#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../common/include/common.h"

// #define BUF_SIZE 1024

// int main() {
//     int sock;
//     char message[BUF_SIZE];
//     int str_len;

//     struct sockaddr_in serv_addr;

//     // 创建socket对象
//     sock = socket(PF_INET, SOCK_STREAM, 0);
//     if (sock == -1) {
//         perror("socket() error");
//         exit(1);
//     }

//     // 设置服务器地址和端口
//     memset(&serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
//     serv_addr.sin_port = htons(12345);

//     // 连接服务器
//     if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
//         perror("connect() error");
//         exit(1);
//     }



//     while (1) {
//         printf("请输入要发送的消息：");
//         fgets(message, BUF_SIZE, stdin);
//         str_len = strlen(message);

//         // 发送消息给服务器
//         write(sock, message, str_len);
//         // 接收服务器发送的消息
//         str_len = read(sock, message, BUF_SIZE - 1);
//         message[str_len] = 0;
//         printf("收到服务器的消息：%s", message);

//     }

//     // 关闭socket
//     close(sock);

//     return 0;
// }