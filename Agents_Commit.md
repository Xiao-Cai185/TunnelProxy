# TunnelProxy 项目提交记录

本文件记录项目开发过程中的所有 Git 提交信息和细节。

---

## 2026-05-11

### Commit: e4252c4
**提交信息**: 彻底移除 PowerShell here-string 语法，改用字符串拼接

**修改内容**:
- 完全移除 PowerShell here-string 语法（`@"..."@`）
- 改用 PowerShell 的字符串拼接和换行符 `` `n `` 方式创建 README 文件
- 转义双引号字符（`` `" ``）避免语法冲突
- 将 40 行的 here-string 改为 35 行的字符串拼接代码

**修改原因**:
- PowerShell here-string 语法在 YAML 的 `run: |` 多行块中持续导致解析错误
- 即使使用变量赋值仍然无法解决 YAML 解析器的问题
- 字符串拼接方式更安全，不会触发 YAML 特殊字符解析

**影响范围**:
- GitHub Actions 自动编译流程中的 README 文件生成步骤

**技术细节**:
- 使用 `$readme += "内容`n"` 逐行拼接
- 使用 PowerShell 转义字符：`` `n ``（换行）、`` `" ``（双引号）
- 最终通过 `Out-File` 输出到文件

---

### Commit: 5dce1a4
**提交信息**: 修复 GitHub Actions 工作流 PowerShell here-string 语法错误

**修改内容**:
- 修复 `.github/workflows/build-windows.yml` 第 117 行的 PowerShell here-string 语法问题
- 将 here-string 先赋值给变量 `$readmeContent`，再输出到文件
- 避免 YAML 解析器将行首的 `@"` 识别为特殊字符

**修改原因**:
- GitHub Actions 工作流文件 YAML 语法检查失败
- PowerShell 的 here-string 语法在 YAML 中需要特殊处理
- 直接在 YAML 中使用 `@"` 会导致解析错误

**影响范围**:
- GitHub Actions 自动编译流程中的 README 文件生成步骤

**技术细节**:
- 原代码：`@" ... "@ | Out-File`
- 修复后：`$readmeContent = @" ... "@` 然后 `$readmeContent | Out-File`

---

### Commit: 0cda222
**提交信息**: 修复 GitHub Actions 工作流 YAML 语法错误

**修改内容**:
- 修复 `.github/workflows/build-windows.yml` 第 84 行的 PowerShell 命令拼写错误
- 将 `rite-Error` 修正为 `Write-Error`

**修改原因**:
- GitHub Actions 工作流文件 YAML 语法检查失败
- PowerShell 命令拼写错误导致工作流无法正常运行

**影响范围**:
- GitHub Actions 自动编译流程

---

### Commit: 643b716
**提交信息**: 添加 GitHub Actions 自动编译 Windows 客户端

**修改内容**:
- 创建 `.github/workflows/build-windows.yml` - Windows 客户端自动编译工作流
  - 自动下载 WinDivert SDK
  - 使用 CMake 编译 C 核心引擎
  - 使用 .NET 8.0 编译 GUI 应用
  - 打包为 ZIP 文件
  - 上传为 Artifact（保留 90 天）
  - 支持 tag 推送时自动创建 Release

- 创建 `.github/workflows/deploy.yml` - 自动部署工作流
  - 在编译成功后自动触发
  - 通过 SSH 部署到服务器
  - 需要配置 GitHub Secrets（DEPLOY_SSH_KEY, DEPLOY_HOST, DEPLOY_USER）

- 创建 `.github/workflows/README.md` - 工作流说明文档
  - 详细说明工作流的触发条件和构建步骤
  - 提供手动触发和配置部署的指南
  - 包含故障排除说明

**修改原因**:
- 实现 Windows 客户端的自动化编译和部署
- 解决 Linux 环境无法交叉编译 Windows 内核驱动的问题
- 提高开发效率，确保每次代码更新后都能自动生成可用的安装包

**影响范围**:
- CI/CD 流程
- Windows 客户端发布流程
- 前端下载页面的文件更新机制

**技术细节**:
- 使用 GitHub Actions 的 windows-latest runner
- 编译环境：Windows Server 2022 + Visual Studio 2022 + .NET 8.0
- 产物路径：`windows-client/release/TunnelProxy-Windows-x64.zip`
- 部署目标：`/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/`

---

## 2026-05-10

### Commit: ad04ff4
**提交信息**: Merge branch 'main' of github.com:Xiao-Cai185/TunnelProxy

**修改内容**:
- 合并远程仓库的 LICENSE 文件到本地

**修改原因**:
- 首次推送时远程仓库已包含 LICENSE 文件
- 使用 `--allow-unrelated-histories` 合并不相关的历史记录

---

### Commit: 6abc80a
**提交信息**: Initial commit

**修改内容**:
- 远程仓库初始化提交（GitHub 自动创建）

---

### Commit: 6cbd563
**提交信息**: Initial commit: TunnelProxy 集中式管理的跨平台进程级代理系统

**修改内容**:
- 完整的项目初始化
- Web 管理平台（前端 + 后端）
- Windows 客户端（C 核心引擎 + .NET GUI）
- 项目文档和配置文件

**项目结构**:
```
tunnelproxy/
├── web-platform/          # Web 管理平台
│   ├── backend/          # Go + Gin + SQLite
│   └── frontend/         # React 19 + Vite + Ant Design 6
├── windows-client/        # Windows 客户端
│   ├── core/             # C + WinDivert 2.2-A
│   └── gui/              # Avalonia UI + .NET 8.0
├── Agents_Edit.md        # 项目开发记录
├── Agents_HowToDo.md     # 开发要求记录
└── README.md             # 项目说明
```

**技术栈**:
- 后端：Go 1.21 + Gin + SQLite + GORM
- 前端：React 19 + Vite + Ant Design 6
- Windows 核心：C + WinDivert 2.2-A（内核级数据包拦截）
- Windows GUI：Avalonia UI + .NET 8.0 + MVVM

**核心功能**:
- 设备管理（注册、认证、状态监控）
- 策略管理（进程规则、域名规则）
- 代理配置（SOCKS5/HTTP 代理设置）
- 客户端下载（Windows/MacOS）
- 实时连接日志
- 策略自动同步

---

## 提交统计

- 总提交数：5
- 最近更新：2026-05-11
- 主要贡献者：Claude Sonnet 4.5 (AI Assistant)
