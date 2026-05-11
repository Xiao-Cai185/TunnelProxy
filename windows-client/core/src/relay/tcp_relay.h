#ifndef TCP_RELAY_H
#define TCP_RELAY_H

#include "../include/tunnelproxy.h"

// 初始化 TCP 中继服务器
bool TcpRelay_Init(void);

// 清理 TCP 中继服务器
void TcpRelay_Cleanup(void);

// 启动 TCP 中继服务器
bool TcpRelay_Start(void);

// 停止 TCP 中继服务器
void TcpRelay_Stop(void);

// TCP 中继服务器线程
DWORD WINAPI TcpRelay_ServerThread(LPVOID param);

// TCP 连接处理线程
DWORD WINAPI TcpRelay_ConnectionThread(LPVOID param);

#endif // TCP_RELAY_H
