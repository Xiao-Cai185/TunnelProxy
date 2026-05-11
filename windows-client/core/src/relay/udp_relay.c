#include "../include/tunnelproxy.h"
#include "proxy/socks5.h"
#include "utils/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 外部变量
extern bool g_running;
extern ProxyConfig g_proxy_config;

// UDP 中继服务器
static SOCKET g_udp_relay_socket = INVALID_SOCKET;
static HANDLE g_udp_relay_thread = NULL;

// UDP 会话结构
typedef struct UDP_SESSION {
    uint16_t client_port;
    uint32_t orig_dest_ip;
    uint16_t orig_dest_port;
    SOCKET proxy_sock;
    uint32_t proxy_relay_ip;
    uint16_t proxy_relay_port;
    ULONGLONG last_activity;
    struct UDP_SESSION *next;
} UDP_SESSION;

static UDP_SESSION *g_udp_sessions = NULL;
static CRITICAL_SECTION g_udp_session_lock;

// 初始化 UDP 中继服务器
bool UdpRelay_Init(void) {
    InitializeCriticalSection(&g_udp_session_lock);
    return true;
}

// 清理 UDP 中继服务器
void UdpRelay_Cleanup(void) {
    // 清理所有会话
    EnterCriticalSection(&g_udp_session_lock);

    UDP_SESSION *session = g_udp_sessions;
    while (session != NULL) {
        UDP_SESSION *next = session->next;
        if (session->proxy_sock != INVALID_SOCKET) {
            closesocket(session->proxy_sock);
        }
        free(session);
        session = next;
    }
    g_udp_sessions = NULL;

    LeaveCriticalSection(&g_udp_session_lock);
    DeleteCriticalSection(&g_udp_session_lock);
}

// 查找或创建 UDP 会话
static UDP_SESSION *find_or_create_session(uint16_t client_port, uint32_t orig_dest_ip,
                                          uint16_t orig_dest_port) {
    EnterCriticalSection(&g_udp_session_lock);

    // 查找现有会话
    UDP_SESSION *session = g_udp_sessions;
    while (session != NULL) {
        if (session->client_port == client_port) {
            session->last_activity = GetTickCount64();
            LeaveCriticalSection(&g_udp_session_lock);
            return session;
        }
        session = session->next;
    }

    // 创建新会话
    session = (UDP_SESSION *)malloc(sizeof(UDP_SESSION));
    if (session == NULL) {
        LeaveCriticalSection(&g_udp_session_lock);
        return NULL;
    }

    memset(session, 0, sizeof(UDP_SESSION));
    session->client_port = client_port;
    session->orig_dest_ip = orig_dest_ip;
    session->orig_dest_port = orig_dest_port;
    session->last_activity = GetTickCount64();

    // 连接到代理服务器
    session->proxy_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (session->proxy_sock == INVALID_SOCKET) {
        free(session);
        LeaveCriticalSection(&g_udp_session_lock);
        return NULL;
    }

    struct sockaddr_in proxy_addr;
    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = inet_addr(g_proxy_config.host);
    proxy_addr.sin_port = htons(g_proxy_config.port);

    if (connect(session->proxy_sock, (struct sockaddr *)&proxy_addr,
               sizeof(proxy_addr)) == SOCKET_ERROR) {
        closesocket(session->proxy_sock);
        free(session);
        LeaveCriticalSection(&g_udp_session_lock);
        return NULL;
    }

    // SOCKS5 握手
    if (!Socks5_Handshake(session->proxy_sock, &g_proxy_config)) {
        closesocket(session->proxy_sock);
        free(session);
        LeaveCriticalSection(&g_udp_session_lock);
        return NULL;
    }

    // SOCKS5 UDP ASSOCIATE
    if (!Socks5_UdpAssociate(session->proxy_sock, &session->proxy_relay_ip,
                            &session->proxy_relay_port)) {
        closesocket(session->proxy_sock);
        free(session);
        LeaveCriticalSection(&g_udp_session_lock);
        return NULL;
    }

    // 添加到会话列表
    session->next = g_udp_sessions;
    g_udp_sessions = session;

    LeaveCriticalSection(&g_udp_session_lock);
    return session;
}

// 清理过期会话
static void cleanup_expired_sessions(void) {
    ULONGLONG current_time = GetTickCount64();

    EnterCriticalSection(&g_udp_session_lock);

    UDP_SESSION **prev = &g_udp_sessions;
    UDP_SESSION *session = g_udp_sessions;

    while (session != NULL) {
        // 超时时间：60 秒
        if (current_time - session->last_activity > 60000) {
            *prev = session->next;
            if (session->proxy_sock != INVALID_SOCKET) {
                closesocket(session->proxy_sock);
            }
            free(session);
            session = *prev;
        } else {
            prev = &session->next;
            session = session->next;
        }
    }

    LeaveCriticalSection(&g_udp_session_lock);
}

// 启动 UDP 中继服务器
bool UdpRelay_Start(void) {
    // 创建 UDP 套接字
    g_udp_relay_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_udp_relay_socket == INVALID_SOCKET) {
        return false;
    }

    // 绑定到本地地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(LOCAL_UDP_RELAY_PORT);

    if (bind(g_udp_relay_socket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(g_udp_relay_socket);
        g_udp_relay_socket = INVALID_SOCKET;
        return false;
    }

    // 创建服务器线程
    g_udp_relay_thread = CreateThread(NULL, 0, UdpRelay_ServerThread, NULL, 0, NULL);
    if (g_udp_relay_thread == NULL) {
        closesocket(g_udp_relay_socket);
        g_udp_relay_socket = INVALID_SOCKET;
        return false;
    }

    return true;
}

