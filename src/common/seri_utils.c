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

void SeriIntM(uint8_t **bufCursor, int32_t value)
{
    *((int32_t *)(*bufCursor)) = value;
    *bufCursor += sizeof(int32_t);
}

void SeriStringM(char **bufCursor, const char *value)
{
    uint32_t strLen = strlen(value);
    *((uint32_t *)(*bufCursor)) = strLen;
    *bufCursor += sizeof(uint32_t);
    memcpy(*bufCursor, value, strLen);
    *bufCursor += strLen;
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

void DeseriStringM(uint8_t **bufCursor, char *value)
{
    uint32_t strLen = *((uint32_t *)(*bufCursor));
    *bufCursor += sizeof(uint32_t);
    memcpy(value, *bufCursor, strLen);
    *bufCursor += strLen;
}





// ******************************************************
// 不移动位置
// ******************************************************

// 序列化
void SeriChar(uint8_t **bufCursor, char value)
{
    *((char *)(*bufCursor)) = value;
}

void SeriInt(uint8_t **bufCursor, int32_t value)
{
    *((int32_t *)(*bufCursor)) = value;
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