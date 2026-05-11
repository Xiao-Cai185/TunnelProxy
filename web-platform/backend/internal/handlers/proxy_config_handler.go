package handlers

import (
	"net/http"
	"tunnelproxy-server/internal/models"
	"tunnelproxy-server/internal/services"

	"github.com/gin-gonic/gin"
)

// ProxyConfigHandler 代理配置处理器
type ProxyConfigHandler struct {
	service *services.ProxyConfigService
}

// NewProxyConfigHandler 创建代理配置处理器实例
func NewProxyConfigHandler() *ProxyConfigHandler {
	return &ProxyConfigHandler{
		service: services.NewProxyConfigService(),
	}
}

// GetProxyConfig 获取代理配置（全局配置，不区分设备）
func (h *ProxyConfigHandler) GetProxyConfig(c *gin.Context) {
	// 使用固定的全局配置 ID
	config, err := h.service.GetByDeviceID("global")
	if err != nil {
		// 如果没有配置，返回默认值
		c.JSON(http.StatusOK, gin.H{
			"type":     "SOCKS5",
			"host":     "",
			"port":     1080,
			"username": "",
			"password": "",
		})
		return
	}

	c.JSON(http.StatusOK, config)
}

// UpdateProxyConfig 更新代理配置
func (h *ProxyConfigHandler) UpdateProxyConfig(c *gin.Context) {
	var req models.ProxyConfigUpdateRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// 先尝试获取全局配置
	config, err := h.service.GetByDeviceID("global")
	if err != nil {
		// 如果不存在，创建新配置
		createReq := &models.ProxyConfigCreateRequest{
			DeviceID: "global",
			Type:     req.Type,
			Host:     req.Host,
			Port:     req.Port,
			Username: req.Username,
			Password: req.Password,
		}
		config, err = h.service.CreateProxyConfig(createReq)
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
			return
		}
		c.JSON(http.StatusOK, config)
		return
	}

	// 更新现有配置
	if err := h.service.UpdateProxyConfig(config.ID, &req); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	// 重新获取更新后的配置
	config, _ = h.service.GetByDeviceID("global")
	c.JSON(http.StatusOK, config)
}
