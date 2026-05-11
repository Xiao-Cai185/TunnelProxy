#include "../include/tunnelproxy.h"
#include "windivert.h"
#include "windivert/packet_handler.h"
#include "utils/process.h"
#include "rules/rule_engine.h"
#include "proxy/socks5.h"
#include "proxy/http_connect.h"
#include "relay/tcp_relay.h"
#include "relay/udp_relay.h"
#include "sync/sync.h"
#include "config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 全局状态
static bool g_running = false;
static bool g_initialized = false;
static HANDLE g_windivert_handle = INVALID_HANDLE_VALUE;
static HANDLE g_packet_threads[4] = {NULL};
static HANDLE g_proxy_thread = NULL;
static HANDLE g_udp_relay_thread = NULL;

// 回调函数
static LogCallback g_log_callback = NULL;
static ConnectionCallback g_connection_callback = NULL;

// 配置
static ProxyConfig g_proxy_config = {0};
static PolicyRule *g_policies = NULL;
static int g_policy_count = 0;
static SyncConfig g_sync_config = {0};

// 统计信息
static ConnectionStats g_stats = {0};

// 日志函数
static void log_message(const char *format, ...) {
    if (g_log_callback == NULL) return;

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    g_log_callback(buffer);
}

// ==================== 核心引擎实现 ====================

bool TunnelProxy_Init(LogCallback log_cb, ConnectionCallback conn_cb) {
    if (g_initialized) {
        return true;
    }

    g_log_callback = log_cb;
    g_connection_callback = conn_cb;

    // 初始化 Winsock
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_message("Failed to initialize Winsock");
        return false;
    }

    // 初始化进程识别模块
    Process_Init();

    // 初始化数据包处理模块
    if (!PacketHandler_Init()) {
        log_message("Failed to initialize packet handler");
        WSACleanup();
        return false;
    }

    // 初始化 TCP 中继服务器
    if (!TcpRelay_Init()) {
        log_message("Failed to initialize TCP relay");
        PacketHandler_Cleanup();
        Process_Cleanup();
        WSACleanup();
        return false;
    }

    // 初始化 UDP 中继服务器
    if (!UdpRelay_Init()) {
        log_message("Failed to initialize UDP relay");
        TcpRelay_Cleanup();
        PacketHandler_Cleanup();
        Process_Cleanup();
        WSACleanup();
        return false;
    }

    // 初始化同步模块
    if (!Sync_Init()) {
        log_message("Failed to initialize sync module");
        UdpRelay_Cleanup();
        TcpRelay_Cleanup();
        PacketHandler_Cleanup();
        Process_Cleanup();
        WSACleanup();
        return false;
    }

    // 加载配置文件
    Config_Load();

    // 初始化策略列表
    if (g_policies == NULL) {
        g_policies = NULL;
        g_policy_count = 0;
    }

    // 重置统计信息
    memset(&g_stats, 0, sizeof(g_stats));

    g_initialized = true;
    log_message("TunnelProxy initialized successfully");

    return true;
}

bool TunnelProxy_Start(void) {
    if (!g_initialized) {
        log_message("TunnelProxy not initialized");
        return false;
    }

    if (g_running) {
        log_message("TunnelProxy already running");
        return true;
    }

    // 打开 WinDivert
    g_windivert_handle = WinDivertOpen(
        "outbound and (tcp or udp)",
        WINDIVERT_LAYER_NETWORK,
        0,
        0
    );

    if (g_windivert_handle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        log_message("Failed to open WinDivert: error %lu", error);
        return false;
    }

    g_running = true;

    // 启动 TCP 中继服务器
    if (!TcpRelay_Start()) {
        log_message("Failed to start TCP relay server");
        WinDivertClose(g_windivert_handle);
        g_windivert_handle = INVALID_HANDLE_VALUE;
        g_running = false;
        return false;
    }

    // 启动 UDP 中继服务器
    if (!UdpRelay_Start()) {
        log_message("Failed to start UDP relay server");
        TcpRelay_Stop();
        WinDivertClose(g_windivert_handle);
        g_windivert_handle = INVALID_HANDLE_VALUE;
        g_running = false;
        return false;
    }

    // 启动数据包处理线程
    if (!PacketHandler_Start()) {
        log_message("Failed to start packet handler");
        UdpRelay_Stop();
        TcpRelay_Stop();
        WinDivertClose(g_windivert_handle);
        g_windivert_handle = INVALID_HANDLE_VALUE;
        g_running = false;
        return false;
    }

    log_message("TunnelProxy started successfully");

    return true;
}

void TunnelProxy_Stop(void) {
    if (!g_running) {
        return;
    }

    g_running = false;

    // 停止数据包处理线程
    PacketHandler_Stop();

    // 停止 TCP 中继服务器
    TcpRelay_Stop();

    // 停止 UDP 中继服务器
    UdpRelay_Stop();

    // 关闭 WinDivert
    if (g_windivert_handle != INVALID_HANDLE_VALUE) {
        WinDivertClose(g_windivert_handle);
        g_windivert_handle = INVALID_HANDLE_VALUE;
    }

    log_message("TunnelProxy stopped");
}

