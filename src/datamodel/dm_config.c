#include "dm_common.h"

#define DM_CONFIG_FILE "/root/db/mul_database/config/kvserver.ini"
#define MAX_LINE_LENGTH 256

typedef struct DmConfig {
    bool debugInfo; // 是否打印内部调试信息
} DmConfigT;

DmConfigT *g_dmConfig = NULL;

bool IsDebugInfoOn() {
    return g_dmConfig->debugInfo;
}

// 去除字符串两端的空格
void StrTrim(char *str) {
    char *start = str;
    char *end = str + strlen(str) - 1;

    while (*start == ' ')
        start++;
    while (end > start && *end == ' ')
        end--;

    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

Status ParseKeyValue(char *line, char *key, char *value) {
    uint32_t len = strlen(line);
    char *equals = strchr(line, '=');
    if (equals) {
        *equals = '\0';                                                 // 分割键和值
        strncpy(key, line, equals - line);                              // 复制键
        StrTrim(key);                                                   // 去除键两端的空格
        strncpy(value, equals + 1, len - (equals - line) - 1); // 复制值
        StrTrim(value);                                                 // 去除值两端的空格
    } else {
        key[0] = '\0';
        value[0] = '\0';
        log_error("config file line is not valid, line is %s", line);
        return GMERR_CONFIG_FILE_LINE_INVAILD;
    }
    return GMERR_OK;
}

Status DealKeyValue(const char *key, const char *value, DmConfigT *dmConfig) {
    if (strcmp(key, "debugInfo") == 0) {
        dmConfig->debugInfo = (strcmp(value, "on") == 0);
        return GMERR_OK;
    }
    log_warn("config key is not valid, key is %s.", key);
    return GMERR_OK;
}

void TraceConfigState() { log_info("debugInfo = %d.", g_dmConfig->debugInfo); }
Status DmConfigInit(void) {
    DmConfigT *dmConfig = DbDynMemCtxAlloc(NULL, sizeof(DmConfigT));

    FILE *file = fopen(DM_CONFIG_FILE, "r");
    if (file == NULL) {
        log_error("open config file failed, path is %s.", DM_CONFIG_FILE);
        return GMERR_CONFIG_FILE_OPEN_FAILED;
    }
    char line[MAX_LINE_LENGTH] = {0};
    char key[MAX_LINE_LENGTH] = {0};
    char value[MAX_LINE_LENGTH] = {0};
    while (fgets(line, sizeof(line), file)) {
        // 去除行尾的换行符
        line[strcspn(line, "\n")] = '\0';

        // 忽略空行和注释
        if (strlen(line) > 0 && line[0] != '#') {
            Status ret = ParseKeyValue(line, key, value);
            if (ret != GMERR_OK) {
                return ret;
            }
            // 处理键值对
            ret = DealKeyValue(key, value, dmConfig);
            if (ret != GMERR_OK) {
                return ret;
            }
        }
    }
    fclose(file);
    g_dmConfig = dmConfig;
    TraceConfigState();
    return GMERR_OK;
}