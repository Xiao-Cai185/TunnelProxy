# TunnelProxy Windows 客户端编译说明

## 编译环境要求

由于 TunnelProxy Windows 客户端使用了 Windows 特定的技术（WinDivert、.NET），必须在 Windows 环境下编译。

### 所需软件

1. **Visual Studio 2019 或更高版本**
   - 包含 C/C++ 桌面开发工具
   - 下载地址：https://visualstudio.microsoft.com/

2. **.NET 8.0 SDK**
   - 下载地址：https://dotnet.microsoft.com/download/dotnet/8.0

3. **CMake 3.15+**
   - 下载地址：https://cmake.org/download/

4. **WinDivert 2.2-A SDK**
   - 下载地址：https://github.com/basil00/Divert/releases/download/v2.2.2-A/WinDivert-2.2.2-A.zip

## 编译步骤

### 第一步：准备 WinDivert SDK

1. 下载 WinDivert-2.2.2-A.zip
2. 解压到 `windows-client/core/third_party/WinDivert/`
3. 确保目录结构如下：
```
windows-client/core/third_party/WinDivert/
├── include/
│   └── windivert.h
├── x64/
│   ├── WinDivert.dll
│   ├── WinDivert.lib
│   └── WinDivert64.sys
└── x86/
    ├── WinDivert.dll
    ├── WinDivert.lib
    └── WinDivert32.sys
```

### 第二步：编译核心引擎 DLL

打开 "x64 Native Tools Command Prompt for VS 2019"：

```cmd
cd windows-client\core
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

编译成功后，在 `build/lib/Release/` 目录下会生成：
- `tunnelproxy.dll`
- `WinDivert.dll`
- `WinDivert64.sys`

### 第三步：编译 GUI 应用程序

```cmd
cd windows-client\gui
dotnet restore
dotnet publish -c Release -r win-x64 --self-contained -o publish
```

编译成功后，在 `publish/` 目录下会生成完整的应用程序。

### 第四步：复制核心引擎文件

将核心引擎文件复制到 GUI 发布目录：

```cmd
copy ..\core\build\lib\Release\tunnelproxy.dll publish\
copy ..\core\build\lib\Release\WinDivert.dll publish\
copy ..\core\build\lib\Release\WinDivert64.sys publish\
```

### 第五步：打包发布

创建发布包目录结构：

```
TunnelProxy-Windows-x64/
├── TunnelProxyGUI.exe
├── TunnelProxyGUI.dll
├── tunnelproxy.dll
├── WinDivert.dll
├── WinDivert64.sys
├── Avalonia.*.dll (所有 Avalonia 相关 DLL)
└── README.txt
```

将整个目录打包为 `TunnelProxy-Windows-x64.zip`

### 第六步：上传到服务器

使用 SCP 或其他工具将 zip 文件上传到服务器：

```bash
scp TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/
```

## 使用说明

### 系统要求
- Windows 10/11 (x64)
- 管理员权限

### 安装步骤
1. 解压 `TunnelProxy-Windows-x64.zip`
2. 右键点击 `TunnelProxyGUI.exe`，选择"以管理员身份运行"
3. 首次启动时输入：
   - 设备 ID（在管理平台的设备管理页面创建）
   - 同步服务器地址（例如：http://144.24.14.106:8080）
4. 配置代理服务器信息
5. 点击"同步策略"获取最新配置
6. 点击"启动"开始代理服务

### 注意事项
- 必须以管理员权限运行
- 首次运行时 Windows 可能会提示安装驱动程序
- 防火墙可能会提示允许网络访问
- 确保代理服务器地址可访问

## 常见问题

### 1. 编译失败：找不到 WinDivert
确保 WinDivert SDK 已正确解压到 `third_party/WinDivert` 目录。

### 2. 运行时错误：无法加载 DLL
确保所有 DLL 文件都在同一目录下。

### 3. 驱动加载失败
以管理员权限运行，并确保 Windows 驱动签名验证已关闭（测试模式）。

### 4. 连接服务器失败
检查服务器地址是否正确，防火墙是否允许访问。

## 技术支持

如有问题，请查看项目文档或提交 Issue。
