# TunnelProxy 项目部署完成

## 🎉 服务状态

### ✅ Web 后端服务
- **状态：** 运行中
- **地址：** http://144.24.14.106:8080
- **API 端点：** http://144.24.14.106:8080/api
- **数据库：** SQLite (tunnelproxy.db)

### ✅ Web 前端服务
- **状态：** 运行中
- **地址：** http://144.24.14.106:3001
- **框架：** React + Vite + Ant Design

## 📱 功能页面

1. **设备管理** - http://144.24.14.106:3001/devices
   - 添加/编辑/删除设备
   - 查看设备在线状态
   - 支持 Windows/macOS/Linux 三种平台

2. **策略管理** - http://144.24.14.106:3001/policies
   - 配置应用程序代理规则
   - 支持通配符匹配
   - 动作：代理/直连/阻止

3. **代理配置** - http://144.24.14.106:3001/proxy-config
   - 配置上游代理服务器
   - 支持 SOCKS5 和 HTTP 代理
   - 支持用户名密码认证

4. **客户端下载** - http://144.24.14.106:3001/download
   - Windows 客户端下载（待编译）
   - macOS 客户端（开发中）

## 🔧 Windows 客户端编译

由于 Windows 客户端需要在 Windows 环境下编译，请参考以下文档：

- **编译说明：** `/home/ubuntu/cc/tunnelproxy/windows-client/COMPILE.md`
- **核心引擎构建：** `/home/ubuntu/cc/tunnelproxy/windows-client/core/BUILD.md`
- **CMake 配置：** `/home/ubuntu/cc/tunnelproxy/windows-client/core/CMakeLists.txt`

### 编译步骤概要

1. 在 Windows 环境下安装：
   - Visual Studio 2019+
   - .NET 8.0 SDK
   - CMake 3.15+
   - WinDivert 2.2-A SDK

2. 编译核心引擎 DLL：
   ```cmd
   cd windows-client\core
   mkdir build && cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   cmake --build . --config Release
   ```

3. 编译 GUI 应用程序：
   ```cmd
   cd windows-client\gui
   dotnet publish -c Release -r win-x64 --self-contained -o publish
   ```

4. 打包并上传到服务器：
   ```bash
   scp TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/
   ```

## 📂 项目结构

```
tunnelproxy/
├── web-platform/
│   ├── backend/                    # Go 后端服务
│   │   ├── tunnelproxy-server      # 编译好的可执行文件
│   │   └── tunnelproxy.db          # SQLite 数据库
│   └── frontend/                   # React 前端
│       └── public/Client/download/ # 客户端下载目录
├── windows-client/
│   ├── core/                       # C 核心引擎
│   │   ├── CMakeLists.txt          # CMake 构建配置
│   │   └── BUILD.md                # 构建说明
│   ├── gui/                        # C# GUI 应用
│   └── COMPILE.md                  # 完整编译说明
└── docs/                           # 项目文档
```

## 🚀 使用流程

### 1. 管理员操作
1. 访问 http://144.24.14.106:3001
2. 在"设备管理"页面添加设备，记录设备 ID
3. 在"策略管理"页面配置应用程序规则
4. 在"代理配置"页面设置上游代理服务器

### 2. 客户端用户操作
1. 从"客户端下载"页面下载对应平台的客户端
2. 以管理员权限运行客户端
3. 输入设备 ID 和服务器地址（http://144.24.14.106:8080）
4. 点击"同步策略"获取配置
5. 点击"启动"开始代理服务

## 📊 API 接口

### 设备管理
- `POST /api/devices` - 创建设备
- `GET /api/devices` - 获取设备列表
- `GET /api/devices/:id` - 获取设备详情
- `PUT /api/devices/:id` - 更新设备
- `DELETE /api/devices/:id` - 删除设备

### 策略同步
- `GET /api/sync/:device_id` - 客户端同步策略
- `POST /api/sync/heartbeat` - 客户端心跳

## 🔒 安全提示

- 后端服务已启用 CORS，允许跨域访问
- 客户端需要管理员权限运行
- WinDivert 驱动需要数字签名（测试环境可使用测试模式）
- 建议在生产环境中启用 HTTPS 和身份认证

## 📝 下一步工作

1. ✅ Web 后端服务 - 已完成
2. ✅ Web 前端服务 - 已完成
3. ✅ 客户端下载页面 - 已完成
4. ⏳ Windows 客户端编译 - 需要 Windows 环境
5. ⏳ macOS 客户端开发 - 待开发
6. ⏳ Linux 客户端开发 - 待开发

## 🛠️ 维护命令

### 停止服务
```bash
# 停止后端
pkill -f tunnelproxy-server

# 停止前端
pkill -f vite
```

### 重启服务
```bash
# 重启后端
cd /home/ubuntu/cc/tunnelproxy/web-platform/backend
./tunnelproxy-server &

# 重启前端
cd /home/ubuntu/cc/tunnelproxy/web-platform/frontend
npm run dev &
```

### 查看日志
```bash
# 后端日志
tail -f /home/ubuntu/cc/tunnelproxy/web-platform/backend/tunnelproxy.log

# 前端日志（控制台输出）
```

## 📞 技术支持

如有问题，请查看项目文档或联系开发团队。
