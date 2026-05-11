#ifndef HTTP_CONNECT_H
#define HTTP_CONNECT_H

#include "../include/tunnelproxy.h"

// HTTP CONNECT 握手
bool HttpConnect_Handshake(SOCKET sock, const ProxyConfig *config);

// HTTP CONNECT 连接请求
bool HttpConnect_Connect(SOCKET sock, uint32_t dest_ip, uint16_t dest_port);

#endif // HTTP_CONNECT_H
