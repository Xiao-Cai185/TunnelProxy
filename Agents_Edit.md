# TunnelProxy 项目开发记录

## 第一章：项目架构与技术栈

### 项目概述
**项目名称：** TunnelProxy  
**项目类型：** 集中式管理的跨平台进程级代理系统  
**开发模式：** Web 管理平台 + 多平台客户端

### 核心功能

1. **Web 管理平台**
   - 设备管理（Windows/Mac/Linux）
   - 策略配置（应用程序 + 动作）
   - 代理服务器配置
   - 客户端状态监控
   - 连接日志查看

2. **客户端功能**
   - 策略同步（从服务器拉取）
   - 流量拦截（基于 ProxyBridge 方案）
   - 规则匹配（进程名 + IP + 端口）
   - 动作执行（PROXY/DIRECT/BLOCK）
   - 日志上报

### 技术栈详情

#### Web 管理平台
- **后端框架：** 待定（Node.js/Python）
- **前端框架：** 待定（React/Vue.js）
- **数据库：** 待定（MySQL/PostgreSQL/MongoDB）
- **API 设计：** RESTful API
- **认证方式：** JWT Token

#### Windows 客户端
- **核心引擎：** C（基于 WinDivert）
- **GUI 框架：** 待定（WPF/Avalonia UI）
- **通信协议：** HTTP/HTTPS
- **配置存储：** JSON 文件

#### 代理技术方案
- **Windows：** WinDivert 2.2-A（内核级数据包拦截）
- **macOS：** Network Extension Framework（系统扩展）
- **Linux：** Netfilter NFQUEUE（内核特性）

### 项目结构
```
tunnelproxy/
├── web-platform/          # Web 管理平台
│   ├── backend/           # 后端 API 服务
│   │   ├── src/
│   │   │   ├── controllers/    # 控制器
│   │   │   ├── models/         # 数据模型
│   │   │   ├── routes/         # 路由
│   │   │   ├── services/       # 业务逻辑
│   │   │   └── utils/          # 工具函数
│   │   ├── config/             # 配置文件
│   │   └── package.json
│   └── frontend/          # 前端管理界面
│       ├── src/
│       │   ├── components/     # 组件
│       │   ├── pages/          # 页面
│       │   ├── services/       # API 服务
│       │   └── utils/          # 工具函数
│       └── package.json
├── windows-client/        # Windows 客户端
│   ├── core/              # 核心引擎（C）
│   │   ├── src/
│   │   │   ├── windivert/      # WinDivert 封装
│   │   │   ├── proxy/          # 代理处理
│   │   │   ├── rules/          # 规则引擎
│   │   │   └── sync/           # 策略同步
│   │   └── include/
│   └── gui/               # GUI 界面
│       └── src/
├── macos-client/          # macOS 客户端（待开发）
├── linux-client/          # Linux 客户端（待开发）
└── docs/                  # 项目文档
    ├── api/               # API 文档
    ├── architecture/      # 架构设计
    └── deployment/        # 部署文档
```

---

## 开发时间线

### 2026-05-02 17:31 - 项目启动

**任务：** 初始化 TunnelProxy 项目

**用户需求：**
1. 创建新项目 tunnelproxy
2. 分为 Web 管理平台和客户端
3. Web 平台负责设置三端设备的应用策略
4. 客户端支持查看和同步策略
5. 支持带参数启动附带策略服务器地址
6. 默认打开需要输入同步地址才能使用
7. 先开发 Web 管理平台和 Windows 客户端
8. 引流代理方法学习 ProxyBridge 方案

**技术方案：**
- 基于 ProxyBridge 的 WinDivert 方案实现流量拦截
- Web 平台提供 RESTful API 供客户端同步策略
- 客户端定期轮询或 WebSocket 实时同步
- 策略包含：设备 ID、应用规则、代理配置

