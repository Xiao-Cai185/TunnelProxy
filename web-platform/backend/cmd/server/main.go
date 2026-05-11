package main

import (
	"log"
	"tunnelproxy-server/internal/database"
	"tunnelproxy-server/internal/handlers"

	"github.com/gin-gonic/gin"
)

func main() {
	// 初始化数据库
	if err := database.InitDB("./tunnelproxy.db"); err != nil {
		log.Fatalf("Failed to initialize database: %v", err)
	}
	defer database.CloseDB()

	// 创建 Gin 路由
	r := gin.Default()

	// CORS 中间件
	r.Use(func(c *gin.Context) {
		c.Writer.Header().Set("Access-Control-Allow-Origin", "*")
		c.Writer.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Content-Type, Authorization")

		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}

		c.Next()
	})

	// 创建处理器
	deviceHandler := handlers.NewDeviceHandler()
	syncHandler := handlers.NewSyncHandler()
	proxyConfigHandler := handlers.NewProxyConfigHandler()

	// API 路由
	api := r.Group("/api")
	{
		// 设备管理
		devices := api.Group("/devices")
		{
			devices.POST("", deviceHandler.CreateDevice)
			devices.GET("", deviceHandler.ListDevices)
			devices.GET("/:id", deviceHandler.GetDevice)
			devices.PUT("/:id", deviceHandler.UpdateDevice)
			devices.DELETE("/:id", deviceHandler.DeleteDevice)
		}

		// 代理配置
		api.GET("/proxy-config", proxyConfigHandler.GetProxyConfig)
		api.PUT("/proxy-config", proxyConfigHandler.UpdateProxyConfig)

		// 同步接口
		sync := api.Group("/sync")
		{
			sync.GET("/:device_id", syncHandler.SyncPolicies)
			sync.POST("/heartbeat", syncHandler.Heartbeat)
		}
	}

	// 启动服务器
	log.Println("Server starting on :8080")
	if err := r.Run(":8080"); err != nil {
		log.Fatalf("Failed to start server: %v", err)
	}
}
