#include "../include/tunnelproxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 外部变量
extern PolicyRule *g_policies;
extern int g_policy_count;
extern ProxyConfig g_proxy_config;

// 读取文件内容
static char *read_file(const char *filename) {
    FILE *file = NULL;
    if (fopen_s(&file, filename, "rb") != 0 || file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 0) {
        fclose(file);
        return NULL;
    }

    char *content = (char *)malloc(size + 1);
    if (content == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(content, 1, size, file);
    content[read_size] = '\0';

    fclose(file);
    return content;
}

// 写入文件内容
static bool write_file(const char *filename, const char *content) {
    FILE *file = NULL;
    if (fopen_s(&file, filename, "wb") != 0 || file == NULL) {
        return false;
    }

    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, file);

    fclose(file);
    return (written == len);
}

// 查找 JSON 字符串值
static bool json_get_string(const char *json, const char *key, char *value, int value_size) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);

    const char *start = strstr(json, search_key);
    if (start == NULL) {
        return false;
    }

    start += strlen(search_key);
    const char *end = strchr(start, '"');
    if (end == NULL) {
        return false;
    }

    int len = (int)(end - start);
    if (len >= value_size) {
        len = value_size - 1;
    }

    strncpy_s(value, value_size, start, len);
    return true;
}

// 查找 JSON 整数值
static bool json_get_int(const char *json, const char *key, int *value) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\":", key);

    const char *start = strstr(json, search_key);
    if (start == NULL) {
        return false;
    }

    start += strlen(search_key);
    *value = atoi(start);
    return true;
}

// 查找 JSON 布尔值
static bool json_get_bool(const char *json, const char *key, bool *value) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\":", key);

    const char *start = strstr(json, search_key);
    if (start == NULL) {
        return false;
    }

    start += strlen(search_key);
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start++;
    }

    *value = (strncmp(start, "true", 4) == 0);
    return true;
}

// 加载代理配置
bool Config_LoadProxyConfig(ProxyConfig *config) {
    char *json = read_file(CONFIG_FILE_PATH);
    if (json == NULL) {
        return false;
    }

    bool success = true;

    // 查找 proxy 对象
    const char *proxy_start = strstr(json, "\"proxy\":{");
    if (proxy_start != NULL) {
        char type_str[32];
        if (json_get_string(proxy_start, "type", type_str, sizeof(type_str))) {
            if (strcmp(type_str, "socks5") == 0) {
                config->type = PROXY_TYPE_SOCKS5;
            } else if (strcmp(type_str, "http") == 0) {
                config->type = PROXY_TYPE_HTTP;
            }
        }

        json_get_string(proxy_start, "host", config->host, sizeof(config->host));
        json_get_int(proxy_start, "port", &config->port);
        json_get_string(proxy_start, "username", config->username, sizeof(config->username));
        json_get_string(proxy_start, "password", config->password, sizeof(config->password));
    } else {
        success = false;
    }

    free(json);
    return success;
}

// 保存代理配置
bool Config_SaveProxyConfig(const ProxyConfig *config) {
    char json[4096];
    const char *type_str = (config->type == PROXY_TYPE_SOCKS5) ? "socks5" : "http";

    snprintf(json, sizeof(json),
             "{\n"
             "  \"proxy\": {\n"
             "    \"type\": \"%s\",\n"
             "    \"host\": \"%s\",\n"
             "    \"port\": %d,\n"
             "    \"username\": \"%s\",\n"
             "    \"password\": \"%s\"\n"
             "  }\n"
             "}\n",
             type_str, config->host, config->port,
             config->username, config->password);

    return write_file(CONFIG_FILE_PATH, json);
}