**当前进度：**
- ✅ 项目目录结构创建完成
- ✅ README 文档编写完成
- ✅ 技术栈选型完成（Go + Gin + SQLite + React + Avalonia UI）
- ✅ Web 后端基础框架完成
- ✅ Windows 核心引擎 API 设计完成
- 🔄 Windows 核心引擎实现进行中

**技术栈确认：**
- Web 后端：Go + Gin
- Web 前端：React + Ant Design
- 数据库：SQLite
- Windows GUI：Avalonia UI
- Windows 核心：C + WinDivert

**已完成模块：**
1. Web 后端
   - ✅ 数据库模型（Device, Policy, ProxyConfig）
   - ✅ 业务服务层（DeviceService, PolicyService, ProxyConfigService）
   - ✅ API 处理器（DeviceHandler, SyncHandler）
   - ✅ 主程序入口和路由配置
   - ✅ SQLite 数据库自动初始化

2. Windows 核心引擎
   - ✅ 公共 API 头文件设计（tunnelproxy.h）
   - ✅ 核心引擎框架实现（tunnelproxy.c）
   - ✅ 进程识别模块（process.c/h）
     - PID 缓存机制（1秒 TTL）
     - GetExtendedTcpTable/GetExtendedUdpTable API 集成
     - 进程名提取（QueryFullProcessImageNameA）
     - 连接跟踪哈希表
     - 连接日志记录（去重）
   - ✅ 规则匹配引擎（rule_engine.c/h）
     - 通配符模式匹配（前缀、后缀、中间、精确）
     - 策略遍历和匹配
     - 统计信息更新
     - 自身进程排除
   - ✅ 数据包处理模块（packet_handler.c/h）
     - WinDivert 数据包接收
     - IP/TCP/UDP 头部解析
     - 出站数据包处理（重定向到本地中继）
     - 入站数据包处理（恢复原始地址）
     - 多线程处理（4个线程）
     - 校验和重新计算
   - ✅ SOCKS5 协议处理（socks5.c/h）
     - 握手协商（无认证/用户名密码）
     - 用户名密码认证
     - CONNECT 命令
     - UDP ASSOCIATE 命令
   - ✅ HTTP CONNECT 协议处理（http_connect.c/h）
     - CONNECT 请求构造
     - Base64 认证编码
     - 响应解析
   - ✅ TCP 中继服务器（tcp_relay.c/h）
     - 监听本地端口 34010
     - 连接到上游代理
     - 双向数据转发
     - 多线程连接处理
   - ✅ UDP 中继服务器（udp_relay.c/h）
     - 监听本地端口 34011
     - SOCKS5 UDP 封装/解封装
     - 会话管理（60秒超时）
     - 过期会话清理
   - ✅ 策略同步模块（sync.c/h）
     - WinHTTP API 集成
     - HTTP GET 请求
     - JSON 策略解析
     - 设备状态上报
   - ✅ 配置管理模块（config.c/h）
     - JSON 配置文件读写
     - 代理配置加载/保存
     - 策略规则加载/保存
     - 同步配置加载/保存

---

### 2026-05-02 - Windows 客户端核心引擎完成

**任务：** 完成 Windows 客户端核心引擎的所有模块实现

**实现内容：**

1. **进程识别模块（process.c/h）**
   - 实现了基于哈希表的 PID 缓存机制，TTL 为 1 秒
   - 集成 Windows API（GetExtendedTcpTable/GetExtendedUdpTable）获取连接对应的进程 ID
   - 使用 QueryFullProcessImageNameA 从 PID 提取进程名
   - 实现连接跟踪哈希表，存储原始目标地址
   - 实现连接日志记录功能，带去重机制避免重复日志

2. **规则匹配引擎（rule_engine.c/h）**
   - 实现通配符模式匹配，支持前缀（"chr*"）、后缀（"*.exe"）、中间（"chr*me.exe"）和精确匹配
   - 实现策略遍历和匹配逻辑
   - 集成统计信息更新（总连接数、代理连接数、直连数、阻止数）
   - 实现自身进程排除逻辑，防止代理循环

