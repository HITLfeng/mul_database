#ifndef __KV_MAP_H__
#define __KV_MAP_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// HASHMAP
typedef int32_t (*HashCodeFuncT)(void *key);

typedef struct DbBucket {
    void *key;
    void *value;
    DbBucketT *next; // 冲突链表
} DbBucketT;

typedef struct DbHashMap {
    DbBucketT *buckets; // 存储区域
    uint32_t bucketCnt; // 有多少个哈希桶
    uint32_t valueCnt;  // 有多少个键值对
    uint32_t listCnt;   // 当前存储有多大
    HashCodeFuncT hashFunc; // 哈希函数
} DbHashMapT;


int32_t DbHashInt32(void *key) {
    DB_POINT(key);
    int32_t hashCode = *(int32_t *)key;
    return hashCode;
}

Status DbCreateHashMap(DbHashMapT **map, HashCodeFuncT hashFunc);

// Status DbMapInsert();

#ifdef __cplusplus
}
#endif

#endif // __KV_MAP_H__