// 加载策略规则
bool Config_LoadPolicies(PolicyRule **policies, int *count) {
    char *json = read_file(CONFIG_FILE_PATH);
    if (json == NULL) {
        return false;
    }

    // 查找 policies 数组
    const char *policies_start = strstr(json, "\"policies\":[");
    if (policies_start == NULL) {
        free(json);
        return false;
    }

    policies_start += 12; // 跳过 "policies":[

    // 计算策略数量
    int policy_count = 0;
    const char *p = policies_start;
    while (*p != ']' && *p != '\0') {
        if (*p == '{') {
            policy_count++;
        }
        p++;
    }

    if (policy_count == 0) {
        *policies = NULL;
        *count = 0;
        free(json);
        return true;
    }

    // 分配内存
    *policies = (PolicyRule *)malloc(sizeof(PolicyRule) * policy_count);
    if (*policies == NULL) {
        free(json);
        return false;
    }

    // 解析每个策略
    p = policies_start;
    int index = 0;

    while (*p != ']' && *p != '\0' && index < policy_count) {
        if (*p == '{') {
            PolicyRule *rule = &(*policies)[index];
            memset(rule, 0, sizeof(PolicyRule));

            json_get_string(p, "app_name", rule->app_name, sizeof(rule->app_name));

            char action_str[32];
            if (json_get_string(p, "action", action_str, sizeof(action_str))) {
                if (strcmp(action_str, "PROXY") == 0) {
                    rule->action = RULE_ACTION_PROXY;
                } else if (strcmp(action_str, "DIRECT") == 0) {
                    rule->action = RULE_ACTION_DIRECT;
                } else if (strcmp(action_str, "BLOCK") == 0) {
                    rule->action = RULE_ACTION_BLOCK;
                }
            }

            json_get_bool(p, "enabled", &rule->enabled);

            index++;
        }

        const char *next = strchr(p + 1, '{');
        if (next == NULL) {
            break;
        }
        p = next;
    }

    *count = index;
    free(json);
    return true;
}

// 保存策略规则
bool Config_SavePolicies(const PolicyRule *policies, int count) {
    char json[65536];
    int offset = 0;

    offset += snprintf(json + offset, sizeof(json) - offset,
                      "{\n  \"policies\": [\n");

    for (int i = 0; i < count; i++) {
        const PolicyRule *rule = &policies[i];
        const char *action_str;

        switch (rule->action) {
        case RULE_ACTION_PROXY:
            action_str = "PROXY";
            break;
        case RULE_ACTION_DIRECT:
            action_str = "DIRECT";
            break;
        case RULE_ACTION_BLOCK:
            action_str = "BLOCK";
            break;
        default:
            action_str = "DIRECT";
            break;
        }

        offset += snprintf(json + offset, sizeof(json) - offset,
                          "    {\n"
                          "      \"app_name\": \"%s\",\n"
                          "      \"action\": \"%s\",\n"
                          "      \"enabled\": %s\n"
                          "    }%s\n",
                          rule->app_name, action_str,
                          rule->enabled ? "true" : "false",
                          (i < count - 1) ? "," : "");
    }

    offset += snprintf(json + offset, sizeof(json) - offset,
                      "  ]\n}\n");

    return write_file(CONFIG_FILE_PATH, json);
}

// 加载同步配置
bool Config_LoadSyncConfig(char *server_url, int url_size, char *device_id, int id_size) {
    char *json = read_file(CONFIG_FILE_PATH);
    if (json == NULL) {
        return false;
    }

    bool success = true;

    const char *sync_start = strstr(json, "\"sync\":{");
    if (sync_start != NULL) {
        json_get_string(sync_start, "server_url", server_url, url_size);
        json_get_string(sync_start, "device_id", device_id, id_size);
    } else {
        success = false;
    }

    free(json);
    return success;
}

// 保存同步配置
bool Config_SaveSyncConfig(const char *server_url, const char *device_id) {
    char json[4096];

    snprintf(json, sizeof(json),
             "{\n"
             "  \"sync\": {\n"
             "    \"server_url\": \"%s\",\n"
             "    \"device_id\": \"%s\"\n"
             "  }\n"
             "}\n",
             server_url, device_id);

    return write_file(CONFIG_FILE_PATH, json);
}

// 加载配置文件
bool Config_Load(void) {
    // 加载代理配置
    if (!Config_LoadProxyConfig(&g_proxy_config)) {
        // 使用默认配置
        memset(&g_proxy_config, 0, sizeof(g_proxy_config));
        g_proxy_config.type = PROXY_TYPE_SOCKS5;
        strcpy_s(g_proxy_config.host, sizeof(g_proxy_config.host), "127.0.0.1");
        g_proxy_config.port = 1080;
    }

    // 加载策略规则
    if (!Config_LoadPolicies(&g_policies, &g_policy_count)) {
        g_policies = NULL;
        g_policy_count = 0;
    }

    return true;
}

// 保存配置文件
bool Config_Save(void) {
    // 保存代理配置
    if (!Config_SaveProxyConfig(&g_proxy_config)) {
        return false;
    }

    // 保存策略规则
    if (!Config_SavePolicies(g_policies, g_policy_count)) {
        return false;
    }

    return true;
}
