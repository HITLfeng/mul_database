

#include "dm_common.h"

#define DM_VALUE_GREATER 1
#define DM_VALUE_EQUAL 0
#define DM_VALUE_LESS -1

bool IsIntegerDataType(DmValueTypeT type) {
    return type >= DB_TYPE_INT32 && type <= DB_TYPE_INTEGER;
}

bool IsStringDataType(DmValueTypeT type) {
    return type >= DB_TYPE_STRING && type <= DB_TYPE_STRING;
}

static void SetDmValueType(DmValueT *dmValue, DmValueTypeT type)
{
    dmValue->type = type;
}

//void DmGetIntegerValue(DmValueT *dmValue, const void *buf)
//{
//    switch (dmValue->type) {
//        case DB_TYPE_INT32:
//            dmValue->value.int32 = *(const int32_t *)value;
//            break;
//        case DB_TYPE_UINT32:
//            dmValue->value.uint32 = *(const uint32_t *)value;
//            break;
//        default:
//            log_error("DmSetIntegerValue error. type(%u) is not defined.", dmValue->type);
//            break;
//    }
//}

//void DmGetValue(DmValueT *dmValue, const void *buf, DmValueTypeT type)
//{
//    SetDmValueType(dmValue, type);
//    if (IsIntegerDataType(type)) {
//        DmGetIntegerValue(dmValue, value);
//        return;
//    } else if (IsStringDataType(type)) {
//        DmSetStringValue(dmValue, value, valueLen);
//        return;
//    }
//    log_warn("set value error and type is %u.", type);
//}

/**
typedef enum {
    DB_TYPE_INT32 = 0,
    DB_TYPE_UINT32,
    DB_TYPE_STRING,
    DB_TYPE_BUTT,
} DbValueTypeT;
 */

void DmSetIntegerValue(DmValueT *dmValue, const void *value)
{
    switch (dmValue->type) {
        case DB_TYPE_INT32:
            dmValue->value.int32 = *(const int32_t *)value;
            break;
        case DB_TYPE_UINT32:
            dmValue->value.uint32 = *(const uint32_t *)value;
            break;
        default:
            log_error("DmSetIntegerValue error. type(%u) is not defined.", dmValue->type);
            break;
    }
}

void DmSetStringValue(DmValueT *dmValue, const void *value, uint32_t valueLen)
{
    switch (dmValue->type) {
        case DB_TYPE_STRING:
            DB_ASSERT(valueLen <= DB_VALUE_MAX_LENGTH);
            memcpy(dmValue->value.strings.str, value, valueLen);
            dmValue->value.strings.strLen = valueLen;
            break;
        default:
            log_error("DmSetStringValue error. type(%u) is not defined.", dmValue->type);
            break;
    }
}

void DmSetValue(DmValueT *dmValue, const void *value, uint32_t valueLen, DmValueTypeT type)
{
    DB_POINT2(dmValue, value);
    DB_ASSERT(type < DB_TYPE_BUTT);
    SetDmValueType(dmValue, type);
    if (IsIntegerDataType(type)) {
        DmSetIntegerValue(dmValue, value);
        return;
    } else if (IsStringDataType(type)) {
        DmSetStringValue(dmValue, value, valueLen);
        return;
    }
    log_warn("set value error and type is %u.", type);
}

int32_t DmCmpInt32(int32_t value1, int32_t value2)
{
    if (value1 > value2) {
        return DM_VALUE_GREATER;
    }
    if (value1 == value2) {
        return DM_VALUE_EQUAL;
    }
    return DM_VALUE_LESS;
}

int32_t DmCmpUint32(uint32_t value1, uint32_t value2)
{
    if (value1 > value2) {
        return DM_VALUE_GREATER;
    }
    if (value1 == value2) {
        return DM_VALUE_EQUAL;
    }
    return DM_VALUE_LESS;
}

int32_t DmCmpString(char *value1, char *value2)
{
    if (strcmp(value1, value2) > 0) {
        return DM_VALUE_GREATER;
    }
    if (strcmp(value1, value2) == 0) {
        return DM_VALUE_EQUAL;
    }
    return DM_VALUE_LESS;
}

int32_t DmCmpIntegerValue(DmValueT *dmValueLeft, DmValueT *dmValueRight) {
    DB_ASSERT(dmValueLeft->type == dmValueRight->type);
    switch (dmValueLeft->type) {
        case DB_TYPE_INT32:
            return DmCmpInt32(dmValueLeft->value.int32, dmValueRight->value.int32);
        case DB_TYPE_UINT32:
            return DmCmpUint32(dmValueLeft->value.uint32, dmValueRight->value.uint32);
        default:
            log_error("DmCmpIntegerValue error. type(%u) is not defined.", dmValueLeft->type);
            break;
    }
}

int32_t DmCmpStringValue(DmValueT *dmValueLeft, DmValueT *dmValueRight)
{
    DB_ASSERT(dmValueLeft->type == dmValueRight->type);
    switch (dmValueLeft->type) {
        case DB_TYPE_STRING:
            return DmCmpString(dmValueLeft->value.strings.str, dmValueRight->value.strings.str);
        default:
            log_error("DmCmpStringValue error. type(%u) is not defined.", dmValueLeft->type);
            break;
    }
}

/**
 * > 0 value1 > value2
 * = 0 value1 = value2
 * < 0 value1 < value2
 */
int32_t DmCmpValue(DmValueT *dmValueLeft, DmValueT *dmValueRight)
{
    DB_POINT2(dmValueLeft, dmValueRight);
    DB_ASSERT(dmValueLeft->type == dmValueRight->type);
    if (IsIntegerDataType(dmValueLeft->type)) {
        return DmCmpIntegerValue(dmValueLeft, dmValueRight);
    } else if (IsStringDataType(dmValueLeft->type)) {
        return DmCmpStringValue(dmValueLeft, dmValueRight);
    }
    log_warn("cmp value error and type is %u.", dmValueLeft->type);
}