3. **数据包处理模块（packet_handler.c/h）**
   - 实现 WinDivert 数据包接收循环
   - 实现 IP/TCP/UDP 头部解析
   - 实现出站数据包处理：根据规则动作重定向到本地中继服务器
   - 实现入站数据包处理：恢复原始源地址
   - 实现多线程数据包处理（4 个线程）
   - 使用 WinDivertHelperCalcChecksums 重新计算校验和

4. **SOCKS5 协议处理（socks5.c/h）**
   - 实现 SOCKS5 握手协商（支持无认证和用户名密码认证）
   - 实现用户名密码认证流程
   - 实现 CONNECT 命令（TCP 连接）
   - 实现 UDP ASSOCIATE 命令（UDP 关联）

5. **HTTP CONNECT 协议处理（http_connect.c/h）**
   - 实现 HTTP CONNECT 请求构造
   - 实现 Base64 编码用于 Proxy-Authorization 认证
   - 实现 HTTP 响应解析

6. **TCP 中继服务器（tcp_relay.c/h）**
   - 实现监听本地端口 34010
   - 实现连接到上游代理服务器（SOCKS5/HTTP）
   - 实现双向数据转发（使用 select 多路复用）
   - 实现多线程连接处理（每个连接一个线程）

7. **UDP 中继服务器（udp_relay.c/h）**
   - 实现监听本地端口 34011
   - 实现 SOCKS5 UDP 数据包封装和解封装
   - 实现 UDP 会话管理（基于客户端端口）
   - 实现会话超时机制（60 秒）
   - 实现过期会话自动清理

8. **策略同步模块（sync.c/h）**
   - 集成 WinHTTP API 实现 HTTP 客户端
   - 实现 HTTP GET 请求发送
   - 实现简单的 JSON 策略解析
   - 实现设备状态上报功能

9. **配置管理模块（config.c/h）**
   - 实现 JSON 配置文件读写
   - 实现代理配置加载和保存
   - 实现策略规则加载和保存
   - 实现同步配置加载和保存

10. **主程序集成（tunnelproxy.c）**
    - 更新 TunnelProxy_Init 集成所有模块初始化
    - 更新 TunnelProxy_Start 启动所有服务（数据包处理、TCP 中继、UDP 中继）
    - 更新 TunnelProxy_Stop 停止所有服务
    - 更新 TunnelProxy_Cleanup 清理所有模块
    - 实现 TunnelProxy_SyncPolicies 调用同步模块
    - 实现 TunnelProxy_SendHeartbeat 上报设备状态

**技术要点：**
- 使用 WinDivert 2.2-A 进行内核级数据包拦截
- 使用哈希表优化 PID 查询和连接跟踪性能
- 使用临界区（CRITICAL_SECTION）保证多线程安全
- 使用 select 实现高效的双向数据转发
- 支持 SOCKS5 和 HTTP CONNECT 两种代理协议
- 支持 TCP 和 UDP 流量代理

**文件结构：**
```
windows-client/core/src/
├── include/
│   └── tunnelproxy.h          # 公共 API 头文件
├── windivert/
│   ├── packet_handler.c/h     # 数据包处理
│   └── windivert.h            # WinDivert 库头文件
├── utils/
│   └── process.c/h            # 进程识别
├── rules/
│   └── rule_engine.c/h        # 规则匹配
├── proxy/
│   ├── socks5.c/h             # SOCKS5 协议
│   └── http_connect.c/h       # HTTP CONNECT 协议
├── relay/
│   ├── tcp_relay.c/h          # TCP 中继服务器
│   └── udp_relay.c/h          # UDP 中继服务器
├── sync/
│   └── sync.c/h               # 策略同步
├── config/
│   └── config.c/h             # 配置管理
└── tunnelproxy.c              # 主程序
```

