#include "../include/tunnelproxy.h"
#include "proxy/socks5.h"
#include "proxy/http_connect.h"
#include "utils/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 外部变量
extern bool g_running;
extern ProxyConfig g_proxy_config;

// TCP 中继服务器
static SOCKET g_tcp_relay_socket = INVALID_SOCKET;
static HANDLE g_tcp_relay_thread = NULL;

// 连接参数结构
typedef struct TCP_CONNECTION_PARAM {
    SOCKET client_sock;
    uint16_t client_port;
} TCP_CONNECTION_PARAM;

// 初始化 TCP 中继服务器
bool TcpRelay_Init(void) {
    // 初始化 Winsock
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return false;
    }

    return true;
}

// 清理 TCP 中继服务器
void TcpRelay_Cleanup(void) {
    WSACleanup();
}

// 启动 TCP 中继服务器
bool TcpRelay_Start(void) {
    // 创建监听套接字
    g_tcp_relay_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_tcp_relay_socket == INVALID_SOCKET) {
        return false;
    }

    // 设置地址重用
    int reuse = 1;
    setsockopt(g_tcp_relay_socket, SOL_SOCKET, SO_REUSEADDR,
              (const char *)&reuse, sizeof(reuse));

    // 绑定到本地地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(LOCAL_TCP_RELAY_PORT);

    if (bind(g_tcp_relay_socket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(g_tcp_relay_socket);
        g_tcp_relay_socket = INVALID_SOCKET;
        return false;
    }

    // 开始监听
    if (listen(g_tcp_relay_socket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(g_tcp_relay_socket);
        g_tcp_relay_socket = INVALID_SOCKET;
        return false;
    }

    // 创建服务器线程
    g_tcp_relay_thread = CreateThread(NULL, 0, TcpRelay_ServerThread, NULL, 0, NULL);
    if (g_tcp_relay_thread == NULL) {
        closesocket(g_tcp_relay_socket);
        g_tcp_relay_socket = INVALID_SOCKET;
        return false;
    }

    return true;
}

// 停止 TCP 中继服务器
void TcpRelay_Stop(void) {
    // 关闭监听套接字
    if (g_tcp_relay_socket != INVALID_SOCKET) {
        closesocket(g_tcp_relay_socket);
        g_tcp_relay_socket = INVALID_SOCKET;
    }

    // 等待服务器线程退出
    if (g_tcp_relay_thread != NULL) {
        WaitForSingleObject(g_tcp_relay_thread, INFINITE);
        CloseHandle(g_tcp_relay_thread);
        g_tcp_relay_thread = NULL;
    }
}

// 数据转发函数
static void relay_data(SOCKET src_sock, SOCKET dst_sock) {
    char buffer[8192];
    fd_set read_fds;
    struct timeval timeout;

    while (g_running) {
        FD_ZERO(&read_fds);
        FD_SET(src_sock, &read_fds);
        FD_SET(dst_sock, &read_fds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(0, &read_fds, NULL, NULL, &timeout);
        if (ret == SOCKET_ERROR) {
            break;
        }

        if (ret == 0) {
            continue; // 超时，继续循环
        }

        // 从客户端读取数据并发送到代理
        if (FD_ISSET(src_sock, &read_fds)) {
            int received = recv(src_sock, buffer, sizeof(buffer), 0);
            if (received <= 0) {
                break;
            }

            int sent = 0;
            while (sent < received) {
                int ret = send(dst_sock, buffer + sent, received - sent, 0);
                if (ret == SOCKET_ERROR) {
                    return;
                }
                sent += ret;
            }
        }

        // 从代理读取数据并发送到客户端
        if (FD_ISSET(dst_sock, &read_fds)) {
            int received = recv(dst_sock, buffer, sizeof(buffer), 0);
            if (received <= 0) {
                break;
            }

            int sent = 0;
            while (sent < received) {
                int ret = send(src_sock, buffer + sent, received - sent, 0);
                if (ret == SOCKET_ERROR) {
                    return;
                }
                sent += ret;
            }
        }
    }
}

// TCP 连接处理线程
DWORD WINAPI TcpRelay_ConnectionThread(LPVOID param) {
    TCP_CONNECTION_PARAM *conn_param = (TCP_CONNECTION_PARAM *)param;
    SOCKET client_sock = conn_param->client_sock;
    uint16_t client_port = conn_param->client_port;
    free(conn_param);

    // 获取原始目标地址
    uint32_t orig_dest_ip;
    uint16_t orig_dest_port;

    if (!Connection_Get(client_port, &orig_dest_ip, &orig_dest_port)) {
        closesocket(client_sock);
        return 0;
    }

    // 连接到代理服务器
    SOCKET proxy_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (proxy_sock == INVALID_SOCKET) {
        closesocket(client_sock);
        return 0;
    }

    // 设置超时
    DWORD timeout = 10000; // 10 秒
    setsockopt(proxy_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(proxy_sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));

    struct sockaddr_in proxy_addr;
    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = inet_addr(g_proxy_config.host);
    proxy_addr.sin_port = htons(g_proxy_config.port);

    if (connect(proxy_sock, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) == SOCKET_ERROR) {
        closesocket(proxy_sock);
        closesocket(client_sock);
        return 0;
    }

    // 根据代理类型进行握手
    bool handshake_ok = false;

    if (g_proxy_config.type == PROXY_TYPE_SOCKS5) {
        // SOCKS5 握手
        if (Socks5_Handshake(proxy_sock, &g_proxy_config)) {
            // SOCKS5 连接请求
            handshake_ok = Socks5_Connect(proxy_sock, orig_dest_ip, orig_dest_port);
        }
    } else if (g_proxy_config.type == PROXY_TYPE_HTTP) {
        // HTTP CONNECT 请求
        handshake_ok = HttpConnect_Connect(proxy_sock, orig_dest_ip, orig_dest_port);
    }

    if (!handshake_ok) {
        closesocket(proxy_sock);
        closesocket(client_sock);
        return 0;
    }

    // 开始转发数据
    relay_data(client_sock, proxy_sock);

    // 清理
    closesocket(proxy_sock);
    closesocket(client_sock);

    return 0;
}

// TCP 中继服务器线程
DWORD WINAPI TcpRelay_ServerThread(LPVOID param) {
    while (g_running) {
        // 接受客户端连接
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);

        SOCKET client_sock = accept(g_tcp_relay_socket,
                                    (struct sockaddr *)&client_addr, &addr_len);

        if (client_sock == INVALID_SOCKET) {
            if (!g_running) {
                break;
            }
            continue;
        }

        // 获取客户端端口
        uint16_t client_port = ntohs(client_addr.sin_port);

        // 创建连接参数
        TCP_CONNECTION_PARAM *conn_param = (TCP_CONNECTION_PARAM *)malloc(sizeof(TCP_CONNECTION_PARAM));
        if (conn_param == NULL) {
            closesocket(client_sock);
            continue;
        }

        conn_param->client_sock = client_sock;
        conn_param->client_port = client_port;

        // 创建连接处理线程
        HANDLE thread = CreateThread(NULL, 0, TcpRelay_ConnectionThread,
                                     conn_param, 0, NULL);
        if (thread != NULL) {
            CloseHandle(thread); // 分离线程
        } else {
            free(conn_param);
            closesocket(client_sock);
        }
    }

    return 0;
}
