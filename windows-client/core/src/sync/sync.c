#include "../include/tunnelproxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

// 外部变量
extern PolicyRule *g_policies;
extern int g_policy_count;
extern ProxyConfig g_proxy_config;
extern ConnectionStats g_stats;

// 同步配置
static char g_sync_server_url[256] = {0};
static char g_device_id[64] = {0};

// 初始化同步模块
bool Sync_Init(void) {
    return true;
}

// 清理同步模块
void Sync_Cleanup(void) {
    // 无需特殊清理
}

// 设置同步服务器
bool Sync_SetServer(const char *server_url, const char *device_id) {
    if (server_url == NULL || device_id == NULL) {
        return false;
    }

    strncpy_s(g_sync_server_url, sizeof(g_sync_server_url), server_url, _TRUNCATE);
    strncpy_s(g_device_id, sizeof(g_device_id), device_id, _TRUNCATE);

    return true;
}

// HTTP GET 请求
static bool http_get(const char *url, char *response, int response_size) {
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool success = false;

    // 解析 URL
    wchar_t wurl[512];
    MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, 512);

    URL_COMPONENTS urlComp;
    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t hostname[256];
    wchar_t urlpath[256];
    urlComp.lpszHostName = hostname;
    urlComp.dwHostNameLength = 256;
    urlComp.lpszUrlPath = urlpath;
    urlComp.dwUrlPathLength = 256;

    if (!WinHttpCrackUrl(wurl, 0, 0, &urlComp)) {
        return false;
    }

    // 创建会话
    hSession = WinHttpOpen(L"TunnelProxy/1.0",
                          WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                          WINHTTP_NO_PROXY_NAME,
                          WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession == NULL) {
        goto cleanup;
    }

    // 连接到服务器
    hConnect = WinHttpConnect(hSession, hostname, urlComp.nPort, 0);
    if (hConnect == NULL) {
        goto cleanup;
    }

    // 创建请求
    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    hRequest = WinHttpOpenRequest(hConnect, L"GET", urlpath, NULL,
                                  WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  flags);

    if (hRequest == NULL) {
        goto cleanup;
    }

    // 发送请求
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                           WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        goto cleanup;
    }

    // 接收响应
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        goto cleanup;
    }

    // 读取响应数据
    DWORD bytesRead = 0;
    DWORD totalRead = 0;

    while (totalRead < response_size - 1) {
        if (!WinHttpReadData(hRequest, response + totalRead,
                            response_size - totalRead - 1, &bytesRead)) {
            break;
        }

        if (bytesRead == 0) {
            break;
        }

        totalRead += bytesRead;
    }

    response[totalRead] = '\0';
    success = (totalRead > 0);

cleanup:
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return success;
}

// 解析 JSON 策略（简单实现）
static bool parse_policies(const char *json) {
    // 清空现有策略
    if (g_policies != NULL) {
        free(g_policies);
        g_policies = NULL;
        g_policy_count = 0;
    }

    // 简单的 JSON 解析（实际项目应使用 JSON 库）
    // 这里只是示例，假设 JSON 格式为：
    // {"policies":[{"app_name":"chrome.exe","action":"PROXY","enabled":true},...]}

    const char *policies_start = strstr(json, "\"policies\":[");
    if (policies_start == NULL) {
        return false;
    }

    policies_start += 12; // 跳过 "policies":[

    // 计算策略数量
    int count = 0;
    const char *p = policies_start;
    while (*p != ']' && *p != '\0') {
        if (*p == '{') {
            count++;
        }
        p++;
    }

    if (count == 0) {
        return true; // 空策略列表
    }

    // 分配内存
    g_policies = (PolicyRule *)malloc(sizeof(PolicyRule) * count);
    if (g_policies == NULL) {
        return false;
    }

    // 解析每个策略
    p = policies_start;
    int index = 0;

    while (*p != ']' && *p != '\0' && index < count) {
        if (*p == '{') {
            PolicyRule *rule = &g_policies[index];
            memset(rule, 0, sizeof(PolicyRule));

            // 查找 app_name
            const char *app_name_start = strstr(p, "\"app_name\":\"");
            if (app_name_start != NULL) {
                app_name_start += 12;
                const char *app_name_end = strchr(app_name_start, '"');
                if (app_name_end != NULL) {
                    int len = (int)(app_name_end - app_name_start);
                    if (len < MAX_APP_NAME) {
                        strncpy_s(rule->app_name, MAX_APP_NAME, app_name_start, len);
                    }
                }
            }

            // 查找 action
            const char *action_start = strstr(p, "\"action\":\"");
            if (action_start != NULL) {
                action_start += 10;
                if (strncmp(action_start, "PROXY", 5) == 0) {
                    rule->action = RULE_ACTION_PROXY;
                } else if (strncmp(action_start, "DIRECT", 6) == 0) {
                    rule->action = RULE_ACTION_DIRECT;
                } else if (strncmp(action_start, "BLOCK", 5) == 0) {
                    rule->action = RULE_ACTION_BLOCK;
                }
            }

            // 查找 enabled
            const char *enabled_start = strstr(p, "\"enabled\":");
            if (enabled_start != NULL) {
                enabled_start += 10;
                rule->enabled = (strncmp(enabled_start, "true", 4) == 0);
            }

            index++;
        }

        // 跳到下一个策略
        const char *next = strchr(p + 1, '{');
        if (next == NULL) {
            break;
        }
        p = next;
    }

    g_policy_count = index;
    return true;
}

// 同步策略（从服务器拉取）
bool Sync_PullPolicies(void) {
    if (g_sync_server_url[0] == '\0' || g_device_id[0] == '\0') {
        return false;
    }

    // 构造请求 URL
    char url[512];
    snprintf(url, sizeof(url), "%s/api/sync/policies?device_id=%s",
             g_sync_server_url, g_device_id);

    // 发送 HTTP GET 请求
    char response[65536];
    if (!http_get(url, response, sizeof(response))) {
        return false;
    }

    // 解析策略
    return parse_policies(response);
}

// 上报设备状态
bool Sync_ReportStatus(void) {
    if (g_sync_server_url[0] == '\0' || g_device_id[0] == '\0') {
        return false;
    }

    // 构造请求 URL（包含统计信息）
    char url[512];
    snprintf(url, sizeof(url),
             "%s/api/sync/status?device_id=%s&total=%lld&proxied=%lld&direct=%lld&blocked=%lld",
             g_sync_server_url, g_device_id,
             g_stats.total_connections,
             g_stats.proxied_connections,
             g_stats.direct_connections,
             g_stats.blocked_connections);

    // 发送 HTTP GET 请求
    char response[1024];
    return http_get(url, response, sizeof(response));
}