// 停止 UDP 中继服务器
void UdpRelay_Stop(void) {
    // 关闭套接字
    if (g_udp_relay_socket != INVALID_SOCKET) {
        closesocket(g_udp_relay_socket);
        g_udp_relay_socket = INVALID_SOCKET;
    }

    // 等待服务器线程退出
    if (g_udp_relay_thread != NULL) {
        WaitForSingleObject(g_udp_relay_thread, INFINITE);
        CloseHandle(g_udp_relay_thread);
        g_udp_relay_thread = NULL;
    }
}

// UDP 中继服务器线程
DWORD WINAPI UdpRelay_ServerThread(LPVOID param) {
    unsigned char buffer[65536];
    struct sockaddr_in client_addr;
    int addr_len;
    ULONGLONG last_cleanup = GetTickCount64();

    while (g_running) {
        // 设置接收超时
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(g_udp_relay_socket, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(0, &read_fds, NULL, NULL, &timeout);
        if (ret == SOCKET_ERROR || ret == 0) {
            // 定期清理过期会话
            ULONGLONG current_time = GetTickCount64();
            if (current_time - last_cleanup > 10000) {
                cleanup_expired_sessions();
                last_cleanup = current_time;
            }
            continue;
        }

        // 接收数据
        addr_len = sizeof(client_addr);
        int received = recvfrom(g_udp_relay_socket, (char *)buffer, sizeof(buffer), 0,
                               (struct sockaddr *)&client_addr, &addr_len);

        if (received <= 0) {
            continue;
        }

        // 获取客户端端口
        uint16_t client_port = ntohs(client_addr.sin_port);

        // 获取原始目标地址
        uint32_t orig_dest_ip;
        uint16_t orig_dest_port;

        if (!Connection_Get(client_port, &orig_dest_ip, &orig_dest_port)) {
            continue;
        }

        // 查找或创建会话
        UDP_SESSION *session = find_or_create_session(client_port, orig_dest_ip, orig_dest_port);
        if (session == NULL) {
            continue;
        }

        // 构造 SOCKS5 UDP 请求
        // +----+------+------+----------+----------+----------+
        // |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
        // +----+------+------+----------+----------+----------+
        // | 2  |  1   |  1   | Variable |    2     | Variable |
        // +----+------+------+----------+----------+----------+

        unsigned char socks5_packet[65536];
        int socks5_len = 0;

        socks5_packet[socks5_len++] = 0x00; // RSV
        socks5_packet[socks5_len++] = 0x00; // RSV
        socks5_packet[socks5_len++] = 0x00; // FRAG
        socks5_packet[socks5_len++] = SOCKS5_ATYP_IPV4; // ATYP

        // 目标 IP
        socks5_packet[socks5_len++] = (orig_dest_ip >> 0) & 0xFF;
        socks5_packet[socks5_len++] = (orig_dest_ip >> 8) & 0xFF;
        socks5_packet[socks5_len++] = (orig_dest_ip >> 16) & 0xFF;
        socks5_packet[socks5_len++] = (orig_dest_ip >> 24) & 0xFF;

        // 目标端口
        socks5_packet[socks5_len++] = (orig_dest_port >> 8) & 0xFF;
        socks5_packet[socks5_len++] = (orig_dest_port >> 0) & 0xFF;

        // 数据
        memcpy(socks5_packet + socks5_len, buffer, received);
        socks5_len += received;

        // 发送到代理服务器
        struct sockaddr_in proxy_relay_addr;
        memset(&proxy_relay_addr, 0, sizeof(proxy_relay_addr));
        proxy_relay_addr.sin_family = AF_INET;
        proxy_relay_addr.sin_addr.s_addr = htonl(session->proxy_relay_ip);
        proxy_relay_addr.sin_port = htons(session->proxy_relay_port);

        // 创建临时 UDP 套接字发送数据
        SOCKET udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udp_sock != INVALID_SOCKET) {
            sendto(udp_sock, (const char *)socks5_packet, socks5_len, 0,
                  (struct sockaddr *)&proxy_relay_addr, sizeof(proxy_relay_addr));

            // 接收响应（带超时）
            fd_set recv_fds;
            FD_ZERO(&recv_fds);
            FD_SET(udp_sock, &recv_fds);

            struct timeval recv_timeout;
            recv_timeout.tv_sec = 5;
            recv_timeout.tv_usec = 0;

            if (select(0, &recv_fds, NULL, NULL, &recv_timeout) > 0) {
                int response_len = recvfrom(udp_sock, (char *)socks5_packet,
                                           sizeof(socks5_packet), 0, NULL, NULL);

                if (response_len > 10) {
                    // 解析 SOCKS5 UDP 响应，跳过头部
                    sendto(g_udp_relay_socket, (const char *)(socks5_packet + 10),
                          response_len - 10, 0,
                          (struct sockaddr *)&client_addr, sizeof(client_addr));
                }
            }

            closesocket(udp_sock);
        }
    }

    return 0;
}
