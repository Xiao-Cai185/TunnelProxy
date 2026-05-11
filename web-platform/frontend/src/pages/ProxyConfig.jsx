import { useState, useEffect } from 'react';
import { Form, Input, Select, Button, Card, message, InputNumber } from 'antd';
import { SaveOutlined } from '@ant-design/icons';
import { proxyAPI } from '../services/api';

const { Option } = Select;

const ProxyConfig = () => {
  const [form] = Form.useForm();
  const [loading, setLoading] = useState(false);
  const [saving, setSaving] = useState(false);

  // 加载代理配置
  const loadConfig = async () => {
    setLoading(true);
    try {
      const data = await proxyAPI.getProxyConfig();
      if (data) {
        form.setFieldsValue(data);
      }
    } catch (error) {
      message.error('加载代理配置失败');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadConfig();
  }, []);

  // 保存配置
  const handleSave = async () => {
    try {
      const values = await form.validateFields();
      setSaving(true);

      await proxyAPI.updateProxyConfig(values);
      message.success('代理配置保存成功');
    } catch (error) {
      message.error('保存失败');
    } finally {
      setSaving(false);
    }
  };

  return (
    <div style={{ padding: '24px', maxWidth: '800px' }}>
      <h2>代理服务器配置</h2>
      <Card loading={loading} style={{ marginTop: '16px' }}>
        <Form
          form={form}
          layout="vertical"
          initialValues={{
            type: 'socks5',
            port: 1080,
          }}
        >
          <Form.Item
            name="type"
            label="代理类型"
            rules={[{ required: true, message: '请选择代理类型' }]}
          >
            <Select placeholder="请选择代理类型">
              <Option value="socks5">SOCKS5</Option>
              <Option value="http">HTTP</Option>
            </Select>
          </Form.Item>

          <Form.Item
            name="host"
            label="代理服务器地址"
            rules={[
              { required: true, message: '请输入代理服务器地址' },
              {
                pattern: /^(\d{1,3}\.){3}\d{1,3}$|^[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$/,
                message: '请输入有效的 IP 地址或域名',
              },
            ]}
          >
            <Input placeholder="例如：127.0.0.1 或 proxy.example.com" />
          </Form.Item>

          <Form.Item
            name="port"
            label="端口"
            rules={[
              { required: true, message: '请输入端口' },
              {
                type: 'number',
                min: 1,
                max: 65535,
                message: '端口范围：1-65535',
              },
            ]}
          >
            <InputNumber
              style={{ width: '100%' }}
              placeholder="例如：1080"
              min={1}
              max={65535}
            />
          </Form.Item>

          <Form.Item
            name="username"
            label="用户名"
            extra="如果代理服务器需要认证，请填写用户名"
          >
            <Input placeholder="可选" />
          </Form.Item>

          <Form.Item
            name="password"
            label="密码"
            extra="如果代理服务器需要认证，请填写密码"
          >
            <Input.Password placeholder="可选" />
          </Form.Item>

          <Form.Item>
            <Button
              type="primary"
              icon={<SaveOutlined />}
              onClick={handleSave}
              loading={saving}
              size="large"
            >
              保存配置
            </Button>
          </Form.Item>
        </Form>
      </Card>

      <Card title="配置说明" style={{ marginTop: '16px' }}>
        <ul style={{ paddingLeft: '20px', lineHeight: '2' }}>
          <li>
            <strong>SOCKS5：</strong>支持 TCP 和 UDP 流量代理，推荐使用
          </li>
          <li>
            <strong>HTTP：</strong>仅支持 TCP 流量代理（HTTP CONNECT 方法）
          </li>
          <li>
            <strong>认证：</strong>如果代理服务器需要用户名密码认证，请填写相应信息
          </li>
          <li>
            <strong>生效时间：</strong>配置保存后，客户端下次同步时会自动更新
          </li>
        </ul>
      </Card>
    </div>
  );
};

export default ProxyConfig;
