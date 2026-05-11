package handlers

import (
	"net/http"
	"time"
	"tunnelproxy-server/internal/models"
	"tunnelproxy-server/internal/services"

	"github.com/gin-gonic/gin"
)

// SyncHandler 同步处理器
type SyncHandler struct {
	deviceService      *services.DeviceService
	policyService      *services.PolicyService
	proxyConfigService *services.ProxyConfigService
}

// NewSyncHandler 创建同步处理器实例
func NewSyncHandler() *SyncHandler {
	return &SyncHandler{
		deviceService:      services.NewDeviceService(),
		policyService:      services.NewPolicyService(),
		proxyConfigService: services.NewProxyConfigService(),
	}
}

// SyncPolicies 同步策略
func (h *SyncHandler) SyncPolicies(c *gin.Context) {
	deviceID := c.Param("device_id")

	// 检查设备是否存在
	device, err := h.deviceService.GetDevice(deviceID)
	if err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "Device not found"})
		return
	}

	// 获取策略列表
	policies, err := h.policyService.GetPoliciesByDevice(deviceID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	// 获取代理配置
	proxyConfig, err := h.proxyConfigService.GetByDeviceID(deviceID)
	if err != nil {
		// 代理配置可能不存在，不算错误
		proxyConfig = nil
	}

	// 更新最后同步时间
	if err := h.deviceService.UpdateLastSync(deviceID); err != nil {
		// 记录错误但不影响响应
	}

	// 构造响应
	response := models.SyncResponse{
		DeviceID:    device.ID,
		Policies:    policies,
		ProxyConfig: proxyConfig,
		SyncTime:    time.Now(),
	}

	c.JSON(http.StatusOK, response)
}

// Heartbeat 心跳
func (h *SyncHandler) Heartbeat(c *gin.Context) {
	var req models.HeartbeatRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// 检查设备是否存在
	device, err := h.deviceService.GetDevice(req.DeviceID)
	if err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "Device not found"})
		return
	}

	// 更新设备状态
	if err := h.deviceService.UpdateStatus(req.DeviceID, req.Status); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	// 检查是否需要同步（简单实现：检查最后同步时间）
	needSync := time.Since(device.LastSync) > 5*time.Minute

	response := models.HeartbeatResponse{
		Success:      true,
		Message:      "Heartbeat received",
		ServerTime:   time.Now(),
		NeedSync:     needSync,
		LastSyncTime: device.LastSync,
	}

	c.JSON(http.StatusOK, response)
}
