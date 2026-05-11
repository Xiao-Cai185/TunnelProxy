# TunnelProxy Server

## 快速开始

### 安装依赖

```bash
go mod download
```

### 运行服务器

```bash
go run cmd/server/main.go
```

服务器将在 `http://localhost:8080` 启动

### API 文档

#### 设备管理

**创建设备**
```
POST /api/devices
Content-Type: application/json

{
  "name": "我的Windows电脑",
  "platform": "Windows"
}
```

**获取设备列表**
```
GET /api/devices
```

**获取设备详情**
```
GET /api/devices/:id
```

**更新设备**
```
PUT /api/devices/:id
Content-Type: application/json

{
  "name": "更新后的名称",
  "status": "online"
}
```

**删除设备**
```
DELETE /api/devices/:id
```

#### 同步接口

**同步策略**
```
GET /api/sync/:device_id
```

响应示例：
```json
{
  "device_id": "xxx",
  "policies": [
    {
      "id": "xxx",
      "device_id": "xxx",
      "app_name": "chrome.exe",
      "action": "PROXY",
      "enabled": true
    }
  ],
  "proxy_config": {
    "type": "SOCKS5",
    "host": "127.0.0.1",
    "port": 1080
  },
  "sync_time": "2026-05-02T17:31:00Z"
}
```

**心跳**
```
POST /api/sync/heartbeat
Content-Type: application/json

{
  "device_id": "xxx",
  "status": "online"
}
```

## 项目结构

```
backend/
├── cmd/
│   └── server/
│       └── main.go           # 主程序入口
├── internal/
│   ├── database/
│   │   └── database.go       # 数据库初始化
│   ├── handlers/
│   │   ├── device_handler.go # 设备处理器
│   │   └── sync_handler.go   # 同步处理器
│   ├── models/
│   │   ├── device.go         # 设备模型
│   │   ├── policy.go         # 策略模型
│   │   ├── proxy_config.go   # 代理配置模型
│   │   └── sync.go           # 同步模型
│   └── services/
│       ├── device_service.go       # 设备服务
│       ├── policy_service.go       # 策略服务
│       └── proxy_config_service.go # 代理配置服务
├── go.mod
└── README.md
```

## 数据库

使用 SQLite 数据库，文件名为 `tunnelproxy.db`

### 表结构

**devices** - 设备表
- id: 设备ID
- name: 设备名称
- platform: 平台（Windows/macOS/Linux）
- last_sync: 最后同步时间
- status: 状态（online/offline）
- created_at: 创建时间
- updated_at: 更新时间

**policies** - 策略表
- id: 策略ID
- device_id: 设备ID
- app_name: 应用程序名称
- action: 动作（PROXY/DIRECT/BLOCK）
- enabled: 是否启用
- created_at: 创建时间
- updated_at: 更新时间

**proxy_configs** - 代理配置表
- id: 配置ID
- device_id: 设备ID
- type: 代理类型（SOCKS5/HTTP）
- host: 代理服务器地址
- port: 代理服务器端口
- username: 用户名
- password: 密码
- created_at: 创建时间
- updated_at: 更新时间
