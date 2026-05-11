# TunnelProxy 开发要求记录

## 项目开发要求

### 2026-05-02 - 项目初始化

**用户需求：**
1. 新建项目文件夹 `tunnelproxy`
2. 项目分为两大部分：
   - Web 管理平台
   - 客户端（Windows/Mac/Linux）

**功能需求：**

#### Web 管理平台
1. 设备管理
   - 支持添加 Windows/Mac/Linux 三种设备
   - 每个设备有唯一标识
   - 显示设备在线状态

2. 策略配置
   - 配置应用程序名称
   - 设置动作：代理（PROXY）/ 直连（DIRECT）/ 丢弃（BLOCK）
   - 支持批量配置
   - 支持策略模板

3. 代理服务器配置
   - 配置上游代理服务器地址
   - 支持 SOCKS5 和 HTTP 代理
   - 支持认证信息

#### 客户端功能
1. 策略同步
   - 支持查看当前策略
   - 支持从服务器同步策略
   - 定期自动同步

2. 启动方式
   - 支持带参数启动：`--sync-server=http://server:port`
   - 默认打开需要输入同步地址才能使用
   - 记住上次使用的服务器地址

3. 流量处理
   - 基于 ProxyBridge 的代理方案
   - 进程级流量拦截
   - 规则匹配和动作执行

**技术要求：**

1. **引流代理方法**
   - 必须学习 ProxyBridge 的解决方案
   - 该方案已验证，非常稳定
   - Windows 使用 WinDivert
   - macOS 使用 Network Extension
   - Linux 使用 Netfilter NFQUEUE

2. **开发优先级**
   - 第一阶段：Web 管理平台 + Windows 客户端
   - 第二阶段：macOS 客户端
   - 第三阶段：Linux 客户端

3. **数据同步**
   - 客户端定期向服务器拉取策略
   - 支持手动刷新
   - 策略变更实时生效

**输出要求：**
- 完整的项目结构
- 清晰的代码组织
- 详细的 API 文档
- 部署说明文档

---

## 技术实现细节

### Web 管理平台

**数据模型：**
1. 设备表（Devices）
   - device_id: 设备唯一标识
   - device_name: 设备名称
   - platform: Windows/Mac/Linux
   - last_sync: 最后同步时间
   - status: 在线/离线

2. 策略表（Policies）
   - policy_id: 策略 ID
   - device_id: 关联设备
   - app_name: 应用程序名称
   - action: PROXY/DIRECT/BLOCK
   - enabled: 是否启用

3. 代理配置表（ProxyConfig）
   - config_id: 配置 ID
   - device_id: 关联设备
   - proxy_type: SOCKS5/HTTP
   - proxy_host: 代理服务器地址
   - proxy_port: 代理服务器端口
   - username: 认证用户名（可选）
   - password: 认证密码（可选）

**API 接口：**
1. 设备管理
   - POST /api/devices - 注册设备
   - GET /api/devices - 获取设备列表
   - GET /api/devices/:id - 获取设备详情
   - PUT /api/devices/:id - 更新设备信息
   - DELETE /api/devices/:id - 删除设备

2. 策略管理
   - GET /api/policies/:device_id - 获取设备策略
   - POST /api/policies - 创建策略
   - PUT /api/policies/:id - 更新策略
   - DELETE /api/policies/:id - 删除策略

3. 代理配置
   - GET /api/proxy-config/:device_id - 获取代理配置
   - POST /api/proxy-config - 创建代理配置
   - PUT /api/proxy-config/:id - 更新代理配置

4. 同步接口
   - GET /api/sync/:device_id - 客户端同步策略
   - POST /api/sync/heartbeat - 客户端心跳

### Windows 客户端

**核心模块：**
1. WinDivert 封装
   - 数据包拦截
   - 数据包修改
   - 数据包重新注入

2. 规则引擎
   - 进程识别
   - 规则匹配
   - 动作执行

3. 代理处理
   - SOCKS5 协议实现
   - HTTP CONNECT 实现
   - 本地中继服务器

4. 策略同步
   - HTTP API 调用
   - JSON 解析
   - 本地缓存

**配置文件格式：**
```json
{
  "sync_server": "http://server:port",
  "device_id": "unique-device-id",
  "last_sync": "2026-05-02T17:31:00Z",
  "policies": [
    {
      "app_name": "chrome.exe",
      "action": "PROXY"
    },
    {
      "app_name": "firefox.exe",
      "action": "DIRECT"
    }
  ],
  "proxy_config": {
    "type": "SOCKS5",
    "host": "127.0.0.1",
    "port": 1080,
    "username": "",
    "password": ""
  }
}
```
