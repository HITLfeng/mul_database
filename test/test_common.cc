#include "test_common.h"
#include <fstream>
#include <sstream>


char* ReadFile(const char* filename) {
    FILE *file = fopen(filename, "rb"); // 以二进制模式打开文件
    if (!file) {
        perror("无法打开文件");
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配足够的内存来存储文件内容
    char *buffer = (char*)malloc(file_size + 1); // +1是为了存储字符串结束符'\0'
    if (!buffer) {
        perror("内存分配失败");
        fclose(file);
        return NULL;
    }

    // 读取文件内容到缓冲区
    size_t result = fread(buffer, 1, file_size, file);
    if (result != (size_t)file_size) {
        perror("读取文件失败");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // 添加字符串结束符
    buffer[file_size] = '\0';

    // 关闭文件
    fclose(file);

    return buffer;
}

std::string ReadFileCpp(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}