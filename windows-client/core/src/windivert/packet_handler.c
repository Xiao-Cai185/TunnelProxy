#include "../include/tunnelproxy.h"
#include "windivert.h"
#include "utils/process.h"
#include "rules/rule_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 外部变量
extern HANDLE g_windivert_handle;
extern bool g_running;
extern ProxyConfig g_proxy_config;
extern ConnectionStats g_stats;

// 数据包处理线程句柄
static HANDLE g_packet_threads[PACKET_THREAD_COUNT] = {NULL};

// 本地中继服务器地址
#define LOCAL_RELAY_IP 0x0100007F  // 127.0.0.1 (little-endian)
#define LOCAL_TCP_RELAY_PORT 34010
#define LOCAL_UDP_RELAY_PORT 34011

// 初始化数据包处理模块
bool PacketHandler_Init(void) {
    // 无需特殊初始化
    return true;
}

// 清理数据包处理模块
void PacketHandler_Cleanup(void) {
    // 清理由 PacketHandler_Stop 完成
}

// 启动数据包处理线程
bool PacketHandler_Start(void) {
    if (g_windivert_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    // 创建多个数据包处理线程
    for (int i = 0; i < PACKET_THREAD_COUNT; i++) {
        g_packet_threads[i] = CreateThread(NULL, 0, PacketHandler_Thread,
                                          (LPVOID)(intptr_t)i, 0, NULL);
        if (g_packet_threads[i] == NULL) {
            // 创建失败，停止已创建的线程
            PacketHandler_Stop();
            return false;
        }
    }

    return true;
}

// 停止数据包处理线程
void PacketHandler_Stop(void) {
    // 等待所有线程退出
    for (int i = 0; i < PACKET_THREAD_COUNT; i++) {
        if (g_packet_threads[i] != NULL) {
            WaitForSingleObject(g_packet_threads[i], INFINITE);
            CloseHandle(g_packet_threads[i]);
            g_packet_threads[i] = NULL;
        }
    }
}

// 处理出站数据包
static bool handle_outbound_packet(unsigned char *packet, UINT packet_len,
                                   WINDIVERT_ADDRESS *addr) {
    PWINDIVERT_IPHDR ip_header = NULL;
    PWINDIVERT_TCPHDR tcp_header = NULL;
    PWINDIVERT_UDPHDR udp_header = NULL;

    // 解析数据包
    WinDivertHelperParsePacket(packet, packet_len, &ip_header, NULL, NULL, NULL,
                              NULL, &tcp_header, &udp_header, NULL, NULL, NULL, NULL);

    if (ip_header == NULL) {
        return true; // 非 IP 数据包，直接放行
    }

    bool is_udp = (udp_header != NULL);
    bool is_tcp = (tcp_header != NULL);

    if (!is_tcp && !is_udp) {
        return true; // 非 TCP/UDP 数据包，直接放行
    }

    // 提取连接信息
    uint32_t src_ip = ntohl(ip_header->SrcAddr);
    uint32_t dest_ip = ntohl(ip_header->DstAddr);
    uint16_t src_port = is_tcp ? ntohs(tcp_header->SrcPort) : ntohs(udp_header->SrcPort);
    uint16_t dest_port = is_tcp ? ntohs(tcp_header->DstPort) : ntohs(udp_header->DstPort);

    // 检查是否是本地中继端口（避免循环）
    if (dest_ip == LOCAL_RELAY_IP &&
        (dest_port == LOCAL_TCP_RELAY_PORT || dest_port == LOCAL_UDP_RELAY_PORT)) {
        return true; // 放行到本地中继服务器的连接
    }

    // 检查规则并获取动作
    DWORD pid = 0;
    RuleAction action = Rule_CheckProcess(src_ip, src_port, dest_ip, dest_port, is_udp, &pid);

    switch (action) {
    case RULE_ACTION_BLOCK:
        // 丢弃数据包
        return false;

    case RULE_ACTION_DIRECT:
        // 直接放行
        return true;

    case RULE_ACTION_PROXY:
        // 重定向到本地中继服务器
        if (is_tcp) {
            // TCP: 仅重定向 SYN 包（新连接）
            if (tcp_header->Syn && !tcp_header->Ack) {
                // 保存原始目标地址
                Connection_Add(src_port, src_ip, dest_ip, dest_port);

                // 修改目标地址为本地中继服务器
                ip_header->DstAddr = htonl(LOCAL_RELAY_IP);
                tcp_header->DstPort = htons(LOCAL_TCP_RELAY_PORT);

                // 重新计算校验和
                WinDivertHelperCalcChecksums(packet, packet_len, addr, 0);
            }
        } else {
            // UDP: 重定向所有数据包
            // 检查是否已跟踪
            if (!Connection_IsTracked(src_port)) {
                Connection_Add(src_port, src_ip, dest_ip, dest_port);
            }

            // 修改目标地址为本地中继服务器
            ip_header->DstAddr = htonl(LOCAL_RELAY_IP);
            udp_header->DstPort = htons(LOCAL_UDP_RELAY_PORT);

            // 重新计算校验和
            WinDivertHelperCalcChecksums(packet, packet_len, addr, 0);
        }
        return true;

    default:
        return true;
    }
}

// 处理入站数据包
static bool handle_inbound_packet(unsigned char *packet, UINT packet_len,
                                  WINDIVERT_ADDRESS *addr) {
    PWINDIVERT_IPHDR ip_header = NULL;
    PWINDIVERT_TCPHDR tcp_header = NULL;
    PWINDIVERT_UDPHDR udp_header = NULL;

    // 解析数据包
    WinDivertHelperParsePacket(packet, packet_len, &ip_header, NULL, NULL, NULL,
                              NULL, &tcp_header, &udp_header, NULL, NULL, NULL, NULL);

    if (ip_header == NULL) {
        return true;
    }

    bool is_udp = (udp_header != NULL);
    bool is_tcp = (tcp_header != NULL);

    if (!is_tcp && !is_udp) {
        return true;
    }

    // 提取连接信息
    uint32_t src_ip = ntohl(ip_header->SrcAddr);
    uint16_t dest_port = is_tcp ? ntohs(tcp_header->DstPort) : ntohs(udp_header->DstPort);

    // 检查是否是从本地中继服务器返回的数据包
    if (src_ip == LOCAL_RELAY_IP &&
        (is_tcp ? ntohs(tcp_header->SrcPort) == LOCAL_TCP_RELAY_PORT :
                 ntohs(udp_header->SrcPort) == LOCAL_UDP_RELAY_PORT)) {

        // 检查是否有跟踪的连接
        if (Connection_IsTracked(dest_port)) {
            uint32_t orig_dest_ip;
            uint16_t orig_dest_port;

            // 获取原始目标地址
            if (Connection_Get(dest_port, &orig_dest_ip, &orig_dest_port)) {
                // 恢复原始源地址
                ip_header->SrcAddr = htonl(orig_dest_ip);
                if (is_tcp) {
                    tcp_header->SrcPort = htons(orig_dest_port);

                    // TCP FIN 包，清理连接
                    if (tcp_header->Fin) {
                        Connection_Remove(dest_port);
                    }
                } else {
                    udp_header->SrcPort = htons(orig_dest_port);
                }

                // 重新计算校验和
                WinDivertHelperCalcChecksums(packet, packet_len, addr, 0);
            }
        }
    }

    return true;
}

// 数据包处理线程函数
DWORD WINAPI PacketHandler_Thread(LPVOID param) {
    int thread_id = (int)(intptr_t)param;
    unsigned char packet[PACKET_BUFFER_SIZE];
    UINT packet_len;
    WINDIVERT_ADDRESS addr;

    while (g_running) {
        // 接收数据包
        if (!WinDivertRecv(g_windivert_handle, packet, sizeof(packet),
                          &packet_len, &addr)) {
            DWORD error = GetLastError();
            if (error == ERROR_NO_DATA || error == ERROR_OPERATION_ABORTED) {
                // 正常退出
                break;
            }
            // 其他错误，继续处理
            continue;
        }

        bool should_reinject = true;

        // 根据方向处理数据包
        if (addr.Outbound) {
            should_reinject = handle_outbound_packet(packet, packet_len, &addr);
        } else {
            should_reinject = handle_inbound_packet(packet, packet_len, &addr);
        }

        // 重新注入数据包
        if (should_reinject) {
            WinDivertSend(g_windivert_handle, packet, packet_len, NULL, &addr);
        }
    }

    return 0;
}
