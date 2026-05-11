import { useState, useEffect } from 'react';
import { Table, Button, Modal, Form, Input, Select, Tag, Space, message } from 'antd';
import { PlusOutlined, EditOutlined, DeleteOutlined, SyncOutlined } from '@ant-design/icons';
import { deviceAPI } from '../services/api';

const { Option } = Select;

const DeviceManagement = () => {
  const [devices, setDevices] = useState([]);
  const [loading, setLoading] = useState(false);
  const [modalVisible, setModalVisible] = useState(false);
  const [editingDevice, setEditingDevice] = useState(null);
  const [form] = Form.useForm();

  // 加载设备列表
  const loadDevices = async () => {
    setLoading(true);
    try {
      const data = await deviceAPI.getDevices();
      setDevices(data.devices || []);
    } catch (error) {
      message.error('加载设备列表失败');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadDevices();
  }, []);

  // 打开新建/编辑对话框
  const handleOpenModal = (device = null) => {
    setEditingDevice(device);
    if (device) {
      form.setFieldsValue(device);
    } else {
      form.resetFields();
    }
    setModalVisible(true);
  };

  // 关闭对话框
  const handleCloseModal = () => {
    setModalVisible(false);
    setEditingDevice(null);
    form.resetFields();
  };

  // 提交表单
  const handleSubmit = async () => {
    try {
      const values = await form.validateFields();

      if (editingDevice) {
        // 更新设备
        await deviceAPI.updateDevice(editingDevice.id, values);
        message.success('设备更新成功');
      } else {
        // 创建设备
        await deviceAPI.createDevice(values);
        message.success('设备创建成功');
      }

      handleCloseModal();
      loadDevices();
    } catch (error) {
      message.error('操作失败');
    }
  };

  // 删除设备
  const handleDelete = (device) => {
    Modal.confirm({
      title: '确认删除',
      content: `确定要删除设备 "${device.name}" 吗？`,
      okText: '确定',
      cancelText: '取消',
      onOk: async () => {
        try {
          await deviceAPI.deleteDevice(device.id);
          message.success('设备删除成功');
          loadDevices();
        } catch (error) {
          message.error('删除失败');
        }
      },
    });
  };

  // 表格列定义
  const columns = [
    {
      title: '设备名称',
      dataIndex: 'name',
      key: 'name',
    },
    {
      title: '平台',
      dataIndex: 'platform',
      key: 'platform',
      render: (platform) => {
        const colorMap = {
          Windows: 'blue',
          macOS: 'green',
          Linux: 'orange',
        };
        return <Tag color={colorMap[platform]}>{platform}</Tag>;
      },
    },
    {
      title: '状态',
      dataIndex: 'status',
      key: 'status',
      render: (status) => {
        const colorMap = {
          online: 'success',
          offline: 'default',
        };
        const textMap = {
          online: '在线',
          offline: '离线',
        };
        return <Tag color={colorMap[status]}>{textMap[status]}</Tag>;
      },
    },
    {
      title: '最后同步时间',
      dataIndex: 'last_sync',
      key: 'last_sync',
      render: (time) => time ? new Date(time).toLocaleString('zh-CN') : '-',
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
      <div style={{ marginBottom: '16px', display: 'flex', justifyContent: 'space-between' }}>
        <h2>设备管理</h2>
        <Space>
          <Button icon={<SyncOutlined />} onClick={loadDevices}>
            刷新
          </Button>
          <Button type="primary" icon={<PlusOutlined />} onClick={() => handleOpenModal()}>
            添加设备
          </Button>
        </Space>
      </div>

      <Table
        columns={columns}
        dataSource={devices}
        rowKey="id"
        loading={loading}
        pagination={{ pageSize: 10 }}
      />

      <Modal
        title={editingDevice ? '编辑设备' : '添加设备'}
        open={modalVisible}
        onOk={handleSubmit}
        onCancel={handleCloseModal}
        okText="确定"
        cancelText="取消"
      >
        <Form form={form} layout="vertical">
          <Form.Item
            name="name"
            label="设备名称"
            rules={[{ required: true, message: '请输入设备名称' }]}
          >
            <Input placeholder="请输入设备名称" />
          </Form.Item>

          <Form.Item
            name="platform"
            label="平台"
            rules={[{ required: true, message: '请选择平台' }]}
          >
            <Select placeholder="请选择平台">
              <Option value="Windows">Windows</Option>
              <Option value="macOS">macOS</Option>
              <Option value="Linux">Linux</Option>
            </Select>
          </Form.Item>
        </Form>
      </Modal>
    </div>
  );
};

export default DeviceManagement;
