#include "../include/tunnelproxy.h"
#include "windivert.h"
#include <iphlpapi.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")

// 外部变量声明
extern LogCallback g_log_callback;
extern ConnectionCallback g_connection_callback;
extern bool g_running;
extern HANDLE g_windivert_handle;
extern PolicyRule *g_policies;
extern int g_policy_count;
extern ProxyConfig g_proxy_config;
extern ConnectionStats g_stats;

// PID 缓存结构
typedef struct PID_CACHE_ENTRY {
    uint32_t src_ip;
    uint16_t src_port;
    DWORD pid;
    DWORD timestamp;
    bool is_udp;
    struct PID_CACHE_ENTRY *next;
} PID_CACHE_ENTRY;

static PID_CACHE_ENTRY *g_pid_cache[PID_CACHE_SIZE] = {NULL};
static CRITICAL_SECTION g_pid_cache_lock;

// 连接信息缓存
typedef struct CONNECTION_INFO {
    uint16_t src_port;
    uint32_t src_ip;
    uint32_t orig_dest_ip;
    uint16_t orig_dest_port;
    bool is_tracked;
    ULONGLONG last_activity;
    struct CONNECTION_INFO *next;
} CONNECTION_INFO;

static CONNECTION_INFO *g_connection_hash[CONNECTION_HASH_SIZE] = {NULL};
static CRITICAL_SECTION g_connection_lock;

// 日志记录缓存（避免重复日志）
typedef struct LOGGED_CONNECTION {
    DWORD pid;
    uint32_t dest_ip;
    uint16_t dest_port;
    RuleAction action;
    struct LOGGED_CONNECTION *next;
} LOGGED_CONNECTION;

static LOGGED_CONNECTION *g_logged_connections = NULL;
static CRITICAL_SECTION g_log_lock;

// 初始化进程识别模块
void Process_Init(void) {
    InitializeCriticalSection(&g_pid_cache_lock);
    InitializeCriticalSection(&g_connection_lock);
    InitializeCriticalSection(&g_log_lock);
}

// 清理进程识别模块
void Process_Cleanup(void) {
    // 清理 PID 缓存
    EnterCriticalSection(&g_pid_cache_lock);
    for (int i = 0; i < PID_CACHE_SIZE; i++) {
        PID_CACHE_ENTRY *entry = g_pid_cache[i];
        while (entry != NULL) {
            PID_CACHE_ENTRY *next = entry->next;
            free(entry);
            entry = next;
        }
        g_pid_cache[i] = NULL;
    }
    LeaveCriticalSection(&g_pid_cache_lock);

    // 清理连接缓存
    EnterCriticalSection(&g_connection_lock);
    for (int i = 0; i < CONNECTION_HASH_SIZE; i++) {
        CONNECTION_INFO *conn = g_connection_hash[i];
        while (conn != NULL) {
            CONNECTION_INFO *next = conn->next;
            free(conn);
            conn = next;
        }
        g_connection_hash[i] = NULL;
    }
    LeaveCriticalSection(&g_connection_lock);

    // 清理日志缓存
    EnterCriticalSection(&g_log_lock);
    LOGGED_CONNECTION *log = g_logged_connections;
    while (log != NULL) {
        LOGGED_CONNECTION *next = log->next;
        free(log);
        log = next;
    }
    g_logged_connections = NULL;
    LeaveCriticalSection(&g_log_lock);

    DeleteCriticalSection(&g_pid_cache_lock);
    DeleteCriticalSection(&g_connection_lock);
    DeleteCriticalSection(&g_log_lock);
}

// PID 缓存哈希函数
static uint32_t pid_cache_hash(uint32_t src_ip, uint16_t src_port) {
    return (src_ip ^ src_port) % PID_CACHE_SIZE;
}

// 缓存 PID
static void cache_pid(uint32_t src_ip, uint16_t src_port, DWORD pid, bool is_udp) {
    uint32_t hash = pid_cache_hash(src_ip, src_port);

    EnterCriticalSection(&g_pid_cache_lock);

    PID_CACHE_ENTRY *entry = (PID_CACHE_ENTRY *)malloc(sizeof(PID_CACHE_ENTRY));
    if (entry != NULL) {
        entry->src_ip = src_ip;
        entry->src_port = src_port;
        entry->pid = pid;
        entry->timestamp = GetTickCount();
        entry->is_udp = is_udp;
        entry->next = g_pid_cache[hash];
        g_pid_cache[hash] = entry;
    }

    LeaveCriticalSection(&g_pid_cache_lock);
}

