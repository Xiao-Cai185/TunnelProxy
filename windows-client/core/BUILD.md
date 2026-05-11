# TunnelProxy 核心引擎编译指南

## 环境要求

### Windows 编译环境
- Visual Studio 2019 或更高版本（包含 C/C++ 工具）
- CMake 3.15 或更高版本
- WinDivert 2.2-A SDK

## 准备工作

### 1. 下载 WinDivert SDK

从 GitHub 下载 WinDivert 2.2-A：
```
https://github.com/basil00/Divert/releases/download/v2.2.2-A/WinDivert-2.2.2-A.zip
```

解压到 `third_party/WinDivert` 目录：
```
windows-client/core/
├── third_party/
│   └── WinDivert/
│       ├── include/
│       │   └── windivert.h
│       ├── x64/
│       │   ├── WinDivert.dll
│       │   ├── WinDivert.lib
│       │   └── WinDivert64.sys
│       └── x86/
│           ├── WinDivert.dll
│           ├── WinDivert.lib
│           └── WinDivert32.sys
```

## 编译步骤

### 方法一：使用 CMake（推荐）

1. 打开 "x64 Native Tools Command Prompt for VS 2019"

2. 创建构建目录：
```cmd
cd windows-client\core
mkdir build
cd build
```

3. 生成项目文件：
```cmd
cmake .. -G "Visual Studio 16 2019" -A x64
```

4. 编译：
```cmd
cmake --build . --config Release
```

5. 输出文件位于：
```
build/lib/Release/tunnelproxy.dll
build/lib/Release/WinDivert.dll
build/lib/Release/WinDivert64.sys
```

### 方法二：使用 Visual Studio

1. 使用 CMake 生成 Visual Studio 解决方案：
```cmd
cmake .. -G "Visual Studio 16 2019" -A x64
```

2. 打开生成的 `TunnelProxy.sln`

3. 在 Visual Studio 中选择 Release 配置

4. 右键点击 `tunnelproxy` 项目，选择"生成"

## 编译 32 位版本

如果需要编译 32 位版本，将 `-A x64` 改为 `-A Win32`：
```cmd
cmake .. -G "Visual Studio 16 2019" -A Win32
```

## 验证编译结果

编译成功后，应该生成以下文件：
- `tunnelproxy.dll` - 核心引擎动态链接库
- `WinDivert.dll` - WinDivert 运行时库
- `WinDivert64.sys` - WinDivert 驱动程序（64位）

## 常见问题

### 1. 找不到 WinDivert SDK
确保 WinDivert SDK 已正确解压到 `third_party/WinDivert` 目录。

### 2. 链接错误
确保使用了正确的 Visual Studio 命令提示符（x64 或 x86）。

### 3. 权限错误
WinDivert 需要管理员权限才能加载驱动程序。

## 下一步

编译完成后，继续编译 GUI 应用程序：
```cmd
cd ..\gui
dotnet build -c Release
```
