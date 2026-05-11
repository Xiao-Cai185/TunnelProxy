#!/bin/bash
# TunnelProxy Windows 客户端上传脚本
# 在 Windows 编译完成后，使用此脚本上传到服务器

echo "========================================"
echo "TunnelProxy 客户端上传脚本"
echo "========================================"
echo ""

# 检查文件是否存在
if [ ! -f "release/TunnelProxy-Windows-x64.zip" ]; then
    echo "错误：未找到编译好的 ZIP 包"
    echo "请先运行 build.bat 编译客户端"
    exit 1
fi

# 显示文件信息
echo "准备上传文件："
ls -lh release/TunnelProxy-Windows-x64.zip
echo ""

# 上传到服务器
echo "正在上传到服务器..."
scp release/TunnelProxy-Windows-x64.zip ubuntu@144.24.14.106:/home/ubuntu/cc/tunnelproxy/web-platform/frontend/public/Client/download/

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "上传成功！"
    echo "========================================"
    echo ""
    echo "客户端下载地址："
    echo "http://144.24.14.106:3001/download"
    echo ""
else
    echo ""
    echo "上传失败，请检查网络连接和服务器地址"
    exit 1
fi
