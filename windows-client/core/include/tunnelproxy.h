#ifndef TUNNELPROXY_H
#define TUNNELPROXY_H

// 必须先包含 winsock2.h，再包含 windows.h，避免 winsock.h 被自动包含
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

// 版本信息
#define TUNNELPROXY_VERSION "1.0.0"

// 端口定义
#define LOCAL_PROXY_PORT 34010
#define LOCAL_UDP_RELAY_PORT 34011

// 缓冲区大小
#define MAX_PROCESS_NAME 256
#define MAX_APP_NAME 256
#define MAX_BUFFER_SIZE 65536
#define PID_CACHE_SIZE 1024
#define CONNECTION_HASH_SIZE 256

// 规则协议类型
typedef enum {
    RULE_PROTOCOL_TCP = 0,
    RULE_PROTOCOL_UDP = 1,
    RULE_PROTOCOL_BOTH = 2
} RuleProtocol;

// 规则动作类型
typedef enum {
    RULE_ACTION_PROXY = 0,
    RULE_ACTION_DIRECT = 1,
    RULE_ACTION_BLOCK = 2
} RuleAction;

// 代理类型
typedef enum {
    PROXY_TYPE_SOCKS5 = 0,
    PROXY_TYPE_HTTP = 1
} ProxyType;

// 策略规则结构
typedef struct {
    char app_name[MAX_PROCESS_NAME];
    RuleAction action;
    bool enabled;
} PolicyRule;

// 代理配置结构
typedef struct {
    ProxyType type;
    char host[256];
    uint16_t port;
    char username[256];
    char password[256];
} ProxyConfig;

// 同步配置结构
typedef struct {
    char server_url[512];
    char device_id[128];
    PolicyRule *policies;
    int policy_count;
    ProxyConfig proxy_config;
} SyncConfig;

// 日志回调函数类型
typedef void (*LogCallback)(const char *message);

// 连接日志回调函数类型
typedef void (*ConnectionCallback)(const char *protocol, const char *process,
                                   const char *destination, uint16_t port,
                                   const char *action);

// ==================== 核心引擎 API ====================

// 初始化 TunnelProxy 引擎
bool TunnelProxy_Init(LogCallback log_cb, ConnectionCallback conn_cb);

// 启动 TunnelProxy 引擎
bool TunnelProxy_Start(void);

// 停止 TunnelProxy 引擎
void TunnelProxy_Stop(void);

// 清理 TunnelProxy 引擎
void TunnelProxy_Cleanup(void);

// 检查引擎是否正在运行
bool TunnelProxy_IsRunning(void);

// ==================== 配置管理 API ====================

// 设置代理配置
bool TunnelProxy_SetProxyConfig(const ProxyConfig *config);

// 获取代理配置
bool TunnelProxy_GetProxyConfig(ProxyConfig *config);

// 添加策略规则
bool TunnelProxy_AddPolicy(const PolicyRule *policy);

// 删除策略规则
bool TunnelProxy_RemovePolicy(const char *app_name);

// 清空所有策略规则
void TunnelProxy_ClearPolicies(void);

// 获取策略规则数量
int TunnelProxy_GetPolicyCount(void);

// 获取策略规则列表
bool TunnelProxy_GetPolicies(PolicyRule *policies, int max_count);

// ==================== 同步管理 API ====================

// 设置同步服务器地址
bool TunnelProxy_SetSyncServer(const char *server_url, const char *device_id);

// 从服务器同步策略
bool TunnelProxy_SyncPolicies(void);

// 发送心跳到服务器
bool TunnelProxy_SendHeartbeat(void);

// 获取最后同步时间
bool TunnelProxy_GetLastSyncTime(char *time_str, int max_len);

// ==================== 统计信息 API ====================

// 获取连接统计信息
typedef struct {
    uint64_t total_connections;
    uint64_t proxied_connections;
    uint64_t direct_connections;
    uint64_t blocked_connections;
} ConnectionStats;

bool TunnelProxy_GetStats(ConnectionStats *stats);

// 重置统计信息
void TunnelProxy_ResetStats(void);

#endif // TUNNELPROXY_H
