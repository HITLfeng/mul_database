#include "kv_map.h"
#include "kv_memory.h"

uint32_t DbHashUInt32(void *key)
{
    DB_POINT(key);
    uint32_t hashCode = *(uint32_t *) key;
    return hashCode;
}

uint32_t DbCmpUInt32(const void *key1, const void *key2)
{
    DB_POINT2(key1, key2);
    if (*(const uint32_t *) key1 == *(const uint32_t *) key2) {
        return DB_HASH_CMP_EQUAL;
    }
    return DB_HASH_CMP_NOT_EQUAL;
}

Status DbHashMapCreate(DbHashMapT **map, HashCodeFuncT hashFunc, HashCmpFuncT hashCmpFunc, DbMemCtxT *memCtx)
{
    DB_POINT3(map, hashFunc, memCtx);
    if (*map != NULL) {
        return GMERR_OK;
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
    for (uint32_t i = 0; i < tmpMap->mapCapacity; ++i) {
        tmpMap->buckets[i].state = BUCKET_FREE;
    }
    tmpMap->memCtx = memCtx;
    tmpMap->hashFunc = hashFunc;
    tmpMap->hashCmpFunc = hashCmpFunc;
    tmpMap->bucketCnt = 0;
//    tmpMap->elementCnt = 0;
    return GMERR_OK;
}


uint32_t GetNextFreeHashPos(DbHashMapT *map, void *key)
{
    uint32_t hash = map->hashFunc(key);
    uint32_t pos = hash % (map->mapCapacity);
    // TODO: 这里好像不能直接用NULL来判断哈 申请的是结构体 考虑下怎么改！
    while (map->buckets[pos].state != BUCKET_FREE) {
        pos = (pos + 1) % map->mapCapacity;
    }
    return pos;
}

uint32_t GetFirstHashPos(DbHashMapT *map, void *key)
{
    uint32_t hash = map->hashFunc(key);
    uint32_t pos = hash % (map->mapCapacity);
    return pos;
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
        DbBucketT currBucket = oldBuckets[i];
        uint32_t pos = GetNextFreeHashPos(map, currBucket.key);
        map->buckets[pos] = currBucket;
    }
    return GMERR_OK;
}

Status DbHashMapInsert(DbHashMapT *map, void *key, void *value)
{
    DB_POINT3(map, key, value);
    Status ret = GMERR_OK;
    // 负载因子 0.6
    if (map->bucketCnt * 10 / map->mapCapacity >= DB_HASH_MAP_EXTEND_RADIO) {
        ret = DbHashMapExtend(map);
        if (ret != GMERR_OK) {
            return ret;
        }
    }

    uint32_t pos = GetNextFreeHashPos(map, key);
    DB_ASSERT(pos >= 0 && pos < map->mapCapacity);
    map->buckets[pos].key = key;
    map->buckets[pos].value = value;
    map->buckets[pos].state = BUCKET_USING;
}

bool DbIsBucketMatch(HashCmpFuncT hashCmpFunc, void *key1, void *key2)
{
    uint32_t res = hashCmpFunc(key1, key2);
    if (res == DB_HASH_CMP_NOT_EQUAL) {
        return false;
    }
    return true;
}

void *DbHashMapFind(DbHashMapT *map, void *key)
{
    uint32_t pos = GetFirstHashPos(map, key);
    uint32_t findTime = 0;
    while (map->buckets[pos].state != BUCKET_FREE) {
        DbBucketT currBucket = map->buckets[pos];
        if (DbIsBucketMatch(map->hashCmpFunc, currBucket.key, key)) {
            return currBucket.value;
        }
        pos = (pos + 1) % map->mapCapacity;
        findTime++;
        log_trace("find time is %u now.", findTime);
    }
    return NULL;
}

// hash map delete 时释放 key value 内存 请保证这段内存申请自 map->memCtx
Status DbHashMapDelete(DbHashMapT *map, void *key, bool isFreeMem)
{
    uint32_t pos = GetFirstHashPos(map, key);
    uint32_t findTime = 0;
    while (map->buckets[pos].state != BUCKET_FREE) {
        DbBucketT *currBucket = &map->buckets[pos];
        if (DbIsBucketMatch(map->hashCmpFunc, currBucket->key, key)) {
            if (isFreeMem) {
                DbDynMemCtxFree(map->memCtx, currBucket->key);
                DbDynMemCtxFree(map->memCtx, currBucket->value);
            }
            currBucket->key = NULL;
            currBucket->value = NULL;
            currBucket->state = BUCKET_DELETE;
            return GMERR_OK;
        }
        // TODO: 现在有大BUG 如果中间删除了 会导致后面的数据存在但报找不到！
        pos = (pos + 1) % map->mapCapacity;
        findTime++;
        log_trace("find time is %u now.", findTime);
    }
    log_error("can find this key in map when delete elememt.");
    return GMERR_MAP_KEY_NOT_EXIST;
}


Status DbHashMapFetch(DbHashMapT *map, void **key, void **value, DbHashMapIter *iter)
{
    for (uint32_t i = *iter; i < map->mapCapacity; ++i) {
        if (map->buckets[i].state == BUCKET_USING) {
            *key = map->buckets[i].key;
            *value = map->buckets[i].value;
            *iter = i + 1;
            return GMERR_OK;
        }
    }
    return GMERR_MAP_NO_DATA;
}

// Status DbMapInsert();

//void DbCreateHashMap(DbHashMapT **map, HashCodeFuncT hashFunc)
//{
//    DB_POINT(map);
//    if (*map != NULL) {
//        return;
//    }
//    DbHashMapT *tmpMap = (DbHashMapT *)KVMemAlloc(sizeof(DbHashMapT));
//    DB_ASSERT(tmpMap != NULL);
//    memset(tmpMap, 0x00, sizeof(DbHashMapT));
//    tmpMap->listCnt = DB_HASH_MAP_INIT_LIST_SIZE;
//
//    tmpMap->buckets = (DbBucketT *)KVMemAlloc(sizeof(DbBucketT *) * tmpMap->listCnt);
//    DB_ASSERT(tmpMap->buckets != NULL);
//    tmpMap->hashFunc = hashFunc;
//    tmpMap->bucketCnt = 0;
//    tmpMap->valueCnt = 0;
//}