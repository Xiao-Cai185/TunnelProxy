#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include "../include/tunnelproxy.h"

// 匹配规则
RuleAction Rule_Match(const char *process_name, uint32_t dest_ip, uint16_t dest_port, bool is_udp);

// 检查进程规则并返回动作
RuleAction Rule_CheckProcess(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip,
                             uint16_t dest_port, bool is_udp, DWORD *out_pid);

#endif // RULE_ENGINE_H
