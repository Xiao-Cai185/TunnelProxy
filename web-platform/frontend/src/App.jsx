import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import { ConfigProvider } from 'antd';
import zhCN from 'antd/locale/zh_CN';
import MainLayout from './layouts/MainLayout';
import DeviceManagement from './pages/DeviceManagement';
import PolicyManagement from './pages/PolicyManagement';
import ProxyConfig from './pages/ProxyConfig';
import ClientDownload from './pages/ClientDownload';

function App() {
  return (
    <ConfigProvider locale={zhCN}>
      <BrowserRouter>
        <Routes>
          <Route path="/" element={<MainLayout />}>
            <Route index element={<Navigate to="/devices" replace />} />
            <Route path="devices" element={<DeviceManagement />} />
            <Route path="policies" element={<PolicyManagement />} />
            <Route path="proxy-config" element={<ProxyConfig />} />
            <Route path="download" element={<ClientDownload />} />
          </Route>
        </Routes>
      </BrowserRouter>
    </ConfigProvider>
  );
}

export default App;
