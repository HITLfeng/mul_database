#include "include/kv_json.h"

Status KVStringToJSON(const char *json_text, json_t **root)
{
    *root = json_loads(json_text, 0, NULL);
    if (*root == NULL)
    {
        return GMERR_JSON_LIB_ERROR;
    }
    return GMERR_OK;
}

Status KVJSONToString(const json_t *root, char **json_text)
{
    *json_text = json_dumps(root, JSON_COMPACT);
    if (*json_text == NULL)
    {
        return GMERR_JSON_LIB_ERROR;
    }
    return GMERR_OK;
}

// 根据root获取其中字段
Status KVJsonGetObject(const json_t *root, const char *key, json_t **value)
{
    *value = json_object_get(root, key);
    if (*value == NULL)
    {
        return GMERR_JSON_LIB_ERROR;
    }
    return GMERR_OK;
}

// 解析字符串类型json
Status KVJsonParseStringObj(const json_t *root, const char **value)
{
    *value = json_string_value(root);
    if (*value == NULL)
    {
        return GMERR_JSON_LIB_ERROR;
    }
    return GMERR_OK;
}

// 解析JSON字符串
// const char *json_text = "{\"name\": \"Bob\", \"age\": 25}";
// json_t *root = json_loads(json_text, 0, NULL);
// 访问JSON数据
// json_t *name = json_object_get(root, "name");
// char *name_str = json_string_value(name);
