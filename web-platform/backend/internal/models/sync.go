package models

import "time"

// SyncResponse 同步响应
type SyncResponse struct {
	DeviceID    string       `json:"device_id"`
	Policies    []Policy     `json:"policies"`
	ProxyConfig *ProxyConfig `json:"proxy_config"`
	SyncTime    time.Time    `json:"sync_time"`
}

// HeartbeatRequest 心跳请求
type HeartbeatRequest struct {
	DeviceID string `json:"device_id" binding:"required"`
	Status   string `json:"status" binding:"required,oneof=online offline"`
}

// HeartbeatResponse 心跳响应
type HeartbeatResponse struct {
	Success      bool      `json:"success"`
	Message      string    `json:"message"`
	ServerTime   time.Time `json:"server_time"`
	NeedSync     bool      `json:"need_sync"`
	LastSyncTime time.Time `json:"last_sync_time"`
}
