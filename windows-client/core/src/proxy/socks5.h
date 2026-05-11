#ifndef SOCKS5_H
#define SOCKS5_H

#include "../include/tunnelproxy.h"

// SOCKS5 协议常量
#define SOCKS5_VERSION 0x05
#define SOCKS5_AUTH_NONE 0x00
#define SOCKS5_AUTH_PASSWORD 0x02
#define SOCKS5_AUTH_NO_ACCEPTABLE 0xFF

#define SOCKS5_CMD_CONNECT 0x01
#define SOCKS5_CMD_BIND 0x02
#define SOCKS5_CMD_UDP_ASSOCIATE 0x03

#define SOCKS5_ATYP_IPV4 0x01
#define SOCKS5_ATYP_DOMAIN 0x03
#define SOCKS5_ATYP_IPV6 0x04

#define SOCKS5_REP_SUCCESS 0x00
#define SOCKS5_REP_GENERAL_FAILURE 0x01
#define SOCKS5_REP_CONNECTION_NOT_ALLOWED 0x02
#define SOCKS5_REP_NETWORK_UNREACHABLE 0x03
#define SOCKS5_REP_HOST_UNREACHABLE 0x04
#define SOCKS5_REP_CONNECTION_REFUSED 0x05
#define SOCKS5_REP_TTL_EXPIRED 0x06
#define SOCKS5_REP_COMMAND_NOT_SUPPORTED 0x07
#define SOCKS5_REP_ADDRESS_TYPE_NOT_SUPPORTED 0x08

// SOCKS5 握手
bool Socks5_Handshake(SOCKET sock, const ProxyConfig *config);

// SOCKS5 连接请求
bool Socks5_Connect(SOCKET sock, uint32_t dest_ip, uint16_t dest_port);

// SOCKS5 UDP 关联
bool Socks5_UdpAssociate(SOCKET sock, uint32_t *relay_ip, uint16_t *relay_port);

#endif // SOCKS5_H
