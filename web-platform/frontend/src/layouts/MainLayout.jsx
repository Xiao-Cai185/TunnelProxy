import { useState } from 'react';
import { Layout, Menu } from 'antd';
import {
  DesktopOutlined,
  FileTextOutlined,
  SettingOutlined,
  DownloadOutlined,
} from '@ant-design/icons';
import { Outlet, useNavigate, useLocation } from 'react-router-dom';

const { Header, Sider, Content } = Layout;

const MainLayout = () => {
  const [collapsed, setCollapsed] = useState(false);
  const navigate = useNavigate();
  const location = useLocation();

  const menuItems = [
    {
      key: '/devices',
      icon: <DesktopOutlined />,
      label: '设备管理',
    },
    {
      key: '/policies',
      icon: <FileTextOutlined />,
      label: '策略管理',
    },
    {
      key: '/proxy-config',
      icon: <SettingOutlined />,
      label: '代理配置',
    },
    {
      key: '/download',
      icon: <DownloadOutlined />,
      label: '客户端下载',
    },
  ];

  const handleMenuClick = ({ key }) => {
    navigate(key);
  };

  return (
    <Layout style={{ minHeight: '100vh' }}>
      <Sider collapsible collapsed={collapsed} onCollapse={setCollapsed}>
        <div
          style={{
            height: '64px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            color: 'white',
            fontSize: '20px',
            fontWeight: 'bold',
          }}
        >
          {collapsed ? 'TP' : 'TunnelProxy'}
        </div>
        <Menu
          theme="dark"
          selectedKeys={[location.pathname]}
          mode="inline"
          items={menuItems}
          onClick={handleMenuClick}
        />
      </Sider>
      <Layout>
        <Header
          style={{
            padding: '0 24px',
            background: '#fff',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
          }}
        >
          <h1 style={{ margin: 0 }}>TunnelProxy 管理平台</h1>
        </Header>
        <Content style={{ margin: '0', background: '#f0f2f5', minHeight: 'calc(100vh - 64px)' }}>
          <Outlet />
        </Content>
      </Layout>
    </Layout>
  );
};

export default MainLayout;
