@echo off
REM TunnelProxy Windows 客户端自动编译脚本
REM 请在 Windows 环境下运行此脚本

echo ========================================
echo TunnelProxy Windows 客户端编译脚本
echo ========================================
echo.

REM 检查是否在正确的目录
if not exist "core" (
    echo 错误：请在 windows-client 目录下运行此脚本
    pause
    exit /b 1
)

REM 检查 WinDivert SDK
echo [1/6] 检查 WinDivert SDK...
if not exist "core\third_party\WinDivert\include\windivert.h" (
    echo 错误：未找到 WinDivert SDK
    echo 请下载 WinDivert 2.2-A 并解压到 core\third_party\WinDivert\
    echo 下载地址：https://github.com/basil00/Divert/releases/download/v2.2.2-A/WinDivert-2.2.2-A.zip
    pause
    exit /b 1
)
echo WinDivert SDK 已找到

REM 检查 CMake
echo.
echo [2/6] 检查 CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo 错误：未找到 CMake
    echo 请安装 CMake 3.15 或更高版本
    echo 下载地址：https://cmake.org/download/
    pause
    exit /b 1
)
echo CMake 已安装

REM 检查 .NET SDK
echo.
echo [3/6] 检查 .NET SDK...
dotnet --version >nul 2>&1
if errorlevel 1 (
    echo 错误：未找到 .NET SDK
    echo 请安装 .NET 8.0 SDK
    echo 下载地址：https://dotnet.microsoft.com/download/dotnet/8.0
    pause
    exit /b 1
)
echo .NET SDK 已安装

REM 编译核心引擎
echo.
echo [4/6] 编译核心引擎 DLL...
cd core
if not exist "build" mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
if errorlevel 1 (
    echo 错误：CMake 配置失败
    cd ..\..
    pause
    exit /b 1
)
cmake --build . --config Release
if errorlevel 1 (
    echo 错误：核心引擎编译失败
    cd ..\..
    pause
    exit /b 1
)
cd ..\..
echo 核心引擎编译成功

REM 编译 GUI 应用
echo.
echo [5/6] 编译 GUI 应用程序...
cd gui
dotnet restore
if errorlevel 1 (
    echo 错误：依赖还原失败
    cd ..
    pause
    exit /b 1
)
dotnet publish -c Release -r win-x64 --self-contained -o publish
if errorlevel 1 (
    echo 错误：GUI 编译失败
    cd ..
    pause
    exit /b 1
)
cd ..
echo GUI 应用编译成功

REM 复制核心引擎文件到 GUI 发布目录
echo.
echo [6/6] 打包发布文件...
copy core\build\lib\Release\tunnelproxy.dll gui\publish\
copy core\build\lib\Release\WinDivert.dll gui\publish\
copy core\build\lib\Release\WinDivert64.sys gui\publish\

REM 创建发布包
if not exist "release" mkdir release
cd release
if exist "TunnelProxy-Windows-x64" rmdir /s /q TunnelProxy-Windows-x64
mkdir TunnelProxy-Windows-x64
cd ..

REM 复制所有必要文件
xcopy gui\publish\*.* release\TunnelProxy-Windows-x64\ /E /I /Y

REM 创建 README
echo TunnelProxy Windows 客户端 > release\TunnelProxy-Windows-x64\README.txt
echo. >> release\TunnelProxy-Windows-x64\README.txt
echo 系统要求： >> release\TunnelProxy-Windows-x64\README.txt
echo - Windows 10/11 (x64) >> release\TunnelProxy-Windows-x64\README.txt
echo - 管理员权限 >> release\TunnelProxy-Windows-x64\README.txt
echo. >> release\TunnelProxy-Windows-x64\README.txt
echo 使用方法： >> release\TunnelProxy-Windows-x64\README.txt
echo 1. 右键点击 TunnelProxyGUI.exe，选择"以管理员身份运行" >> release\TunnelProxy-Windows-x64\README.txt
echo 2. 输入设备 ID 和管理平台地址 >> release\TunnelProxy-Windows-x64\README.txt
echo 3. 点击"同步策略"获取配置 >> release\TunnelProxy-Windows-x64\README.txt
echo 4. 点击"启动"开始代理服务 >> release\TunnelProxy-Windows-x64\README.txt

REM 打包为 ZIP
echo.
echo 正在打包...
powershell Compress-Archive -Path release\TunnelProxy-Windows-x64\* -DestinationPath release\TunnelProxy-Windows-x64.zip -Force

echo.
echo ========================================
echo 编译完成！
echo ========================================
echo.
echo 发布文件位置：
echo - 文件夹：release\TunnelProxy-Windows-x64\
echo - ZIP 包：release\TunnelProxy-Windows-x64.zip
echo.
echo 请将 ZIP 包上传到服务器：
echo scp release\TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/
echo.
pause
