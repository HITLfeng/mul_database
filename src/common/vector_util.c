#include "vector_util.h"

// 初始化容量0 不申请buf
#define DB_VECTOR_INIT_CAPACITY 5
// 单次扩容步长
// #define DB_VECTOR_EXTEND_STEP 5

void DbVectorInit(DbVectorT *vector, uint32_t itemSize, DbMemCtxT *memCtx) {
    DB_POINT(vector);
    if (memCtx == NULL) {
        memCtx = DbGetTopMemCtx();
    }
    memset(vector, 0x00, sizeof(DbVectorT));
    vector->size = 0;
    vector->capacity = DB_VECTOR_INIT_CAPACITY;
    vector->itemSize = itemSize;
    uint32_t initAllocSize = vector->capacity * itemSize * DB_VECTOR_INIT_CAPACITY;
    vector->data = DbDynMemCtxAlloc(memCtx, initAllocSize);
    DB_ASSERT(vector->data != NULL);
    vector->memCtx = memCtx;
}

Status DbVectorAppendItem(DbVectorT *vector, void *item) {
    DB_POINT(vector);
    if (vector->size >= vector->capacity) {
        uint32_t newAllocSize = vector->capacity * vector->itemSize * 2;
        log_info("vector capacity extend from %u to %u.", vector->capacity, vector->capacity * 2);
        void *oldData = vector->data;
        vector->data = DbDynMemCtxAlloc(vector->memCtx, newAllocSize);
        // DB_ASSERT(vector->data != NULL);
        if (vector->data == NULL) {
            log_error("realloc data memory error. alloc size is %u.", newAllocSize);
            return GMERR_MEMORY_ALLOC_FAILED;
        }
        memcpy((uint8_t *)vector->data, oldData, vector->size * vector->itemSize);
        DbDynMemCtxFree(vector->memCtx, oldData);
        vector->capacity *= 2;
    }
    memcpy((uint8_t *)vector->data + vector->size * vector->itemSize, item, vector->itemSize);
    vector->size++;
    return GMERR_OK;
}

void *DbVectorGetItem(DbVectorT *vector, uint32_t index) {
    DB_POINT(vector);
    if (index >= vector->size) {
        log_error("DbVectorGetItem: index %u is out of range. vector size is %u.", index, vector->size);
        return NULL;
    }
    return (char *)vector->data + index * vector->itemSize;
}

void DbVectorRemoveItem(DbVectorT *vector, uint32_t index) {
    DB_POINT(vector);
    if (index >= vector->size) {
        log_error("DbVectorRemoveItem: index %u is out of range. vector size is %u.", index, vector->size);
        return;
    }
    if (index == vector->size - 1) {
        vector->size--;
        return;
    }
    memmove((char *)vector->data + index * vector->itemSize, (char *)vector->data + (index + 1) * vector->itemSize,
            (vector->size - index - 1) * vector->itemSize);
    vector->size--;
}

void DbVectorClear(DbVectorT *vector) {
    DB_POINT(vector);
    vector->size = 0;
}

void DbVectorDestroy(DbVectorT *vector) {
    DB_POINT(vector);
    if (vector->data != NULL) {
        DbDynMemCtxFree(vector->memCtx, vector->data);
        vector->data = NULL;
    }
    vector->size = 0;
    vector->capacity = 0;
    vector->itemSize = 0;
}
