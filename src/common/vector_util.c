#include "vector_util.h"

// 初始化容量0 不申请buf
#define DB_VECTOR_INIT_CAPACITY 5
// 单次扩容步长
// #define DB_VECTOR_EXTEND_STEP 5

void DbVectorInit(DbVectorT *vector, uint32_t itemSize)
{
    DB_POINT(vector);
    memset(vector, 0x00, sizeof(DbVectorT));
    vector->size = 0;
    vector->capacity = DB_VECTOR_INIT_CAPACITY;
    vector->itemSize = itemSize;
    uint32_t initAllocSize = vector->capacity * itemSize * DB_VECTOR_INIT_CAPACITY;
    vector->data = malloc(initAllocSize);
    DB_ASSERT(vector->data != NULL);
}

Status DbVectorAppendItem(DbVectorT *vector, void *item)
{
    DB_POINT(vector);
    if (vector->size >= vector->capacity)
    {
        uint32_t newAllocSize = vector->capacity * vector->itemSize * 2;
        normal_info("vector capacity extend from %u to %u.", vector->capacity, vector->capacity * 2);
        vector->data = realloc(vector->data, newAllocSize);
        if (vector->data == NULL)
        {
            error_info("realloc data memory error. alloc size is %u.", newAllocSize);
            return GMERR_MEMORY_ALLOC_FAILED;
        }
        vector->capacity *= 2;
    }
    memcpy((char *)vector->data + vector->size * vector->itemSize, item, vector->itemSize);
    vector->size++;
    return GMERR_OK;
}

void *DbVectorGetItem(DbVectorT *vector, uint32_t index)
{
    DB_POINT(vector);
    if (index >= vector->size)
    {
        error_info("DbVectorGetItem: index %u is out of range. vector size is %u.", index, vector->size);
        return NULL;
    }
    return (char *)vector->data + index * vector->itemSize;
}

void DbVectorRemoveItem(DbVectorT *vector, uint32_t index)
{
    DB_POINT(vector);
    if (index >= vector->size)
    {
        error_info("DbVectorRemoveItem: index %u is out of range. vector size is %u.", index, vector->size);
        return;
    }
    if (index == vector->size - 1)
    {
        vector->size--;
        return;
    }
    memmove((char *)vector->data + index * vector->itemSize, (char *)vector->data + (index + 1) * vector->itemSize, (vector->size - index - 1) * vector->itemSize);
    vector->size--;
}

void DbVectorClear(DbVectorT *vector)
{
    DB_POINT(vector);
    vector->size = 0;
}

void DbVectorDestroy(DbVectorT *vector)
{
    DB_POINT(vector);
    if (vector->data != NULL)
    {
        free(vector->data);
        vector->data = NULL;
    }
    vector->size = 0;
    vector->capacity = 0;
    vector->itemSize = 0;
}
