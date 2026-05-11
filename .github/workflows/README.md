# GitHub Actions 工作流说明

本项目使用 GitHub Actions 自动编译和部署 Windows 客户端。

## 工作流列表

### 1. Build Windows Client (`build-windows.yml`)

**触发条件：**
- 推送到 `main` 分支且修改了 `windows-client/` 目录
- 创建 Pull Request
- 手动触发（Actions 页面）

**构建步骤：**
1. 下载 WinDivert SDK
2. 使用 CMake 编译 C 核心引擎
3. 使用 .NET 8.0 编译 GUI 应用
4. 打包所有文件为 ZIP
5. 上传为 Artifact（保留 90 天）
6. 如果是 tag 推送，自动创建 Release

**产物：**
- `TunnelProxy-Windows-x64.zip` - 完整的 Windows 客户端安装包

### 2. Deploy to Server (`deploy.yml`)

**触发条件：**
- `Build Windows Client` 工作流成功完成
- 手动触发

**部署步骤：**
1. 下载编译好的 Artifact
2. 通过 SSH 上传到服务器
3. 放置到前端下载目录

**需要配置的 Secrets：**
- `DEPLOY_SSH_KEY` - SSH 私钥
- `DEPLOY_HOST` - 服务器地址（144.24.14.106）
- `DEPLOY_USER` - SSH 用户名（ubuntu）

## 使用方法

### 自动构建

每次推送代码到 `main` 分支时，如果修改了 `windows-client/` 目录，会自动触发构建。

### 手动触发

1. 访问 GitHub 仓库的 Actions 页面
2. 选择 "Build Windows Client" 工作流
3. 点击 "Run workflow" 按钮
4. 选择分支并运行

### 下载构建产物

1. 访问 Actions 页面
2. 点击最近的工作流运行
3. 在 "Artifacts" 部分下载 `TunnelProxy-Windows-x64.zip`

### 创建 Release

推送一个 tag 即可自动创建 Release：

```bash
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

## 配置自动部署

如果需要自动部署到服务器，需要配置以下 Secrets：

1. 访问仓库的 Settings → Secrets and variables → Actions
2. 添加以下 Secrets：

### DEPLOY_SSH_KEY

生成 SSH 密钥对：
```bash
ssh-keygen -t ed25519 -C "github-actions" -f deploy_key -N ""
```

将 `deploy_key` 的内容添加为 Secret，将 `deploy_key.pub` 添加到服务器的 `~/.ssh/authorized_keys`。

### DEPLOY_HOST

```
144.24.14.106
```

### DEPLOY_USER

```
ubuntu
```

## 构建环境

- **操作系统**: Windows Server 2022
- **CMake**: 最新版本
- **.NET SDK**: 8.0.x
- **MSBuild**: Visual Studio 2022
- **WinDivert**: 2.2.2-A

## 构建时间

通常需要 5-10 分钟完成整个构建流程。

## 故障排除

### 构建失败

1. 检查 Actions 日志中的错误信息
2. 确认 `windows-client/` 目录下的代码没有语法错误
3. 确认 CMakeLists.txt 配置正确

### 部署失败

1. 检查 SSH 密钥是否正确配置
2. 确认服务器地址和用户名正确
3. 确认服务器上的目标目录存在且有写权限

## 本地测试

在推送到 GitHub 之前，可以在本地测试构建：

```bash
cd windows-client
build.bat
```

## 相关文档

- [GitHub Actions 文档](https://docs.github.com/en/actions)
- [Windows 客户端编译指南](../windows-client/COMPILE.md)
- [项目部署文档](../DEPLOYMENT.md)