**当前状态：**
- ✅ Windows 客户端核心引擎完全实现
- ⏭️ 下一步：创建 Windows 客户端 GUI（Avalonia UI）
- ⏭️ 待开发：Web 前端管理界面（React + Ant Design）

---

### 2026-05-02 - Web 前端管理平台完成

**任务：** 完成 Web 前端管理平台的项目结构和核心页面

**实现内容：**

1. **项目初始化**
   - 使用 Vite 创建 React 项目
   - 安装核心依赖：React 19、Ant Design 6、axios、react-router-dom
   - 配置开发服务器（端口 3000，API 代理到 8080）
   - 创建项目目录结构（pages、components、services、utils、layouts）

2. **API 服务层（services/api.js）**
   - 创建 axios 实例，配置 baseURL 和拦截器
   - 实现设备管理 API（增删改查）
   - 实现策略管理 API（增删改查、批量更新）
   - 实现代理配置 API（获取、更新）
   - 实现同步 API（策略同步、状态上报）

3. **设备管理页面（pages/DeviceManagement.jsx）**
   - 设备列表展示（表格形式）
   - 设备状态显示（在线/离线）
   - 平台标识（Windows/macOS/Linux）
   - 添加/编辑/删除设备功能
   - 最后同步时间显示
   - 刷新功能

4. **策略管理页面（pages/PolicyManagement.jsx）**
   - 设备选择器（下拉框）
   - 策略列表展示（应用名称、动作、状态）
   - 添加/编辑/删除策略功能
   - 策略启用/禁用开关
   - 动作类型（代理/直连/阻止）
   - 通配符支持说明

5. **代理配置页面（pages/ProxyConfig.jsx）**
   - 代理类型选择（SOCKS5/HTTP）
   - 服务器地址和端口配置
   - 用户名密码认证（可选）
   - 表单验证（IP/域名格式、端口范围）
   - 配置说明卡片

6. **主布局组件（layouts/MainLayout.jsx）**
   - 侧边栏导航菜单
   - 顶部标题栏
   - 可折叠侧边栏
   - 路由高亮显示
   - 响应式布局

7. **路由配置（App.jsx）**
   - React Router 集成
   - 路由定义（设备管理、策略管理、代理配置）
   - 默认路由重定向
   - Ant Design 中文语言包配置

**技术栈：**
- React 19.2.5
- Ant Design 6.3.7
- React Router DOM 7.14.2
- Axios 1.15.2
- Vite 8.0.10

**项目结构：**
```
frontend/
├── src/
│   ├── pages/
│   │   ├── DeviceManagement.jsx    # 设备管理页面
│   │   ├── PolicyManagement.jsx    # 策略管理页面
│   │   └── ProxyConfig.jsx         # 代理配置页面
│   ├── layouts/
│   │   └── MainLayout.jsx          # 主布局组件
│   ├── services/
│   │   └── api.js                  # API 服务层
│   ├── components/                 # 公共组件（待扩展）
│   ├── utils/                      # 工具函数（待扩展）
│   ├── App.jsx                     # 路由配置
│   ├── main.jsx                    # 入口文件
│   └── index.css                   # 全局样式
├── vite.config.js                  # Vite 配置
├── package.json                    # 项目配置
└── .gitignore                      # Git 忽略文件

```

**功能特性：**
- 完整的设备管理（CRUD）
- 灵活的策略配置（支持通配符）
- 代理服务器配置（SOCKS5/HTTP）
- 响应式设计，适配不同屏幕
- 中文界面，用户友好
- API 代理配置，开发环境无跨域问题

**启动命令：**
```bash
cd web-platform/frontend
npm run dev
# 访问：http://144.24.14.106:3000
```

**当前状态：**
- ✅ Web 前端管理平台完全实现
- ✅ Web 后端 API 服务完全实现
- ✅ Windows 客户端核心引擎完全实现
- ⏭️ 下一步：创建 Windows 客户端 GUI（Avalonia UI）

