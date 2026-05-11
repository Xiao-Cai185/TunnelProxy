package services

import (
	"database/sql"
	"fmt"
	"time"
	"tunnelproxy-server/internal/database"
	"tunnelproxy-server/internal/models"

	"github.com/google/uuid"
)

// DeviceService 设备服务
type DeviceService struct{}

// NewDeviceService 创建设备服务实例
func NewDeviceService() *DeviceService {
	return &DeviceService{}
}

// CreateDevice 创建设备
func (s *DeviceService) CreateDevice(req *models.DeviceCreateRequest) (*models.Device, error) {
	device := &models.Device{
		ID:        uuid.New().String(),
		Name:      req.Name,
		Platform:  req.Platform,
		Status:    "offline",
		CreatedAt: time.Now(),
		UpdatedAt: time.Now(),
	}

	query := `INSERT INTO devices (id, name, platform, status, created_at, updated_at)
	          VALUES (?, ?, ?, ?, ?, ?)`

	_, err := database.DB.Exec(query, device.ID, device.Name, device.Platform,
		device.Status, device.CreatedAt, device.UpdatedAt)
	if err != nil {
		return nil, fmt.Errorf("failed to create device: %w", err)
	}

	return device, nil
}

// GetDevice 获取设备详情
func (s *DeviceService) GetDevice(id string) (*models.Device, error) {
	device := &models.Device{}
	query := `SELECT id, name, platform, last_sync, status, created_at, updated_at
	          FROM devices WHERE id = ?`

	err := database.DB.QueryRow(query, id).Scan(
		&device.ID, &device.Name, &device.Platform, &device.LastSync,
		&device.Status, &device.CreatedAt, &device.UpdatedAt,
	)
	if err == sql.ErrNoRows {
		return nil, fmt.Errorf("device not found")
	}
	if err != nil {
		return nil, fmt.Errorf("failed to get device: %w", err)
	}

	return device, nil
}

// ListDevices 获取设备列表
func (s *DeviceService) ListDevices() ([]models.Device, error) {
	query := `SELECT id, name, platform, last_sync, status, created_at, updated_at
	          FROM devices ORDER BY created_at DESC`

	rows, err := database.DB.Query(query)
	if err != nil {
		return nil, fmt.Errorf("failed to list devices: %w", err)
	}
	defer rows.Close()

	var devices []models.Device
	for rows.Next() {
		var device models.Device
		err := rows.Scan(&device.ID, &device.Name, &device.Platform, &device.LastSync,
			&device.Status, &device.CreatedAt, &device.UpdatedAt)
		if err != nil {
			return nil, fmt.Errorf("failed to scan device: %w", err)
		}
		devices = append(devices, device)
	}

	return devices, nil
}

// UpdateDevice 更新设备
func (s *DeviceService) UpdateDevice(id string, req *models.DeviceUpdateRequest) error {
	query := `UPDATE devices SET name = ?, status = ?, updated_at = ? WHERE id = ?`

	result, err := database.DB.Exec(query, req.Name, req.Status, time.Now(), id)
	if err != nil {
		return fmt.Errorf("failed to update device: %w", err)
	}

	rows, err := result.RowsAffected()
	if err != nil {
		return fmt.Errorf("failed to get rows affected: %w", err)
	}
	if rows == 0 {
		return fmt.Errorf("device not found")
	}

	return nil
}

// DeleteDevice 删除设备
func (s *DeviceService) DeleteDevice(id string) error {
	query := `DELETE FROM devices WHERE id = ?`

	result, err := database.DB.Exec(query, id)
	if err != nil {
		return fmt.Errorf("failed to delete device: %w", err)
	}

	rows, err := result.RowsAffected()
	if err != nil {
		return fmt.Errorf("failed to get rows affected: %w", err)
	}
	if rows == 0 {
		return fmt.Errorf("device not found")
	}

	return nil
}

// UpdateLastSync 更新最后同步时间
func (s *DeviceService) UpdateLastSync(id string) error {
	query := `UPDATE devices SET last_sync = ?, updated_at = ? WHERE id = ?`

	_, err := database.DB.Exec(query, time.Now(), time.Now(), id)
	if err != nil {
		return fmt.Errorf("failed to update last sync: %w", err)
	}

	return nil
}

// UpdateStatus 更新设备状态
func (s *DeviceService) UpdateStatus(id string, status string) error {
	query := `UPDATE devices SET status = ?, updated_at = ? WHERE id = ?`

	_, err := database.DB.Exec(query, status, time.Now(), id)
	if err != nil {
		return fmt.Errorf("failed to update status: %w", err)
	}

	return nil
}
