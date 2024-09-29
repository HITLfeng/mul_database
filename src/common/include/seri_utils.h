#ifndef __SERI_UTILS__
#define __SERI_UTILS__

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ******************************************************
// 带M的会移动bufCursor指针
// ******************************************************

// 序列化
void SeriCharM(uint8_t **bufCursor, char value);

void SeriIntM(uint8_t **bufCursor, int32_t value);

void SeriUint32M(uint8_t **bufCursor, uint32_t value);

void SeriStringM(char **bufCursor, const char *value);

// 反序列化
char DeseriCharM(uint8_t **bufCursor);

int32_t DeseriIntM(uint8_t **bufCursor);

uint32_t DeseriUint32M(uint8_t **bufCursor);

void DeseriStringM(uint8_t **bufCursor, char *value);

// ******************************************************
// 不移动位置
// ******************************************************

// 序列化
void SeriChar(uint8_t **bufCursor, char value);

void SeriInt(uint8_t **bufCursor, int32_t value);

void SeriUint32(uint8_t **bufCursor, uint32_t value);

// 反序列化
char DeseriChar(uint8_t **bufCursor);

int32_t DeseriInt(uint8_t **bufCursor);

void DeseriString(uint8_t **bufCursor, char *value);

#ifdef __cplusplus
}
#endif

#endif // __SERI_UTILS__
