package services

import (
	"database/sql"
	"fmt"
	"time"
	"tunnelproxy-server/internal/database"
	"tunnelproxy-server/internal/models"

	"github.com/google/uuid"
)

// PolicyService 策略服务
type PolicyService struct{}

// NewPolicyService 创建策略服务实例
func NewPolicyService() *PolicyService {
	return &PolicyService{}
}

// CreatePolicy 创建策略
func (s *PolicyService) CreatePolicy(req *models.PolicyCreateRequest) (*models.Policy, error) {
	policy := &models.Policy{
		ID:        uuid.New().String(),
		DeviceID:  req.DeviceID,
		AppName:   req.AppName,
		Action:    req.Action,
		Enabled:   req.Enabled,
		CreatedAt: time.Now(),
		UpdatedAt: time.Now(),
	}

	query := `INSERT INTO policies (id, device_id, app_name, action, enabled, created_at, updated_at)
	          VALUES (?, ?, ?, ?, ?, ?, ?)`

	_, err := database.DB.Exec(query, policy.ID, policy.DeviceID, policy.AppName,
		policy.Action, policy.Enabled, policy.CreatedAt, policy.UpdatedAt)
	if err != nil {
		return nil, fmt.Errorf("failed to create policy: %w", err)
	}

	return policy, nil
}

// GetPolicy 获取策略详情
func (s *PolicyService) GetPolicy(id string) (*models.Policy, error) {
	policy := &models.Policy{}
	query := `SELECT id, device_id, app_name, action, enabled, created_at, updated_at
	          FROM policies WHERE id = ?`

	err := database.DB.QueryRow(query, id).Scan(
		&policy.ID, &policy.DeviceID, &policy.AppName, &policy.Action,
		&policy.Enabled, &policy.CreatedAt, &policy.UpdatedAt,
	)
	if err == sql.ErrNoRows {
		return nil, fmt.Errorf("policy not found")
	}
	if err != nil {
		return nil, fmt.Errorf("failed to get policy: %w", err)
	}

	return policy, nil
}

// GetPoliciesByDevice 获取设备的所有策略
func (s *PolicyService) GetPoliciesByDevice(deviceID string) ([]models.Policy, error) {
	query := `SELECT id, device_id, app_name, action, enabled, created_at, updated_at
	          FROM policies WHERE device_id = ? ORDER BY created_at DESC`

	rows, err := database.DB.Query(query, deviceID)
	if err != nil {
		return nil, fmt.Errorf("failed to get policies: %w", err)
	}
	defer rows.Close()

	var policies []models.Policy
	for rows.Next() {
		var policy models.Policy
		err := rows.Scan(&policy.ID, &policy.DeviceID, &policy.AppName, &policy.Action,
			&policy.Enabled, &policy.CreatedAt, &policy.UpdatedAt)
		if err != nil {
			return nil, fmt.Errorf("failed to scan policy: %w", err)
		}
		policies = append(policies, policy)
	}

	return policies, nil
}

// UpdatePolicy 更新策略
func (s *PolicyService) UpdatePolicy(id string, req *models.PolicyUpdateRequest) error {
	query := `UPDATE policies SET app_name = ?, action = ?, enabled = ?, updated_at = ? WHERE id = ?`

	result, err := database.DB.Exec(query, req.AppName, req.Action, req.Enabled, time.Now(), id)
	if err != nil {
		return fmt.Errorf("failed to update policy: %w", err)
	}

	rows, err := result.RowsAffected()
	if err != nil {
		return fmt.Errorf("failed to get rows affected: %w", err)
	}
	if rows == 0 {
		return fmt.Errorf("policy not found")
	}

	return nil
}

// DeletePolicy 删除策略
func (s *PolicyService) DeletePolicy(id string) error {
	query := `DELETE FROM policies WHERE id = ?`

	result, err := database.DB.Exec(query, id)
	if err != nil {
		return fmt.Errorf("failed to delete policy: %w", err)
	}

	rows, err := result.RowsAffected()
	if err != nil {
		return fmt.Errorf("failed to get rows affected: %w", err)
	}
	if rows == 0 {
		return fmt.Errorf("policy not found")
	}

	return nil
}