// 获取缓存的 PID
static DWORD get_cached_pid(uint32_t src_ip, uint16_t src_port, bool is_udp) {
    uint32_t hash = pid_cache_hash(src_ip, src_port);
    DWORD current_time = GetTickCount();
    DWORD pid = 0;

    EnterCriticalSection(&g_pid_cache_lock);

    PID_CACHE_ENTRY *entry = g_pid_cache[hash];
    while (entry != NULL) {
        if (entry->src_ip == src_ip &&
            entry->src_port == src_port &&
            entry->is_udp == is_udp) {
            // 检查是否过期（1秒 TTL）
            if (current_time - entry->timestamp < 1000) {
                pid = entry->pid;
                break;
            }
        }
        entry = entry->next;
    }

    LeaveCriticalSection(&g_pid_cache_lock);

    return pid;
}

// 从 TCP 连接表获取 PID
static DWORD get_pid_from_tcp_table(uint32_t src_ip, uint16_t src_port) {
    PMIB_TCPTABLE_OWNER_PID tcp_table = NULL;
    DWORD size = 0;
    DWORD pid = 0;

    // 第一次调用获取所需大小
    GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);

    tcp_table = (PMIB_TCPTABLE_OWNER_PID)malloc(size);
    if (tcp_table == NULL) {
        return 0;
    }

    // 第二次调用获取实际数据
    if (GetExtendedTcpTable(tcp_table, &size, FALSE, AF_INET,
                           TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR) {
        free(tcp_table);
        return 0;
    }

    // 遍历连接表查找匹配项
    for (DWORD i = 0; i < tcp_table->dwNumEntries; i++) {
        if (tcp_table->table[i].dwLocalAddr == src_ip &&
            tcp_table->table[i].dwLocalPort == htons(src_port)) {
            pid = tcp_table->table[i].dwOwningPid;
            break;
        }
    }

    free(tcp_table);
    return pid;
}

// 从 UDP 连接表获取 PID
static DWORD get_pid_from_udp_table(uint32_t src_ip, uint16_t src_port) {
    PMIB_UDPTABLE_OWNER_PID udp_table = NULL;
    DWORD size = 0;
    DWORD pid = 0;

    // 第一次调用获取所需大小
    GetExtendedUdpTable(NULL, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);

    udp_table = (PMIB_UDPTABLE_OWNER_PID)malloc(size);
    if (udp_table == NULL) {
        return 0;
    }

    // 第二次调用获取实际数据
    if (GetExtendedUdpTable(udp_table, &size, FALSE, AF_INET,
                           UDP_TABLE_OWNER_PID, 0) != NO_ERROR) {
        free(udp_table);
        return 0;
    }

    // 遍历连接表查找匹配项
    for (DWORD i = 0; i < udp_table->dwNumEntries; i++) {
        if (udp_table->table[i].dwLocalAddr == src_ip &&
            udp_table->table[i].dwLocalPort == htons(src_port)) {
            pid = udp_table->table[i].dwOwningPid;
            break;
        }
    }

    free(udp_table);
    return pid;
}

// 从 PID 获取进程名
static bool get_process_name_from_pid(DWORD pid, char *name, DWORD name_size) {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (process == NULL) {
        return false;
    }

    char path[MAX_PATH];
    DWORD path_len = sizeof(path);

    // 获取进程可执行文件路径
    if (QueryFullProcessImageNameA(process, 0, path, &path_len)) {
        // 提取文件名
        const char *last_backslash = strrchr(path, '\\');
        const char *last_slash = strrchr(path, '/');
        const char *last_separator = (last_backslash > last_slash) ? last_backslash : last_slash;
        const char *filename = last_separator ? (last_separator + 1) : path;

        strncpy_s(name, name_size, filename, _TRUNCATE);
        CloseHandle(process);
        return true;
    }

    CloseHandle(process);
    return false;
}

// 获取进程 ID（带缓存）
DWORD Process_GetPID(uint32_t src_ip, uint16_t src_port, bool is_udp) {
    // 检查缓存
    DWORD pid = get_cached_pid(src_ip, src_port, is_udp);
    if (pid != 0) {
        return pid;
    }

    // 从系统连接表获取
    if (is_udp) {
        pid = get_pid_from_udp_table(src_ip, src_port);
    } else {
        pid = get_pid_from_tcp_table(src_ip, src_port);
    }

    // 缓存结果
    if (pid != 0) {
        cache_pid(src_ip, src_port, pid, is_udp);
    }

    return pid;
}

// 获取进程名
bool Process_GetName(DWORD pid, char *name, DWORD name_size) {
    return get_process_name_from_pid(pid, name, name_size);
}

// 连接信息哈希函数
static uint32_t connection_hash(uint16_t src_port) {
    return src_port % CONNECTION_HASH_SIZE;
}

