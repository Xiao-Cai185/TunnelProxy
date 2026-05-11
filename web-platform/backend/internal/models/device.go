package models

import "time"

// Device 设备模型
type Device struct {
	ID         string    `json:"id" db:"id"`
	Name       string    `json:"name" db:"name"`
	Platform   string    `json:"platform" db:"platform"` // Windows, macOS, Linux
	LastSync   time.Time `json:"last_sync" db:"last_sync"`
	Status     string    `json:"status" db:"status"` // online, offline
	CreatedAt  time.Time `json:"created_at" db:"created_at"`
	UpdatedAt  time.Time `json:"updated_at" db:"updated_at"`
}

// DeviceCreateRequest 创建设备请求
type DeviceCreateRequest struct {
	Name     string `json:"name" binding:"required"`
	Platform string `json:"platform" binding:"required,oneof=Windows macOS Linux"`
}

// DeviceUpdateRequest 更新设备请求
type DeviceUpdateRequest struct {
	Name   string `json:"name"`
	Status string `json:"status" binding:"omitempty,oneof=online offline"`
}
