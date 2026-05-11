# TunnelProxy Windows Client - Core Engine

## 项目说明

这是 TunnelProxy Windows 客户端的核心引擎，基于 WinDivert 实现进程级流量拦截和代理。

## 技术方案

基于 ProxyBridge 的成熟方案：
- **WinDivert 2.2-A** - 内核级数据包拦截
- **多线程架构** - 高性能数据包处理
- **进程识别** - GetExtendedTcpTable API
- **规则匹配** - 应用程序名称匹配
- **代理协议** - SOCKS5 和 HTTP CONNECT

## 项目结构

```
core/
├── include/
│   └── tunnelproxy.h          # 公共 API 头文件
├── src/
│   ├── tunnelproxy.c          # 核心引擎实现
│   ├── windivert/
│   │   └── packet_handler.c   # 数据包处理
│   ├── proxy/
│   │   ├── socks5.c           # SOCKS5 协议实现
│   │   ├── http.c             # HTTP CONNECT 实现
│   │   └── relay_server.c     # 本地中继服务器
│   ├── rules/
│   │   └── rule_engine.c      # 规则匹配引擎
│   ├── sync/
│   │   └── http_client.c      # HTTP 客户端（策略同步）
│   └── utils/
│       ├── process.c          # 进程识别
│       └── cache.c            # PID 缓存
└── README.md
```

## API 使用示例

```c
#include "tunnelproxy.h"

// 日志回调
void on_log(const char *message) {
    printf("[LOG] %s\n", message);
}

// 连接回调
void on_connection(const char *protocol, const char *process, 
                   const char *destination, uint16_t port, 
                   const char *action) {
    printf("[CONN] %s | %s -> %s:%d | %s\n", 
           protocol, process, destination, port, action);
}

int main() {
    // 1. 初始化
    if (!TunnelProxy_Init(on_log, on_connection)) {
        return 1;
    }
    
    // 2. 设置同步服务器
    TunnelProxy_SetSyncServer("http://localhost:8080", "device-123");
    
    // 3. 同步策略
    TunnelProxy_SyncPolicies();
    
    // 4. 设置代理配置
    ProxyConfig proxy = {
        .type = PROXY_TYPE_SOCKS5,
        .host = "127.0.0.1",
        .port = 1080,
        .username = "",
        .password = ""
    };
    TunnelProxy_SetProxyConfig(&proxy);
    
    // 5. 添加策略
    PolicyRule rule = {
        .app_name = "chrome.exe",
        .action = RULE_ACTION_PROXY,
        .enabled = true
    };
    TunnelProxy_AddPolicy(&rule);
    
    // 6. 启动引擎
    if (!TunnelProxy_Start()) {
        TunnelProxy_Cleanup();
        return 1;
    }
    
    // 7. 运行...
    printf("Press Enter to stop...\n");
    getchar();
    
    // 8. 停止和清理
    TunnelProxy_Stop();
    TunnelProxy_Cleanup();
    
    return 0;
}
```

## 编译说明

### 依赖项
- WinDivert 2.2-A 或更高版本
- Visual Studio 2019 或更高版本
- Windows SDK

### 编译步骤

1. 下载 WinDivert：https://reqrypt.org/windivert.html
2. 解压到 `C:\WinDivert-2.2.2-A`
3. 使用 Visual Studio 打开项目
4. 编译 Release 版本

或使用命令行：

```cmd
cl /O2 /I"C:\WinDivert-2.2.2-A\include" ^
   /I"include" ^
   src\tunnelproxy.c ^
   /link /LIBPATH:"C:\WinDivert-2.2.2-A\x64" ^
   WinDivert.lib ws2_32.lib iphlpapi.lib ^
   /OUT:tunnelproxy.dll /DLL
```

## 下一步开发

- [ ] 实现数据包处理线程
- [ ] 实现进程识别模块
- [ ] 实现规则匹配引擎
- [ ] 实现 SOCKS5 协议
- [ ] 实现 HTTP CONNECT
- [ ] 实现本地中继服务器
- [ ] 实现 HTTP 客户端（策略同步）
- [ ] 实现 PID 缓存机制

## 许可证

MIT License
