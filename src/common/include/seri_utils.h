#ifndef __SERI_UTILS__
#define __SERI_UTILS__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif // __SERI_UTILS__
