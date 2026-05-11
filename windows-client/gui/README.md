# TunnelProxy Windows 客户端 GUI

基于 Avalonia UI 的跨平台桌面应用程序。

## 功能特性

- ✅ 代理配置（SOCKS5/HTTP）
- ✅ 策略管理（添加、删除、查看）
- ✅ 策略同步（从服务器拉取）
- ✅ 实时连接日志显示
- ✅ 统计信息实时更新
- ✅ 启动/停止控制
- ✅ 设备ID和同步服务器配置

## 技术栈

- .NET 8.0
- Avalonia UI 11.0.10
- MVVM 架构模式
- P/Invoke 调用 C 核心引擎

## 项目结构

```
gui/
├── Views/              # 视图（XAML）
├── ViewModels/         # 视图模型
├── Models/             # 数据模型
├── Assets/             # 资源文件
├── NativeMethods.cs    # P/Invoke 接口
├── App.axaml           # 应用程序
├── Program.cs          # 程序入口
└── TunnelProxyGUI.csproj
```

## 编译要求

- .NET 8.0 SDK
- Windows 10/11
- Visual Studio 2022 或 JetBrains Rider

## 编译步骤

### 1. 编译 C 核心引擎

首先需要编译 `core` 目录下的 C 核心引擎生成 `tunnelproxy.dll`：

```bash
cd ../core
# 使用 Visual Studio 或 CMake 编译
# 生成 tunnelproxy.dll
```

### 2. 编译 GUI 应用程序

```bash
cd gui
dotnet restore
dotnet build
```

### 3. 运行应用程序

```bash
dotnet run
```

或者直接运行编译后的可执行文件：

```bash
cd bin/Debug/net8.0-windows
./TunnelProxyGUI.exe
```

## 使用说明

### 1. 配置同步服务器

- **设备 ID**：输入唯一的设备标识符
- **同步服务器**：输入 Web 管理平台的地址（例如：http://144.24.14.106:8080）

### 2. 配置代理服务器

- **类型**：选择 SOCKS5 或 HTTP
- **地址**：代理服务器地址（例如：127.0.0.1）
- **端口**：代理服务器端口（例如：1080）
- **用户名/密码**：如果代理需要认证，填写相应信息

### 3. 管理策略

- **添加策略**：点击"添加策略"按钮添加新的应用规则
- **删除策略**：在策略列表中选择要删除的策略
- **同步策略**：点击"同步策略"按钮从服务器拉取最新策略

### 4. 启动代理

- 点击"启动"按钮启动 TunnelProxy
- 启动后，应用程序会根据策略规则拦截和转发流量
- 连接日志会实时显示在右侧面板
- 统计信息会实时更新在底部状态栏

### 5. 停止代理

- 点击"停止"按钮停止 TunnelProxy
- 所有流量拦截将停止

## 依赖关系

GUI 应用程序依赖于 C 核心引擎（tunnelproxy.dll），需要确保：

1. `tunnelproxy.dll` 已编译
2. `tunnelproxy.dll` 与 GUI 可执行文件在同一目录
3. 或者 `tunnelproxy.dll` 在系统 PATH 中

## 注意事项

- 需要管理员权限运行（WinDivert 需要管理员权限）
- 确保 WinDivert 驱动已正确安装
- 首次运行可能需要允许防火墙访问

## 故障排除

### 1. 启动失败

- 检查是否以管理员权限运行
- 检查 `tunnelproxy.dll` 是否存在
- 检查 WinDivert 驱动是否正确安装

### 2. 策略同步失败

- 检查同步服务器地址是否正确
- 检查网络连接是否正常
- 检查 Web 后端服务是否运行

### 3. 连接日志不显示

- 检查是否已启动代理
- 检查策略规则是否正确配置
- 检查目标应用程序是否正在运行

## 开发说明

### MVVM 架构

- **Model**：数据模型（Models/）
- **View**：用户界面（Views/）
- **ViewModel**：视图逻辑（ViewModels/）

### P/Invoke 调用

通过 `NativeMethods.cs` 调用 C 核心引擎的 API：

```csharp
// 初始化
NativeMethods.TunnelProxy_Init(logCallback, connectionCallback);

// 启动
NativeMethods.TunnelProxy_Start();

// 停止
NativeMethods.TunnelProxy_Stop();
```

### 数据绑定

使用 Avalonia 的数据绑定机制：

```xml
<TextBox Text="{Binding DeviceId}" />
<Button Command="{Binding StartCommand}" />
```

## 许可证

与 TunnelProxy 项目相同。