---

### 2026-05-02 - Windows 客户端 GUI 完成

**任务：** 完成 Windows 客户端 GUI（Avalonia UI）

**实现内容：**

1. **项目初始化**
   - 创建 Avalonia UI 项目（.NET 8.0）
   - 配置项目依赖（Avalonia 11.0.10、ReactiveUI）
   - 创建项目目录结构（Views、ViewModels、Models）

2. **P/Invoke 接口（NativeMethods.cs）**
   - 定义回调函数委托（LogCallback、ConnectionCallback）
   - 定义数据结构（ProxyConfig、PolicyRule、ConnectionStats）
   - 导入核心引擎 API（初始化、启动、停止、配置、策略、同步、统计）
   - 使用 DllImport 调用 tunnelproxy.dll

3. **数据模型（Models/）**
   - ConnectionLogModel - 连接日志模型
   - PolicyRuleModel - 策略规则模型（支持与本地结构互转）
   - ProxyConfigModel - 代理配置模型（支持与本地结构互转）

4. **主视图模型（ViewModels/MainViewModel.cs）**
   - 实现 INotifyPropertyChanged 接口
   - 属性绑定（运行状态、设备ID、同步服务器、统计信息）
   - 命令实现（启动、停止、同步、添加策略、删除策略、清空日志）
   - 回调处理（日志回调、连接回调）
   - 定时更新统计信息（1秒间隔）
   - 策略集合管理
   - 连接日志管理（限制1000条）

5. **主窗口界面（Views/MainWindow.axaml）**
   - 同步配置区域（设备ID、同步服务器URL）
   - 代理配置区域（类型、地址、端口、用户名、密码）
   - 控制按钮（启动、停止、同步策略、添加策略、清空日志）
   - 策略列表（DataGrid显示应用名称、动作、启用状态）
   - 连接日志（DataGrid显示时间、协议、进程、目标、动作）
   - 统计信息栏（总连接数、代理数、直连数、阻止数）
   - 状态消息显示

6. **应用程序入口**
   - Program.cs - 程序入口点
   - App.axaml - 应用程序资源
   - App.axaml.cs - 应用程序类

**技术特性：**
- MVVM 架构模式
- 数据绑定和命令绑定
- P/Invoke 调用 C 核心引擎
- 响应式 UI 更新
- 线程安全的回调处理
- 实时统计信息显示
- 连接日志实时更新

**项目结构：**
```
gui/
├── Views/
│   ├── MainWindow.axaml          # 主窗口 XAML
│   └── MainWindow.axaml.cs       # 主窗口代码
├── ViewModels/
│   └── MainViewModel.cs          # 主视图模型
├── Models/
│   ├── ConnectionLogModel.cs    # 连接日志模型
│   ├── PolicyRuleModel.cs       # 策略规则模型
│   └── ProxyConfigModel.cs      # 代理配置模型
├── Assets/                       # 资源文件
├── NativeMethods.cs              # P/Invoke 接口
├── App.axaml                     # 应用程序 XAML
├── App.axaml.cs                  # 应用程序代码
├── Program.cs                    # 程序入口
└── TunnelProxyGUI.csproj         # 项目文件
```

**功能特性：**
- 完整的代理配置（SOCKS5/HTTP）
- 策略管理（添加、删除、查看）
- 策略同步（从服务器拉取）
- 实时连接日志显示
- 统计信息实时更新
- 启动/停止控制
- 设备ID和同步服务器配置

**编译要求：**
- .NET 8.0 SDK
- Windows 10/11
- Visual Studio 2022 或 Rider

**编译命令：**
```bash
cd windows-client/gui
dotnet restore
dotnet build
dotnet run
```

**依赖关系：**
- 需要编译 core 目录下的 C 核心引擎生成 tunnelproxy.dll
- GUI 通过 P/Invoke 调用 tunnelproxy.dll

