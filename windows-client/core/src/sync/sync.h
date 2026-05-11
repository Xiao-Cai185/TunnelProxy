#ifndef SYNC_H
#define SYNC_H

#include "../include/tunnelproxy.h"

// 初始化同步模块
bool Sync_Init(void);

// 清理同步模块
void Sync_Cleanup(void);

// 设置同步服务器
bool Sync_SetServer(const char *server_url, const char *device_id);

// 同步策略（从服务器拉取）
bool Sync_PullPolicies(void);

// 上报设备状态
bool Sync_ReportStatus(void);

#endif // SYNC_H
