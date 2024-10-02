

2024-04-10 15:21
针对当前采用 char 数组存储所有类型数据的做法，当在一个难以理解的core问题
考虑使用一个联合体进行存储 设想如下

typedef struct KvValue {
    type type;
    union {
        int32_t int32value;
        uint32_t uint32value;
        char str[SR_FIELD_VALUE_MAX_LENGTH];
    } value;
} KvValueT;

如果晚上可以把core原因搞清楚的话，这事就不急着落
