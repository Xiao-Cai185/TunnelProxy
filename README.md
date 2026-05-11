# TunnelProxy - 跨平台进程级代理管理系统

## 项目简介

TunnelProxy 是一个集中式管理的跨平台进程级代理系统，由 Web 管理平台和多平台客户端组成。

### 核心特性

- **集中式管理：** Web 平台统一配置和管理所有客户端的代理策略
- **跨平台支持：** Windows、macOS、Linux 三端客户端
- **进程级控制：** 精确到应用程序的流量控制
- **策略同步：** 客户端自动同步服务器端策略配置
- **三种动作：** 代理（PROXY）、直连（DIRECT）、丢弃（BLOCK）

## 项目架构

```
TunnelProxy/
├── web-platform/          # Web 管理平台
│   ├── backend/           # 后端 API 服务
│   └── frontend/          # 前端管理界面
├── windows-client/        # Windows 客户端
├── macos-client/          # macOS 客户端（待开发）
├── linux-client/          # Linux 客户端（待开发）
└── docs/                  # 项目文档
```

## 技术栈

### Web 管理平台
- **后端：** Node.js + Express / Python + Flask
- **前端：** React / Vue.js
- **数据库：** MySQL / PostgreSQL / MongoDB
- **API：** RESTful API

### Windows 客户端
- **核心引擎：** C (基于 WinDivert)
- **GUI：** C# + WPF / Avalonia UI
- **通信：** HTTP/HTTPS API 调用

### 代理技术
- **Windows：** WinDivert（内核级数据包拦截）
- **macOS：** Network Extension Framework
- **Linux：** Netfilter NFQUEUE

## 工作流程

1. **管理员配置策略**
   - 在 Web 平台创建设备
   - 配置应用程序代理规则
   - 设置代理服务器信息

2. **客户端同步策略**
   - 启动时输入同步服务器地址
   - 自动拉取最新策略配置
   - 定期检查策略更新

3. **流量处理**
   - 拦截应用程序网络流量
   - 根据策略执行动作（代理/直连/丢弃）
   - 上报连接日志到服务器

## 开发计划

### Phase 1: Web 管理平台 + Windows 客户端
- [x] 项目初始化
- [ ] Web 平台后端 API
- [ ] Web 平台前端界面
- [ ] Windows 客户端核心引擎
- [ ] Windows 客户端 GUI
- [ ] 策略同步机制

### Phase 2: macOS 客户端
- [ ] macOS 核心引擎
- [ ] macOS GUI

### Phase 3: Linux 客户端
- [ ] Linux 核心引擎
- [ ] Linux GUI

## 快速开始

### Web 管理平台

```bash
cd web-platform
npm install
npm run dev
```

### Windows 客户端

```bash
cd windows-client
# 编译说明待补充
```

## 许可证

MIT License

---

**开发日期：** 2026-05-02  
**基于技术：** ProxyBridge 代理方案
