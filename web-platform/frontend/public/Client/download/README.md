# TunnelProxy Windows 客户端占位文件

此文件为占位符，实际的 Windows 客户端需要在 Windows 环境下编译。

## 编译步骤

请在 Windows 环境下按照以下步骤编译：

### 1. 编译核心引擎 DLL

```cmd
cd windows-client\core
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

### 2. 编译 GUI 应用程序

```cmd
cd windows-client\gui
dotnet publish -c Release -r win-x64 --self-contained
```

### 3. 打包发布

将以下文件打包为 `TunnelProxy-Windows-x64.zip`：

```
TunnelProxy-Windows-x64/
├── TunnelProxyGUI.exe
├── tunnelproxy.dll
├── WinDivert.dll
├── WinDivert64.sys
└── README.txt
```

### 4. 上传到服务器

将打包好的 zip 文件上传到：
```
/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/TunnelProxy-Windows-x64.zip
```

## 系统要求

- Windows 10/11 (x64)
- .NET 8.0 Runtime
- 管理员权限

## 使用说明

1. 解压 zip 文件
2. 以管理员身份运行 TunnelProxyGUI.exe
3. 输入管理平台地址和设备 ID
4. 同步策略并启动服务
