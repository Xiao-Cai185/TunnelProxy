# TunnelProxy Windows 客户端编译指南

## 快速开始

### 方法一：使用自动编译脚本（推荐）

1. **下载 WinDivert SDK**
   ```
   https://github.com/basil00/Divert/releases/download/v2.2.2-A/WinDivert-2.2.2-A.zip
   ```
   解压到 `core/third_party/WinDivert/`

2. **运行编译脚本**
   ```cmd
   cd windows-client
   build.bat
   ```

3. **上传到服务器**
   
   Windows 下使用 WinSCP 或命令行：
   ```cmd
   scp release\TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/
   ```

   或者在 Linux/Mac 下：
   ```bash
   ./upload.sh
   ```

### 方法二：手动编译

详见 `COMPILE.md` 文档。

## 编译环境要求

- Windows 10/11
- Visual Studio 2019 或更高版本（包含 C/C++ 工具）
- CMake 3.15+
- .NET 8.0 SDK
- WinDivert 2.2-A SDK

## 编译输出

编译成功后会生成：
- `release/TunnelProxy-Windows-x64/` - 完整的客户端文件夹
- `release/TunnelProxy-Windows-x64.zip` - 打包好的 ZIP 文件

## 文件结构

```
TunnelProxy-Windows-x64/
├── TunnelProxyGUI.exe          # GUI 主程序
├── TunnelProxyGUI.dll          # GUI 库
├── tunnelproxy.dll             # 核心引擎
├── WinDivert.dll               # WinDivert 运行时
├── WinDivert64.sys             # WinDivert 驱动
├── Avalonia.*.dll              # Avalonia UI 库
└── README.txt                  # 使用说明
```

## 常见问题

### 1. 找不到 Visual Studio
确保安装了 Visual Studio 2019 或更高版本，并包含"使用 C++ 的桌面开发"工作负载。

### 2. CMake 配置失败
检查是否正确安装了 CMake，并且在 PATH 环境变量中。

### 3. WinDivert SDK 未找到
确保 WinDivert SDK 已解压到正确的位置：
```
core/third_party/WinDivert/
├── include/
│   └── windivert.h
├── x64/
│   ├── WinDivert.dll
│   ├── WinDivert.lib
│   └── WinDivert64.sys
└── x86/
    └── ...
```

### 4. .NET SDK 版本不对
确保安装的是 .NET 8.0 SDK，而不是 Runtime。

## 测试编译结果

编译完成后，可以在本地测试：

1. 进入 `release/TunnelProxy-Windows-x64/`
2. 右键点击 `TunnelProxyGUI.exe`，选择"以管理员身份运行"
3. 输入管理平台地址：`http://144.24.14.106:8080`
4. 输入设备 ID（在管理平台创建）
5. 点击"同步策略"测试连接

## 上传到服务器

### Windows 下使用 SCP
```cmd
scp release\TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/
```

### 使用 WinSCP（图形界面）
1. 打开 WinSCP
2. 连接到 `144.24.14.106`
3. 导航到 `/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/`
4. 上传 `TunnelProxy-Windows-x64.zip`

### Linux/Mac 下使用脚本
```bash
./upload.sh
```

## 验证下载

上传成功后，访问：
```
http://144.24.14.106:3001/download
```

点击"下载客户端"按钮，应该可以下载到刚上传的 ZIP 文件。

## 技术支持

如有问题，请查看：
- `COMPILE.md` - 详细编译说明
- `BUILD.md` - 核心引擎构建说明
- `../DEPLOYMENT.md` - 项目部署文档
