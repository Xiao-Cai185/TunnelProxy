# TunnelProxy Windows 客户端 - 占位包

此文件为占位符。实际的 Windows 客户端需要在 Windows 环境下编译。

## 如何编译

请在 Windows 电脑上按照以下步骤操作：

### 1. 克隆或下载项目代码

```bash
git clone <repository-url>
cd tunnelproxy/windows-client
```

### 2. 下载 WinDivert SDK

从 GitHub 下载 WinDivert 2.2-A：
```
https://github.com/basil00/Divert/releases/download/v2.2.2-A/WinDivert-2.2.2-A.zip
```

解压到 `core/third_party/WinDivert/` 目录。

### 3. 运行自动编译脚本

```cmd
build.bat
```

脚本会自动：
- 检查编译环境
- 编译核心引擎 DLL
- 编译 GUI 应用程序
- 打包发布文件
- 生成 ZIP 包

### 4. 上传到服务器

编译完成后，使用以下命令上传：

```cmd
scp release\TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/
```

## 系统要求

- Windows 10/11 (x64)
- Visual Studio 2019+
- CMake 3.15+
- .NET 8.0 SDK

## 详细文档

- `BUILD_QUICK.md` - 快速编译指南
- `COMPILE.md` - 完整编译说明
- `BUILD.md` - 核心引擎构建文档

## 下载地址

编译并上传后，可从以下地址下载：
```
http://144.24.14.106:3001/download
```
