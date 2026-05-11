#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include "../include/tunnelproxy.h"

// 初始化数据包处理模块
bool PacketHandler_Init(void);

// 清理数据包处理模块
void PacketHandler_Cleanup(void);

// 启动数据包处理线程
bool PacketHandler_Start(void);

// 停止数据包处理线程
void PacketHandler_Stop(void);

// 数据包处理线程函数
DWORD WINAPI PacketHandler_Thread(LPVOID param);

#endif // PACKET_HANDLER_H
