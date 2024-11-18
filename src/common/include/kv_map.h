#ifndef __KV_MAP_H__
#define __KV_MAP_H__

#include "common.h"
#include "db_memctx.h"

#define DB_HASH_MAP_INIT_LIST_SIZE 16
// 扩容负载因子 0.6 * 10
#define DB_HASH_MAP_EXTEND_RADIO 6
#define DB_HASH_MAP_EXTEND_STEP 2

#ifdef __cplusplus
extern "C" {
#endif

// HASHMAP
typedef uint32_t (*HashCodeFuncT)(void *key);

//int32_t DbHashInt32(void *key) {
//    DB_POINT(key);
//    int32_t hashCode = *(int32_t *) key;
//    return hashCode;
//}

uint32_t DbHashUInt32(void *key) {
    DB_POINT(key);
    uint32_t hashCode = *(uint32_t *) key;
    return hashCode;
}

typedef struct DbBucket DbBucketT;

// 使用一次探测法
struct DbBucket {
    void *key;
    void *value;
//    DbBucketT *next; // 冲突链表
};

typedef struct DbHashMap {
    DbBucketT *buckets; // 存储区域 [null,null,null,null,null] (5)
    uint32_t mapCapacity;   // 当前 map 基础数据长度
    uint32_t bucketCnt; // 有多少个哈希桶 [null,0x22,null,0x15,null] (2)
//    uint32_t elementCnt;  // 有多少个键值对 [null,0x22->0x88,null,0x15,null] (3)
    HashCodeFuncT hashFunc; // 哈希函数
    DbMemCtxT *memCtx;
} DbHashMapT;

Status DbHashMapCreate(DbHashMapT **map, HashCodeFuncT hashFunc, DbMemCtxT *memCtx) {
    DB_POINT3(map, hashFunc, memCtx);
    if (*map != NULL) {
        return;
    }
    DbHashMapT *tmpMap = (DbHashMapT *) DbDynMemCtxAlloc(memCtx, sizeof(DbHashMapT));
    if (tmpMap == NULL) {
        log_error("alloc map failed and alloc size is %u.", sizeof(DbHashMapT));
        return GMERR_MEMCTX_DYN_ALLOC_FAILED;
    }
    tmpMap->mapCapacity = DB_HASH_MAP_INIT_LIST_SIZE;
    log_trace("length of void * is %u.", sizeof(void *));
    tmpMap->buckets = (DbBucketT *) DbDynMemCtxAlloc(memCtx, tmpMap->mapCapacity * sizeof(DbBucketT));
    if (tmpMap->buckets == NULL) {
        log_error("alloc map buckets failed and alloc size is %u.", tmpMap->mapCapacity * sizeof(DbBucketT));
        return GMERR_MEMCTX_DYN_ALLOC_FAILED;
    }
    tmpMap->memCtx = memCtx;
    tmpMap->hashFunc = hashFunc;
    tmpMap->bucketCnt = 0;
//    tmpMap->elementCnt = 0;
    return GMERR_OK;
}


uint32_t GetHashPos(DbHashMapT *map, void *key)
{
    uint32_t hash = map->hashFunc(key);
    uint32_t pos = hash % (map->mapCapacity);
    // TODO: 这里好像不能直接用NULL来判断哈 申请的是结构体 考虑下怎么改！
    while (map->)
    return ;
}

Status DbHashMapExtend(DbHashMapT *map)
{
    DbBucketT *oldBuckets = map->buckets;
    uint32_t oldCapacity = map->mapCapacity;
    uint32_t newCapacity = map->mapCapacity * DB_HASH_MAP_EXTEND_STEP;
    DbBucketT *buckets = (DbBucketT *) DbDynMemCtxAlloc(map->memCtx, newCapacity * sizeof(DbBucketT));
    if (buckets == NULL) {
        log_error("alloc buckets failed and alloc size is %u.", newCapacity * sizeof(DbBucketT));
        return GMERR_MEMCTX_DYN_ALLOC_FAILED;
    }
    map->mapCapacity = newCapacity;
    map->buckets = buckets;

    // reHash
    for (uint32_t i = 0; i < oldCapacity; ++i) {
        // TODO: 重点排查这里有没有问题
        DbBucketT *currBucket = &oldBuckets[i];
        if (currBucket == NULL) {
            continue;
        }


    }

}

Status DbHashMapInsert(DbHashMapT *map, void *key, void *value) {
    DB_POINT3(map, key, value);
    Status ret = GMERR_OK;
    // 负载因子 0.6
    if (map->bucketCnt * 10 / map->mapCapacity >= DB_HASH_MAP_EXTEND_RADIO) {
        ret = DbHashMapExtend(map);
        if (ret != GMERR_OK) {
            return ret;
        }
    }
    
    uint32_t pos = GetHashPos(map, key);
    DB_ASSERT(pos >= 0 && pos < map->mapCapacity);
    
}

// Status DbMapInsert();

#ifdef __cplusplus
}
#endif

#endif // __KV_MAP_H__
