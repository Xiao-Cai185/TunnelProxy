#ifndef UDP_RELAY_H
#define UDP_RELAY_H

#include "../include/tunnelproxy.h"

// 初始化 UDP 中继服务器
bool UdpRelay_Init(void);

// 清理 UDP 中继服务器
void UdpRelay_Cleanup(void);

// 启动 UDP 中继服务器
bool UdpRelay_Start(void);

// 停止 UDP 中继服务器
void UdpRelay_Stop(void);

// UDP 中继服务器线程
DWORD WINAPI UdpRelay_ServerThread(LPVOID param);

#endif // UDP_RELAY_H
