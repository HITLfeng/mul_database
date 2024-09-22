#ifndef __VECTOR_H__
#define __VECTOR_H__


#include "common.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct DbVector
{
    uint32_t size;
    uint32_t capacity;
    uint32_t itemSize;
    void *data;
} DbVectorT;

void DbVectorInit(DbVectorT *vector, uint32_t itemSize); // 初始化vector itemSize 存放内容大小

Status DbVectorAppendItem(DbVectorT *vector, void *item);

void *DbVectorGetItem(DbVectorT *vector, uint32_t index);

void DbVectorRemoveItem(DbVectorT *vector, uint32_t index);

void DbVectorClear(DbVectorT *vector);

// 如果内部存储的指针，请自行释放
void DbVectorDestroy(DbVectorT *vector);

inline static uint32_t DbVectorGetSize(DbVectorT *vector)
{
    DB_POINT(vector);
    return vector->size;
}


#ifdef __cplusplus
}
#endif

#endif // __VECTOR_H__
