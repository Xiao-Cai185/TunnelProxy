import { useState, useEffect } from 'react';
import {
  Table,
  Button,
  Modal,
  Form,
  Input,
  Select,
  Switch,
  Space,
  message,
  Card,
} from 'antd';
import { PlusOutlined, EditOutlined, DeleteOutlined, SyncOutlined } from '@ant-design/icons';
import { policyAPI, deviceAPI } from '../services/api';

const { Option } = Select;

const PolicyManagement = () => {
  const [devices, setDevices] = useState([]);
  const [selectedDeviceId, setSelectedDeviceId] = useState(null);
  const [policies, setPolicies] = useState([]);
  const [loading, setLoading] = useState(false);
  const [modalVisible, setModalVisible] = useState(false);
  const [editingPolicy, setEditingPolicy] = useState(null);
  const [form] = Form.useForm();

  // 加载设备列表
  const loadDevices = async () => {
    try {
      const data = await deviceAPI.getDevices();
      const deviceList = data.devices || [];
      setDevices(deviceList);
      if (deviceList.length > 0 && !selectedDeviceId) {
        setSelectedDeviceId(deviceList[0].id);
      }
    } catch (error) {
      message.error('加载设备列表失败');
    }
  };

  // 加载策略列表
  const loadPolicies = async (deviceId) => {
    if (!deviceId) return;

    setLoading(true);
    try {
      const data = await policyAPI.getPolicies(deviceId);
      setPolicies(data || []);
    } catch (error) {
      message.error('加载策略列表失败');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadDevices();
  }, []);

  useEffect(() => {
    if (selectedDeviceId) {
      loadPolicies(selectedDeviceId);
    }
  }, [selectedDeviceId]);

  // 打开新建/编辑对话框
  const handleOpenModal = (policy = null) => {
    setEditingPolicy(policy);
    if (policy) {
      form.setFieldsValue(policy);
    } else {
      form.resetFields();
      form.setFieldsValue({ enabled: true });
    }
    setModalVisible(true);
  };

  // 关闭对话框
  const handleCloseModal = () => {
    setModalVisible(false);
    setEditingPolicy(null);
    form.resetFields();
  };

  // 提交表单
  const handleSubmit = async () => {
    try {
      const values = await form.validateFields();
      values.device_id = selectedDeviceId;

      if (editingPolicy) {
        // 更新策略
        await policyAPI.updatePolicy(editingPolicy.id, values);
        message.success('策略更新成功');
      } else {
        // 创建策略
        await policyAPI.createPolicy(values);
        message.success('策略创建成功');
      }

      handleCloseModal();
      loadPolicies(selectedDeviceId);
    } catch (error) {
      message.error('操作失败');
    }
  };

  // 删除策略
  const handleDelete = (policy) => {
    Modal.confirm({
      title: '确认删除',
      content: `确定要删除策略 "${policy.app_name}" 吗？`,
      okText: '确定',
      cancelText: '取消',
      onOk: async () => {
        try {
          await policyAPI.deletePolicy(policy.id);
          message.success('策略删除成功');
          loadPolicies(selectedDeviceId);
        } catch (error) {
          message.error('删除失败');
        }
      },
    });
  };

  // 切换策略启用状态
  const handleToggleEnabled = async (policy) => {
    try {
      await policyAPI.updatePolicy(policy.id, {
        ...policy,
        enabled: !policy.enabled,
      });
      message.success('状态更新成功');
      loadPolicies(selectedDeviceId);
    } catch (error) {
      message.error('状态更新失败');
    }
  };

  // 表格列定义
  const columns = [
    {
      title: '应用名称',
      dataIndex: 'app_name',
      key: 'app_name',
    },
    {
      title: '动作',
      dataIndex: 'action',
      key: 'action',
      render: (action) => {
        const colorMap = {
          PROXY: 'blue',
          DIRECT: 'green',
          BLOCK: 'red',
        };
        const textMap = {
          PROXY: '代理',
          DIRECT: '直连',
          BLOCK: '阻止',
        };
        return (
          <span style={{ color: colorMap[action], fontWeight: 'bold' }}>
            {textMap[action]}
          </span>
        );
      },
    },
    {
      title: '状态',
      dataIndex: 'enabled',
      key: 'enabled',
      render: (enabled, record) => (
        <Switch
          checked={enabled}
          onChange={() => handleToggleEnabled(record)}
          checkedChildren="启用"
          unCheckedChildren="禁用"
        />
      ),
    },
    {
      title: '创建时间',
      dataIndex: 'created_at',
      key: 'created_at',
      render: (time) => new Date(time).toLocaleString('zh-CN'),
    },
    {
      title: '操作',
      key: 'action',
      render: (_, record) => (
        <Space size="middle">
          <Button
            type="link"
            icon={<EditOutlined />}
            onClick={() => handleOpenModal(record)}
          >
            编辑
          </Button>
          <Button
            type="link"
            danger
            icon={<DeleteOutlined />}
            onClick={() => handleDelete(record)}
          >
            删除
          </Button>
        </Space>
      ),
    },
  ];

  return (
    <div style={{ padding: '24px' }}>
      <Card style={{ marginBottom: '16px' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '16px' }}>
          <span style={{ fontWeight: 'bold' }}>选择设备：</span>
          <Select
            style={{ width: 300 }}
            value={selectedDeviceId}
            onChange={setSelectedDeviceId}
            placeholder="请选择设备"
          >
            {devices.map((device) => (
              <Option key={device.id} value={device.id}>
                {device.name} ({device.platform})
              </Option>
            ))}
          </Select>
        </div>
      </Card>

      <div style={{ marginBottom: '16px', display: 'flex', justifyContent: 'space-between' }}>
        <h2>策略管理</h2>
        <Space>
          <Button
            icon={<SyncOutlined />}
            onClick={() => loadPolicies(selectedDeviceId)}
            disabled={!selectedDeviceId}
          >
            刷新
          </Button>
          <Button
            type="primary"
            icon={<PlusOutlined />}
            onClick={() => handleOpenModal()}
            disabled={!selectedDeviceId}
          >
            添加策略
          </Button>
        </Space>
      </div>

      <Table
        columns={columns}
        dataSource={policies}
        rowKey="id"
        loading={loading}
        pagination={{ pageSize: 10 }}
      />

      <Modal
        title={editingPolicy ? '编辑策略' : '添加策略'}
        open={modalVisible}
        onOk={handleSubmit}
        onCancel={handleCloseModal}
        okText="确定"
        cancelText="取消"
      >
        <Form form={form} layout="vertical">
          <Form.Item
            name="app_name"
            label="应用名称"
            rules={[{ required: true, message: '请输入应用名称' }]}
            extra="支持通配符：chrome.exe, *.exe, chr*"
          >
            <Input placeholder="例如：chrome.exe" />
          </Form.Item>

          <Form.Item
            name="action"
            label="动作"
            rules={[{ required: true, message: '请选择动作' }]}
          >
            <Select placeholder="请选择动作">
              <Option value="PROXY">代理</Option>
              <Option value="DIRECT">直连</Option>
              <Option value="BLOCK">阻止</Option>
            </Select>
          </Form.Item>

          <Form.Item name="enabled" label="启用" valuePropName="checked">
            <Switch checkedChildren="启用" unCheckedChildren="禁用" />
          </Form.Item>
        </Form>
      </Modal>
    </div>
  );
};

export default PolicyManagement;
