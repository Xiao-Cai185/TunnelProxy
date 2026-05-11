#include "../include/tunnelproxy.h"
#include "utils/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 外部变量
extern PolicyRule *g_policies;
extern int g_policy_count;
extern ConnectionStats g_stats;

// 通配符匹配函数
static bool match_wildcard(const char *pattern, const char *text) {
    if (pattern == NULL || text == NULL) {
        return false;
    }

    // 空模式或 * 匹配所有
    if (pattern[0] == '\0' || strcmp(pattern, "*") == 0) {
        return true;
    }

    // 前缀通配符: "chr*" 匹配 "chrome.exe"
    size_t pattern_len = strlen(pattern);
    if (pattern[pattern_len - 1] == '*') {
        return _strnicmp(pattern, text, pattern_len - 1) == 0;
    }

    // 后缀通配符: "*.exe" 匹配 "chrome.exe"
    if (pattern[0] == '*') {
        const char *suffix = pattern + 1;
        size_t suffix_len = strlen(suffix);
        size_t text_len = strlen(text);
        if (text_len < suffix_len) {
            return false;
        }
        return _stricmp(text + text_len - suffix_len, suffix) == 0;
    }

    // 中间通配符: "chr*me.exe" 匹配 "chrome.exe"
    const char *star = strchr(pattern, '*');
    if (star != NULL) {
        size_t prefix_len = star - pattern;
        const char *suffix = star + 1;
        size_t suffix_len = strlen(suffix);
        size_t text_len = strlen(text);

        if (text_len < prefix_len + suffix_len) {
            return false;
        }

        return _strnicmp(pattern, text, prefix_len) == 0 &&
               _stricmp(text + text_len - suffix_len, suffix) == 0;
    }

    // 精确匹配
    return _stricmp(pattern, text) == 0;
}

// 匹配规则
RuleAction Rule_Match(const char *process_name, uint32_t dest_ip, uint16_t dest_port, bool is_udp) {
    if (process_name == NULL) {
        return RULE_ACTION_DIRECT;
    }

    // 遍历所有策略
    for (int i = 0; i < g_policy_count; i++) {
        PolicyRule *rule = &g_policies[i];

        // 检查是否启用
        if (!rule->enabled) {
            continue;
        }

        // 匹配进程名
        if (match_wildcard(rule->app_name, process_name)) {
            return rule->action;
        }
    }

    // 无匹配规则，默认直连
    return RULE_ACTION_DIRECT;
}

// 检查进程规则并返回动作
RuleAction Rule_CheckProcess(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip,
                             uint16_t dest_port, bool is_udp, DWORD *out_pid) {
    // 获取进程 ID
    DWORD pid = Process_GetPID(src_ip, src_port, is_udp);
    if (out_pid != NULL) {
        *out_pid = pid;
    }

    if (pid == 0) {
        return RULE_ACTION_DIRECT;
    }

    // 获取进程名
    char process_name[MAX_PROCESS_NAME];
    if (!Process_GetName(pid, process_name, sizeof(process_name))) {
        return RULE_ACTION_DIRECT;
    }

    // 排除自身进程
    if (_stricmp(process_name, "TunnelProxy.exe") == 0) {
        return RULE_ACTION_DIRECT;
    }

    // 匹配规则
    RuleAction action = Rule_Match(process_name, dest_ip, dest_port, is_udp);

    // 记录连接日志
    Connection_Log(is_udp ? "UDP" : "TCP", process_name, dest_ip, dest_port, action, pid);

    // 更新统计信息
    InterlockedIncrement64(&g_stats.total_connections);
    switch (action) {
    case RULE_ACTION_PROXY:
        InterlockedIncrement64(&g_stats.proxied_connections);
        break;
    case RULE_ACTION_DIRECT:
        InterlockedIncrement64(&g_stats.direct_connections);
        break;
    case RULE_ACTION_BLOCK:
        InterlockedIncrement64(&g_stats.blocked_connections);
        break;
    }

    return action;
}
