package services

import (
	"database/sql"
	"fmt"
	"time"
	"tunnelproxy-server/internal/database"
	"tunnelproxy-server/internal/models"

	"github.com/google/uuid"
)

// ProxyConfigService 代理配置服务
type ProxyConfigService struct{}

// NewProxyConfigService 创建代理配置服务实例
func NewProxyConfigService() *ProxyConfigService {
	return &ProxyConfigService{}
}

// CreateProxyConfig 创建代理配置
func (s *ProxyConfigService) CreateProxyConfig(req *models.ProxyConfigCreateRequest) (*models.ProxyConfig, error) {
	config := &models.ProxyConfig{
		ID:        uuid.New().String(),
		DeviceID:  req.DeviceID,
		Type:      req.Type,
		Host:      req.Host,
		Port:      req.Port,
		Username:  req.Username,
		Password:  req.Password,
		CreatedAt: time.Now(),
		UpdatedAt: time.Now(),
	}

	query := `INSERT INTO proxy_configs (id, device_id, type, host, port, username, password, created_at, updated_at)
	          VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)`

	_, err := database.DB.Exec(query, config.ID, config.DeviceID, config.Type, config.Host,
		config.Port, config.Username, config.Password, config.CreatedAt, config.UpdatedAt)
	if err != nil {
		return nil, fmt.Errorf("failed to create proxy config: %w", err)
	}

	return config, nil
}

// GetByDeviceID 根据设备ID获取代理配置
func (s *ProxyConfigService) GetByDeviceID(deviceID string) (*models.ProxyConfig, error) {
	config := &models.ProxyConfig{}
	query := `SELECT id, device_id, type, host, port, username, password, created_at, updated_at
	          FROM proxy_configs WHERE device_id = ?`

	err := database.DB.QueryRow(query, deviceID).Scan(
		&config.ID, &config.DeviceID, &config.Type, &config.Host, &config.Port,
		&config.Username, &config.Password, &config.CreatedAt, &config.UpdatedAt,
	)
	if err == sql.ErrNoRows {
		return nil, fmt.Errorf("proxy config not found")
	}
	if err != nil {
		return nil, fmt.Errorf("failed to get proxy config: %w", err)
	}

	return config, nil
}

// UpdateProxyConfig 更新代理配置
func (s *ProxyConfigService) UpdateProxyConfig(id string, req *models.ProxyConfigUpdateRequest) error {
	query := `UPDATE proxy_configs SET type = ?, host = ?, port = ?, username = ?, password = ?, updated_at = ? WHERE id = ?`

	result, err := database.DB.Exec(query, req.Type, req.Host, req.Port, req.Username, req.Password, time.Now(), id)
	if err != nil {
		return fmt.Errorf("failed to update proxy config: %w", err)
	}

	rows, err := result.RowsAffected()
	if err != nil {
		return fmt.Errorf("failed to get rows affected: %w", err)
	}
	if rows == 0 {
		return fmt.Errorf("proxy config not found")
	}

	return nil
}

// DeleteProxyConfig 删除代理配置
func (s *ProxyConfigService) DeleteProxyConfig(id string) error {
	query := `DELETE FROM proxy_configs WHERE id = ?`

	result, err := database.DB.Exec(query, id)
	if err != nil {
		return fmt.Errorf("failed to delete proxy config: %w", err)
	}

	rows, err := result.RowsAffected()
	if err != nil {
		return fmt.Errorf("failed to get rows affected: %w", err)
	}
	if rows == 0 {
		return fmt.Errorf("proxy config not found")
	}

	return nil
}
