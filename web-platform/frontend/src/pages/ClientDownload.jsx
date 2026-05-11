import React from 'react';
import { Card, Button, Typography, Space, Tag, Alert, Divider } from 'antd';
import { WindowsOutlined, AppleOutlined, DownloadOutlined, ClockCircleOutlined } from '@ant-design/icons';

const { Title, Paragraph, Text } = Typography;

const ClientDownload = () => {
  const clients = [
    {
      id: 'windows',
      name: 'Windows 客户端',
      icon: <WindowsOutlined style={{ fontSize: 48, color: '#0078D4' }} />,
      version: 'v1.0.0',
      platform: 'Windows 10/11 (x64)',
      size: '待构建',
      status: 'available',
      downloadUrl: '/Client/download/TunnelProxy-Windows-x64.zip',
      features: [
        '基于 WinDivert 的内核级流量拦截',
        '支持 SOCKS5 和 HTTP 代理',
        '进程级规则匹配',
        '实时连接日志',
        '策略自动同步'
      ],
      requirements: [
        'Windows 10 或更高版本',
        '需要管理员权限运行',
        '.NET 8.0 Runtime'
      ]
    },
    {
      id: 'macos',
      name: 'macOS 客户端',
      icon: <AppleOutlined style={{ fontSize: 48, color: '#000000' }} />,
      version: '待开发',
      platform: 'macOS 11.0+',
      size: '-',
      status: 'developing',
      downloadUrl: null,
      features: [
        '基于 Network Extension 的系统扩展',
        '支持 SOCKS5 和 HTTP 代理',
        '进程级规则匹配',
        '实时连接日志',
        '策略自动同步'
      ],
      requirements: [
        'macOS 11.0 (Big Sur) 或更高版本',
        '需要系统扩展权限'
      ]
    }
  ];

  const handleDownload = (client) => {
    if (client.status === 'available' && client.downloadUrl) {
      window.location.href = client.downloadUrl;
    }
  };

  return (
    <div style={{ padding: '24px' }}>
      <Title level={2}>客户端下载</Title>
      <Paragraph>
        下载适合您操作系统的 TunnelProxy 客户端，实现集中式管理的进程级代理。
      </Paragraph>

      <Alert
        message="使用说明"
        description={
          <div>
            <p>1. 下载并解压客户端程序</p>
            <p>2. 以管理员权限运行客户端</p>
            <p>3. 首次启动需要输入管理平台地址（例如：http://144.24.14.106:8080）</p>
            <p>4. 输入设备 ID 进行注册（在设备管理页面创建）</p>
            <p>5. 点击"同步策略"获取最新配置</p>
            <p>6. 点击"启动"开始代理服务</p>
          </div>
        }
        type="info"
        showIcon
        style={{ marginBottom: 24 }}
      />

      <Space direction="vertical" size="large" style={{ width: '100%' }}>
        {clients.map((client) => (
          <Card
            key={client.id}
            hoverable={client.status === 'available'}
            style={{
              opacity: client.status === 'developing' ? 0.7 : 1
            }}
          >
            <div style={{ display: 'flex', gap: 24 }}>
              <div style={{ flex: '0 0 auto', textAlign: 'center' }}>
                {client.icon}
                <div style={{ marginTop: 8 }}>
                  {client.status === 'available' ? (
                    <Tag color="success">可下载</Tag>
                  ) : (
                    <Tag icon={<ClockCircleOutlined />} color="warning">
                      开发中
                    </Tag>
                  )}
                </div>
              </div>

              <div style={{ flex: 1 }}>
                <Title level={4} style={{ marginTop: 0 }}>
                  {client.name}
                </Title>

                <Space direction="vertical" size="small" style={{ width: '100%' }}>
                  <div>
                    <Text strong>版本：</Text>
                    <Text>{client.version}</Text>
                  </div>
                  <div>
                    <Text strong>平台：</Text>
                    <Text>{client.platform}</Text>
                  </div>
                  <div>
                    <Text strong>大小：</Text>
                    <Text>{client.size}</Text>
                  </div>

                  <Divider style={{ margin: '12px 0' }} />

                  <div>
                    <Text strong>功能特性：</Text>
                    <ul style={{ marginTop: 8, marginBottom: 8 }}>
                      {client.features.map((feature, index) => (
                        <li key={index}>
                          <Text type="secondary">{feature}</Text>
                        </li>
                      ))}
                    </ul>
                  </div>

                  <div>
                    <Text strong>系统要求：</Text>
                    <ul style={{ marginTop: 8, marginBottom: 8 }}>
                      {client.requirements.map((req, index) => (
                        <li key={index}>
                          <Text type="secondary">{req}</Text>
                        </li>
                      ))}
                    </ul>
                  </div>
                </Space>

                <div style={{ marginTop: 16 }}>
                  <Button
                    type="primary"
                    icon={<DownloadOutlined />}
                    size="large"
                    disabled={client.status !== 'available'}
                    onClick={() => handleDownload(client)}
                  >
                    {client.status === 'available' ? '下载客户端' : '敬请期待'}
                  </Button>
                </div>
              </div>
            </div>
          </Card>
        ))}
      </Space>

      <Alert
        message="安全提示"
        description="TunnelProxy 客户端需要管理员权限以实现内核级流量拦截。请确保从官方渠道下载，并在受信任的环境中使用。"
        type="warning"
        showIcon
        style={{ marginTop: 24 }}
      />
    </div>
  );
};

export default ClientDownload;
