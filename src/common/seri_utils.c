#include "include/seri_utils.h"
// ******************************************************
// 带M的会移动bufCursor指针
// ******************************************************

// 序列化
void SeriCharM(uint8_t **bufCursor, char value)
{
    *((char *)(*bufCursor)) = value;
    *bufCursor += sizeof(char);
}

void SeriInt32M(uint8_t **bufCursor, int32_t value)
{
    *((int32_t *)(*bufCursor)) = value;
    *bufCursor += sizeof(int32_t);
}

void SeriUint32M(uint8_t **bufCursor, uint32_t value)
{
    *((uint32_t *)(*bufCursor)) = value;
    *bufCursor += sizeof(uint32_t);
}

void SeriStringM(char **bufCursor, const char *value)
{
    uint32_t strLen = strlen(value);
    *((uint32_t *)(*bufCursor)) = strLen;
    *bufCursor += sizeof(uint32_t);
    memcpy(*bufCursor, value, strLen);
    *bufCursor += strLen;
}

void SeriFixedStringM(uint8_t **bufCursor, const char *value, uint32_t fixLen)
{
    uint32_t strLen = strlen(value);
    memcpy(*bufCursor, value, strLen);
    *bufCursor += fixLen;
}

// 反序列化
char DeseriCharM(uint8_t **bufCursor)
{
    char value = *((char *)(*bufCursor));
    *bufCursor += sizeof(char);
    return value;
}

int32_t DeseriIntM(uint8_t **bufCursor)
{
    int32_t value = *((int32_t *)(*bufCursor));
    *bufCursor += sizeof(int32_t);
    return value;
}

uint32_t DeseriUint32M(uint8_t **bufCursor)
{
    uint32_t value = *((uint32_t *)(*bufCursor));
    *bufCursor += sizeof(uint32_t);
    return value;
}

void DeseriStringM(uint8_t **bufCursor, char *value)
{
    uint32_t strLen = *((uint32_t *)(*bufCursor));
    *bufCursor += sizeof(uint32_t);
    memcpy(value, *bufCursor, strLen);
    *bufCursor += strLen;
}

// 请使用fixLen大小value来接收反序列化结果！
void DeseriFixedStringM(uint8_t **bufCursor, char *value, uint32_t fixLen)
{
    memcpy(value, *bufCursor, fixLen);
    *bufCursor += fixLen;
}

// ******************************************************
// 不移动位置
// ******************************************************

// 序列化
void SeriChar(uint8_t **bufCursor, char value)
{
    *((char *)(*bufCursor)) = value;
}

void SeriInt32(uint8_t **bufCursor, int32_t value)
{
    *((int32_t *)(*bufCursor)) = value;
}

void SeriUint32(uint8_t **bufCursor, uint32_t value)
{
    *((uint32_t *)(*bufCursor)) = value;
}

void SeriString(char **bufCursor, const char *value)
{
    char *tmpBufCursor = *bufCursor;
    uint32_t strLen = strlen(value);
    *((uint32_t *)(tmpBufCursor)) = strLen;
    tmpBufCursor += sizeof(uint32_t);
    memcpy(tmpBufCursor, value, strLen);
    tmpBufCursor += strLen;
}

// 反序列化
char DeseriChar(uint8_t **bufCursor)
{
    char value = *((char *)(*bufCursor));
    return value;
}

int32_t DeseriInt(uint8_t **bufCursor)
{
    int32_t value = *((int32_t *)(*bufCursor));
    return value;
}

void DeseriString(uint8_t **bufCursor, char *value)
{
    uint8_t **tmpBufCursor = bufCursor;
    uint32_t strLen = *((uint32_t *)(*tmpBufCursor));
    *tmpBufCursor += sizeof(uint32_t);
    memcpy(value, *tmpBufCursor, strLen);
    // *tmpBufCursor += strLen;
}