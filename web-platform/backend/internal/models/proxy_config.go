package models

import "time"

// ProxyConfig 代理配置模型
type ProxyConfig struct {
	ID        string    `json:"id" db:"id"`
	DeviceID  string    `json:"device_id" db:"device_id"`
	Type      string    `json:"type" db:"type"` // SOCKS5, HTTP
	Host      string    `json:"host" db:"host"`
	Port      int       `json:"port" db:"port"`
	Username  string    `json:"username" db:"username"`
	Password  string    `json:"password" db:"password"`
	CreatedAt time.Time `json:"created_at" db:"created_at"`
	UpdatedAt time.Time `json:"updated_at" db:"updated_at"`
}

// ProxyConfigCreateRequest 创建代理配置请求
type ProxyConfigCreateRequest struct {
	DeviceID string `json:"device_id" binding:"required"`
	Type     string `json:"type" binding:"required,oneof=SOCKS5 HTTP"`
	Host     string `json:"host" binding:"required"`
	Port     int    `json:"port" binding:"required,min=1,max=65535"`
	Username string `json:"username"`
	Password string `json:"password"`
}

// ProxyConfigUpdateRequest 更新代理配置请求
type ProxyConfigUpdateRequest struct {
	Type     string `json:"type" binding:"omitempty,oneof=SOCKS5 HTTP"`
	Host     string `json:"host"`
	Port     int    `json:"port" binding:"omitempty,min=1,max=65535"`
	Username string `json:"username"`
	Password string `json:"password"`
}
