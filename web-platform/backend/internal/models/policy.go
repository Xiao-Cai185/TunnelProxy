package models

import "time"

// Policy 策略模型
type Policy struct {
	ID        string    `json:"id" db:"id"`
	DeviceID  string    `json:"device_id" db:"device_id"`
	AppName   string    `json:"app_name" db:"app_name"`
	Action    string    `json:"action" db:"action"` // PROXY, DIRECT, BLOCK
	Enabled   bool      `json:"enabled" db:"enabled"`
	CreatedAt time.Time `json:"created_at" db:"created_at"`
	UpdatedAt time.Time `json:"updated_at" db:"updated_at"`
}

// PolicyCreateRequest 创建策略请求
type PolicyCreateRequest struct {
	DeviceID string `json:"device_id" binding:"required"`
	AppName  string `json:"app_name" binding:"required"`
	Action   string `json:"action" binding:"required,oneof=PROXY DIRECT BLOCK"`
	Enabled  bool   `json:"enabled"`
}

// PolicyUpdateRequest 更新策略请求
type PolicyUpdateRequest struct {
	AppName string `json:"app_name"`
	Action  string `json:"action" binding:"omitempty,oneof=PROXY DIRECT BLOCK"`
	Enabled *bool  `json:"enabled"`
}

// PolicyListResponse 策略列表响应
type PolicyListResponse struct {
	Policies []Policy `json:"policies"`
	Total    int      `json:"total"`
}
