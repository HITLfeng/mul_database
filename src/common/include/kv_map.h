#ifndef __KV_MAP_H__
#define __KV_MAP_H__

#include "common.h"
#include "db_memctx.h"

#define DB_HASH_MAP_INIT_LIST_SIZE 16
// 扩容负载因子 0.6 * 10
#define DB_HASH_MAP_EXTEND_RADIO 6
#define DB_HASH_MAP_EXTEND_STEP 2

#define DB_HASH_CMP_EQUAL 0
#define DB_HASH_CMP_NOT_EQUAL 1

#ifdef __cplusplus
extern "C" {
#endif

// HASHMAP
typedef uint32_t (*HashCodeFuncT)(void *key);

// hash cmp key  0 --> equal  >= 1 not equal
typedef uint32_t (*HashCmpFuncT)(const void *key1, const void *key2);

//int32_t DbHashInt32(void *key) {
//    DB_POINT(key);
//    int32_t hashCode = *(int32_t *) key;
//    return hashCode;
//}

// 标识 bucket 状态
typedef enum {
    BUCKET_FREE,
    BUCKET_USING,
    BUCKET_DELETE
} BucketStateT;

uint32_t DbHashUInt32(void *key);

uint32_t DbCmpUInt32(const void *key1, const void *key2);

typedef struct DbBucket DbBucketT;

// 使用一次探测法
struct DbBucket {
    void *key;
    void *value;
    BucketStateT state;
//    bool isUsed; // 标识当前槽位是否已被使用
//    DbBucketT *next; // 冲突链表
};

typedef struct DbHashMap {
    DbBucketT *buckets; // 存储区域 [null,null,null,null,null] (5)
    uint32_t mapCapacity;   // 当前 map 基础数据长度
    uint32_t bucketCnt; // 有多少个哈希桶 [null,0x22,null,0x15,null] (2)
//    uint32_t elementCnt;  // 有多少个键值对 [null,0x22->0x88,null,0x15,null] (3)
    HashCodeFuncT hashFunc; // 哈希函数
    HashCmpFuncT hashCmpFunc; // 哈希key值比较函数
    DbMemCtxT *memCtx;
} DbHashMapT;

typedef uint32_t DbHashMapIter;



static inline uint32_t DbHashGetSize(DbHashMapT *map)
{
    return map->bucketCnt;
}

static inline uint32_t DbHashGetCapacity(DbHashMapT *map)
{
    return map->mapCapacity;
}

Status DbHashMapCreate(DbHashMapT **map, HashCodeFuncT hashFunc, HashCmpFuncT hashCmpFunc, DbMemCtxT *memCtx);
Status DbHashMapInsert(DbHashMapT *map, void *key, void *value);
void *DbHashMapFind(DbHashMapT *map, void *key);
// hash map delete 时释放 key value 内存 请保证这段内存申请自 map->memCtx
Status DbHashMapDelete(DbHashMapT *map, void *key, bool isFreeMem);
Status DbHashMapFetch(DbHashMapT *map, void **key, void **value, DbHashMapIter *iter);

#ifdef __cplusplus
}
#endif

#endif // __KV_MAP_H__
