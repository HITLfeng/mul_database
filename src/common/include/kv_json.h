#ifndef __KV_JSON_H__
#define __KV_JSON_H__

/*
安装Jansson库
在开始之前，你需要确保你的系统上安装了Jansson库。在不同的操作系统上安装方法有所不同：

Ubuntu/Debian:
sudo apt-get install libjansson-dev
CentOS/RHEL:
sudo yum install jansson-devel
macOS (使用Homebrew):
brew install jansson
引入Jansson库
在你的C程序中引入Jansson库，需要包含头文件jansson.h：

#include <jansson.h>
确保链接时包含了Jansson库。如果你使用gcc编译器，可以使用以下命令：

gcc your_program.c -ljansson -o your_program
Jansson库的基本用法
创建JSON对象
json_t *object = json_object();
json_object_set_new(object, "name", json_string("Alice"));
json_object_set_new(object, "age", json_integer(30));
创建JSON数组
json_t *array = json_array();
json_array_append_new(array, json_string("apple"));
json_array_append_new(array, json_string("banana"));
解析JSON字符串
const char *json_text = "{\"name\": \"Bob\", \"age\": 25}";
json_t *root = json_loads(json_text, 0, NULL);
访问JSON数据
json_t *name = json_object_get(root, "name");
char *name_str = json_string_value(name);
printf("Name: %s\n", name_str);
修改JSON数据
json_object_set_new(root, "age", json_integer(26));
删除JSON数据
json_object_del(root, "age");
将JSON对象转换为字符串
char *json_text_out = json_dumps(root, JSON_COMPACT);
printf("JSON: %s\n", json_text_out);
free(json_text_out);
清理JSON对象
json_decref(root); // 减少引用计数，如果计数为0，则释放内存
注意事项
Jansson使用引用计数来管理内存，所以你需要确保正确地增加和减少对象的引用计数。
当你完成对JSON对象的操作后，记得释放它们以避免内存泄漏。
Jansson库提供了丰富的API来处理各种JSON数据类型，包括对象、数组、字符串、整数、浮点数、布尔值和null。


*/

#include <jansson.h>
#include "common.h"


#ifdef __cplusplus
extern "C" {
#endif

Status KVStringToJSON(const char *json_text, json_t **root);

Status KVJSONToString(const json_t *root, char **json_text);

// 根据root获取其中字段
Status KVJsonGetObject(const json_t *root, const char *key, json_t **value);

// 解析字符串类型json 无需手动释放内存，要长期使用请自己申请一块内存然后copy
Status KVJsonParseStringObj(const json_t *root, const char **value);

// 外部传入一块已分配好的内存进去，内部帮忙copy
Status KVJsonParseStringObjToBuf(const json_t *root, char *valueBuf, uint32_t valueMaxLen);

Status KVJsonArrayGetItem(const json_t *root, uint32_t index, json_t **value);

bool KVJsonIsArray(const json_t *root);
bool KVJsonIsInterger(const json_t *root);

uint32_t KVJsonGetArraySize(const json_t *root);

// 解析int类型

Status KVJsonParseIntObj(const json_t *root, int32_t *value);



#ifdef __cplusplus
}
#endif

#endif // __KV_JSON_H__