void TunnelProxy_Cleanup(void) {
    if (g_running) {
        TunnelProxy_Stop();
    }

    // 清理同步模块
    Sync_Cleanup();

    // 清理 UDP 中继服务器
    UdpRelay_Cleanup();

    // 清理 TCP 中继服务器
    TcpRelay_Cleanup();

    // 清理数据包处理模块
    PacketHandler_Cleanup();

    // 清理进程识别模块
    Process_Cleanup();

    // 清理策略列表
    if (g_policies != NULL) {
        free(g_policies);
        g_policies = NULL;
        g_policy_count = 0;
    }

    // 清理 Winsock
    WSACleanup();

    g_initialized = false;
    log_message("TunnelProxy cleaned up");
}

bool TunnelProxy_IsRunning(void) {
    return g_running;
}

// ==================== 配置管理实现 ====================

bool TunnelProxy_SetProxyConfig(const ProxyConfig *config) {
    if (config == NULL) {
        return false;
    }

    memcpy(&g_proxy_config, config, sizeof(ProxyConfig));

    log_message("Proxy config updated: %s://%s:%d",
                config->type == PROXY_TYPE_SOCKS5 ? "SOCKS5" : "HTTP",
                config->host, config->port);

    return true;
}

bool TunnelProxy_GetProxyConfig(ProxyConfig *config) {
    if (config == NULL) {
        return false;
    }

    memcpy(config, &g_proxy_config, sizeof(ProxyConfig));
    return true;
}

bool TunnelProxy_AddPolicy(const PolicyRule *policy) {
    if (policy == NULL) {
        return false;
    }

    // 检查是否已存在
    for (int i = 0; i < g_policy_count; i++) {
        if (strcmp(g_policies[i].app_name, policy->app_name) == 0) {
            // 更新现有策略
            g_policies[i] = *policy;
            log_message("Policy updated: %s -> %s",
                       policy->app_name,
                       policy->action == RULE_ACTION_PROXY ? "PROXY" :
                       policy->action == RULE_ACTION_DIRECT ? "DIRECT" : "BLOCK");
            return true;
        }
    }

    // 添加新策略
    PolicyRule *new_policies = (PolicyRule *)realloc(g_policies,
                                                     (g_policy_count + 1) * sizeof(PolicyRule));
    if (new_policies == NULL) {
        log_message("Failed to allocate memory for policy");
        return false;
    }

    g_policies = new_policies;
    g_policies[g_policy_count] = *policy;
    g_policy_count++;

    log_message("Policy added: %s -> %s",
               policy->app_name,
               policy->action == RULE_ACTION_PROXY ? "PROXY" :
               policy->action == RULE_ACTION_DIRECT ? "DIRECT" : "BLOCK");

    return true;
}

bool TunnelProxy_RemovePolicy(const char *app_name) {
    if (app_name == NULL) {
        return false;
    }

    for (int i = 0; i < g_policy_count; i++) {
        if (strcmp(g_policies[i].app_name, app_name) == 0) {
            // 移除策略
            for (int j = i; j < g_policy_count - 1; j++) {
                g_policies[j] = g_policies[j + 1];
            }
            g_policy_count--;

            log_message("Policy removed: %s", app_name);
            return true;
        }
    }

    return false;
}

void TunnelProxy_ClearPolicies(void) {
    if (g_policies != NULL) {
        free(g_policies);
        g_policies = NULL;
    }
    g_policy_count = 0;

    log_message("All policies cleared");
}

int TunnelProxy_GetPolicyCount(void) {
    return g_policy_count;
}

bool TunnelProxy_GetPolicies(PolicyRule *policies, int max_count) {
    if (policies == NULL || max_count <= 0) {
        return false;
    }

    int count = g_policy_count < max_count ? g_policy_count : max_count;
    memcpy(policies, g_policies, count * sizeof(PolicyRule));

    return true;
}

// ==================== 同步管理实现 ====================

bool TunnelProxy_SetSyncServer(const char *server_url, const char *device_id) {
    if (server_url == NULL || device_id == NULL) {
        return false;
    }

    strncpy(g_sync_config.server_url, server_url, sizeof(g_sync_config.server_url) - 1);
    strncpy(g_sync_config.device_id, device_id, sizeof(g_sync_config.device_id) - 1);

    // 设置同步模块的服务器地址
    Sync_SetServer(server_url, device_id);

    log_message("Sync server set: %s (Device: %s)", server_url, device_id);

    return true;
}

bool TunnelProxy_SyncPolicies(void) {
    log_message("Syncing policies from server...");

    // 调用同步模块拉取策略
    if (!Sync_PullPolicies()) {
        log_message("Failed to sync policies from server");
        return false;
    }

    log_message("Policies synced successfully");
    return true;
}

bool TunnelProxy_SendHeartbeat(void) {
    log_message("Sending heartbeat to server...");

    // 调用同步模块上报状态
    if (!Sync_ReportStatus()) {
        log_message("Failed to send heartbeat to server");
        return false;
    }

    log_message("Heartbeat sent successfully");
    return true;
}

bool TunnelProxy_GetLastSyncTime(char *time_str, int max_len) {
    if (time_str == NULL || max_len <= 0) {
        return false;
    }

    // TODO: 实现获取最后同步时间
    strncpy(time_str, "Never", max_len - 1);

    return true;
}

// ==================== 统计信息实现 ====================

bool TunnelProxy_GetStats(ConnectionStats *stats) {
    if (stats == NULL) {
        return false;
    }

    memcpy(stats, &g_stats, sizeof(ConnectionStats));
    return true;
}

void TunnelProxy_ResetStats(void) {
    memset(&g_stats, 0, sizeof(g_stats));
    log_message("Statistics reset");
}
