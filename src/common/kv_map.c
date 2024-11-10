#include "kv_map.h"
#include "kv_memory.h"


#define DB_HASH_MAP_INIT_LIST_SIZE 16

void DbCreateHashMap(DbHashMapT **map, HashCodeFuncT hashFunc)
{
    DB_POINT(map);
    if (*map != NULL) {
        return;
    }
    DbHashMapT *tmpMap = (DbHashMapT *)KVMemAlloc(sizeof(DbHashMapT));
    DB_ASSERT(tmpMap != NULL);
    memset(tmpMap, 0x00, sizeof(DbHashMapT));
    tmpMap->listCnt = DB_HASH_MAP_INIT_LIST_SIZE;

    tmpMap->buckets = (DbBucketT *)KVMemAlloc(sizeof(DbBucketT *) * tmpMap->listCnt);
    DB_ASSERT(tmpMap->buckets != NULL);
    tmpMap->hashFunc = hashFunc;
    tmpMap->bucketCnt = 0;
    tmpMap->valueCnt = 0;
}