// 添加连接信息
void Connection_Add(uint16_t src_port, uint32_t src_ip, uint32_t dest_ip, uint16_t dest_port) {
    uint32_t hash = connection_hash(src_port);

    EnterCriticalSection(&g_connection_lock);

    CONNECTION_INFO *conn = (CONNECTION_INFO *)malloc(sizeof(CONNECTION_INFO));
    if (conn != NULL) {
        conn->src_port = src_port;
        conn->src_ip = src_ip;
        conn->orig_dest_ip = dest_ip;
        conn->orig_dest_port = dest_port;
        conn->is_tracked = true;
        conn->last_activity = GetTickCount64();
        conn->next = g_connection_hash[hash];
        g_connection_hash[hash] = conn;
    }

    LeaveCriticalSection(&g_connection_lock);
}

// 获取连接信息
bool Connection_Get(uint16_t src_port, uint32_t *dest_ip, uint16_t *dest_port) {
    uint32_t hash = connection_hash(src_port);
    bool found = false;

    EnterCriticalSection(&g_connection_lock);

    CONNECTION_INFO *conn = g_connection_hash[hash];
    while (conn != NULL) {
        if (conn->src_port == src_port) {
            *dest_ip = conn->orig_dest_ip;
            *dest_port = conn->orig_dest_port;
            found = true;
            break;
        }
        conn = conn->next;
    }

    LeaveCriticalSection(&g_connection_lock);

    return found;
}

// 检查连接是否被跟踪
bool Connection_IsTracked(uint16_t src_port) {
    uint32_t hash = connection_hash(src_port);
    bool tracked = false;

    EnterCriticalSection(&g_connection_lock);

    CONNECTION_INFO *conn = g_connection_hash[hash];
    while (conn != NULL) {
        if (conn->src_port == src_port) {
            tracked = true;
            break;
        }
        conn = conn->next;
    }

    LeaveCriticalSection(&g_connection_lock);

    return tracked;
}

// 移除连接信息
void Connection_Remove(uint16_t src_port) {
    uint32_t hash = connection_hash(src_port);

    EnterCriticalSection(&g_connection_lock);

    CONNECTION_INFO **prev = &g_connection_hash[hash];
    CONNECTION_INFO *conn = g_connection_hash[hash];

    while (conn != NULL) {
        if (conn->src_port == src_port) {
            *prev = conn->next;
            free(conn);
            break;
        }
        prev = &conn->next;
        conn = conn->next;
    }

    LeaveCriticalSection(&g_connection_lock);
}

// 检查连接是否已记录日志
static bool is_connection_logged(DWORD pid, uint32_t dest_ip, uint16_t dest_port, RuleAction action) {
    EnterCriticalSection(&g_log_lock);

    LOGGED_CONNECTION *log = g_logged_connections;
    while (log != NULL) {
        if (log->pid == pid &&
            log->dest_ip == dest_ip &&
            log->dest_port == dest_port &&
            log->action == action) {
            LeaveCriticalSection(&g_log_lock);
            return true;
        }
        log = log->next;
    }

    LeaveCriticalSection(&g_log_lock);
    return false;
}

// 添加日志记录
static void add_logged_connection(DWORD pid, uint32_t dest_ip, uint16_t dest_port, RuleAction action) {
    EnterCriticalSection(&g_log_lock);

    LOGGED_CONNECTION *log = (LOGGED_CONNECTION *)malloc(sizeof(LOGGED_CONNECTION));
    if (log != NULL) {
        log->pid = pid;
        log->dest_ip = dest_ip;
        log->dest_port = dest_port;
        log->action = action;
        log->next = g_logged_connections;
        g_logged_connections = log;
    }

    LeaveCriticalSection(&g_log_lock);
}

// 记录连接日志
void Connection_Log(const char *protocol, const char *process, uint32_t dest_ip,
                   uint16_t dest_port, RuleAction action, DWORD pid) {
    // 检查是否已记录
    if (is_connection_logged(pid, dest_ip, dest_port, action)) {
        return;
    }

    // 添加到已记录列表
    add_logged_connection(pid, dest_ip, dest_port, action);

    // 格式化目标地址
    char dest_str[32];
    snprintf(dest_str, sizeof(dest_str), "%d.%d.%d.%d",
             (dest_ip >> 0) & 0xFF, (dest_ip >> 8) & 0xFF,
             (dest_ip >> 16) & 0xFF, (dest_ip >> 24) & 0xFF);

    // 格式化动作
    const char *action_str;
    switch (action) {
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
        action_str = "UNKNOWN";
        break;
    }

    // 调用回调函数
    if (g_connection_callback != NULL) {
        g_connection_callback(protocol, process, dest_str, dest_port, action_str);
    }
}
