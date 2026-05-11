#ifndef CONFIG_H
#define CONFIG_H

#include "../include/tunnelproxy.h"

// 配置文件路径
#define CONFIG_FILE_PATH "tunnelproxy_config.json"

// 加载配置文件
bool Config_Load(void);

// 保存配置文件
bool Config_Save(void);

// 加载代理配置
bool Config_LoadProxyConfig(ProxyConfig *config);

// 保存代理配置
bool Config_SaveProxyConfig(const ProxyConfig *config);

// 加载策略规则
bool Config_LoadPolicies(PolicyRule **policies, int *count);

// 保存策略规则
bool Config_SavePolicies(const PolicyRule *policies, int count);

// 加载同步配置
bool Config_LoadSyncConfig(char *server_url, int url_size, char *device_id, int id_size);

// 保存同步配置
bool Config_SaveSyncConfig(const char *server_url, const char *device_id);

#endif // CONFIG_H