**当前状态：**
- ✅ Web 后端 API 服务完全实现
- ✅ Web 前端管理平台完全实现
- ✅ Windows 客户端核心引擎完全实现
- ✅ Windows 客户端 GUI 完全实现
- 🎉 TunnelProxy 项目核心功能全部完成！

**下一步工作：**
- 编译 C 核心引擎生成 DLL
- 编译 C# GUI 应用程序
- 集成测试（GUI + 核心引擎）
- 启动 Web 后端服务
- 启动 Web 前端服务
- 端到端测试

---

### 2026-05-10 - Web 服务部署完成

**任务：** 部署 Web 后端和前端服务，添加客户端下载页面

**实现内容：**

1. **Go 环境配置**
   - 安装 Go 1.21.5 (ARM64)
   - 配置环境变量
   - 下载项目依赖

2. **后端服务部署**
   - 编译 Go 后端程序（tunnelproxy-server）
   - 启动后端服务（端口 8080）
   - 初始化 SQLite 数据库
   - 配置 CORS 跨域支持

3. **前端服务部署**
   - 安装 npm 依赖
   - 启动 Vite 开发服务器（端口 3001）
   - 配置 API 代理

4. **客户端下载页面**
   - 创建 ClientDownload.jsx 页面组件
   - 添加 Windows 和 macOS 客户端下载入口
   - Windows 客户端显示为可下载状态
   - macOS 客户端显示为开发中状态
   - 添加详细的功能特性和系统要求说明
   - 添加使用说明和安全提示

5. **下载目录结构**
   - 创建 `/web-platform/frontend/public/Client/download/` 目录
   - 添加 README.md 说明文件
   - 预留 Windows 客户端下载位置

6. **编译配置文件**
   - 创建 CMakeLists.txt（核心引擎构建配置）
   - 创建 BUILD.md（核心引擎编译说明）
   - 创建 COMPILE.md（完整编译指南）
   - 创建 DEPLOYMENT.md（部署文档）

**服务地址：**
- Web 后端：http://144.24.14.106:8080
- Web 前端：http://144.24.14.106:3001
- API 文档：http://144.24.14.106:8080/api

**功能页面：**
- 设备管理：http://144.24.14.106:3001/devices
- 策略管理：http://144.24.14.106:3001/policies
- 代理配置：http://144.24.14.106:3001/proxy-config
- 客户端下载：http://144.24.14.106:3001/download

**技术要点：**
- 使用 ARM64 架构的 Go 编译器
- 前端端口自动切换（3000 被占用，使用 3001）
- 后端使用 Gin 框架，支持 RESTful API
- 前端使用 React + Vite + Ant Design
- 客户端下载页面支持多平台展示

**当前状态：**
- ✅ Web 后端服务运行中
- ✅ Web 前端服务运行中
- ✅ 客户端下载页面已创建
- ✅ 编译配置文件已创建
- ⏳ Windows 客户端需要在 Windows 环境下编译
- ⏳ macOS 客户端待开发
- ⏳ Linux 客户端待开发

**Windows 客户端编译说明：**

由于 Windows 客户端使用了 Windows 特定技术（WinDivert、.NET），必须在 Windows 环境下编译：

1. 安装编译环境：
   - Visual Studio 2019+
   - .NET 8.0 SDK
   - CMake 3.15+
   - WinDivert 2.2-A SDK

2. 编译核心引擎：
   ```cmd
   cd windows-client\core
   mkdir build && cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   cmake --build . --config Release
   ```

3. 编译 GUI 应用：
   ```cmd
   cd windows-client\gui
   dotnet publish -c Release -r win-x64 --self-contained -o publish
   ```

4. 打包并上传：
   ```bash
   scp TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/
   ```

详细说明请参考：
- `/home/ubuntu/cc/tunnelproxy/windows-client/COMPILE.md`
- `/home/ubuntu/cc/tunnelproxy/DEPLOYMENT.md`
