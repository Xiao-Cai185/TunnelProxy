#ifndef PROCESS_H
#define PROCESS_H

#include "../include/tunnelproxy.h"

// 初始化进程识别模块
void Process_Init(void);

// 清理进程识别模块
void Process_Cleanup(void);

// 获取进程 ID
DWORD Process_GetPID(uint32_t src_ip, uint16_t src_port, bool is_udp);

// 获取进程名
bool Process_GetName(DWORD pid, char *name, DWORD name_size);

// 添加连接信息
void Connection_Add(uint16_t src_port, uint32_t src_ip, uint32_t dest_ip, uint16_t dest_port);

// 获取连接信息
bool Connection_Get(uint16_t src_port, uint32_t *dest_ip, uint16_t *dest_port);

// 检查连接是否被跟踪
bool Connection_IsTracked(uint16_t src_port);

// 移除连接信息
void Connection_Remove(uint16_t src_port);

// 记录连接日志
void Connection_Log(const char *protocol, const char *process, uint32_t dest_ip,
                   uint16_t dest_port, RuleAction action, DWORD pid);

#endif // PROCESS_H
