import axios from 'axios';

// 创建 axios 实例
const api = axios.create({
  baseURL: 'http://144.24.14.106:8080/api',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json',
  },
});

// 请求拦截器
api.interceptors.request.use(
  (config) => {
    // 可以在这里添加 token
    return config;
  },
  (error) => {
    return Promise.reject(error);
  }
);

// 响应拦截器
api.interceptors.response.use(
  (response) => {
    return response.data;
  },
  (error) => {
    console.error('API Error:', error);
    return Promise.reject(error);
  }
);

// ==================== 设备管理 API ====================

export const deviceAPI = {
  // 获取所有设备
  getDevices: () => api.get('/devices'),

  // 获取单个设备
  getDevice: (id) => api.get(`/devices/${id}`),

  // 创建设备
  createDevice: (data) => api.post('/devices', data),

  // 更新设备
  updateDevice: (id, data) => api.put(`/devices/${id}`, data),

  // 删除设备
  deleteDevice: (id) => api.delete(`/devices/${id}`),
};

// ==================== 策略管理 API ====================

export const policyAPI = {
  // 获取设备的所有策略
  getPolicies: (deviceId) => api.get(`/policies?device_id=${deviceId}`),

  // 获取单个策略
  getPolicy: (id) => api.get(`/policies/${id}`),

  // 创建策略
  createPolicy: (data) => api.post('/policies', data),

  // 更新策略
  updatePolicy: (id, data) => api.put(`/policies/${id}`, data),

  // 删除策略
  deletePolicy: (id) => api.delete(`/policies/${id}`),

  // 批量更新策略
  batchUpdatePolicies: (deviceId, policies) =>
    api.post(`/policies/batch`, { device_id: deviceId, policies }),
};

// ==================== 代理配置 API ====================

export const proxyAPI = {
  // 获取代理配置
  getProxyConfig: () => api.get('/proxy-config'),

  // 更新代理配置
  updateProxyConfig: (data) => api.put('/proxy-config', data),
};

// ==================== 同步 API ====================

export const syncAPI = {
  // 获取设备同步的策略
  getSyncPolicies: (deviceId) => api.get(`/sync/policies?device_id=${deviceId}`),

  // 设备状态上报
  reportStatus: (deviceId, stats) =>
    api.post(`/sync/status`, { device_id: deviceId, ...stats }),
};

export default api;
