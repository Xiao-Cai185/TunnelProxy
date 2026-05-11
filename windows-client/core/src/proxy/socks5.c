#include "../include/tunnelproxy.h"
#include "socks5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 发送数据（完整发送）
static bool send_all(SOCKET sock, const unsigned char *data, int len) {
    int sent = 0;
    while (sent < len) {
        int ret = send(sock, (const char *)(data + sent), len - sent, 0);
        if (ret == SOCKET_ERROR) {
            return false;
        }
        sent += ret;
    }
    return true;
}

// 接收数据（完整接收）
static bool recv_all(SOCKET sock, unsigned char *data, int len) {
    int received = 0;
    while (received < len) {
        int ret = recv(sock, (char *)(data + received), len - received, 0);
        if (ret <= 0) {
            return false;
        }
        received += ret;
    }
    return true;
}

// SOCKS5 认证（用户名密码）
static bool socks5_auth_password(SOCKET sock, const char *username, const char *password) {
    unsigned char auth_req[513];
    unsigned char auth_resp[2];

    // 构造认证请求
    // +----+------+----------+------+----------+
    // |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
    // +----+------+----------+------+----------+
    // | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
    // +----+------+----------+------+----------+

    int ulen = (int)strlen(username);
    int plen = (int)strlen(password);

    if (ulen > 255 || plen > 255) {
        return false;
    }

    auth_req[0] = 0x01; // 认证协议版本
    auth_req[1] = (unsigned char)ulen;
    memcpy(auth_req + 2, username, ulen);
    auth_req[2 + ulen] = (unsigned char)plen;
    memcpy(auth_req + 3 + ulen, password, plen);

    // 发送认证请求
    if (!send_all(sock, auth_req, 3 + ulen + plen)) {
        return false;
    }

    // 接收认证响应
    if (!recv_all(sock, auth_resp, 2)) {
        return false;
    }

    // 检查认证结果
    return (auth_resp[0] == 0x01 && auth_resp[1] == 0x00);
}

// SOCKS5 握手
bool Socks5_Handshake(SOCKET sock, const ProxyConfig *config) {
    unsigned char handshake_req[4];
    unsigned char handshake_resp[2];

    // 构造握手请求
    // +----+----------+----------+
    // |VER | NMETHODS | METHODS  |
    // +----+----------+----------+
    // | 1  |    1     | 1 to 255 |
    // +----+----------+----------+

    handshake_req[0] = SOCKS5_VERSION;

    bool need_auth = (config->username[0] != '\0');

    if (need_auth) {
        handshake_req[1] = 2; // 支持两种认证方法
        handshake_req[2] = SOCKS5_AUTH_NONE;
        handshake_req[3] = SOCKS5_AUTH_PASSWORD;

        if (!send_all(sock, handshake_req, 4)) {
            return false;
        }
    } else {
        handshake_req[1] = 1; // 只支持无认证
        handshake_req[2] = SOCKS5_AUTH_NONE;

        if (!send_all(sock, handshake_req, 3)) {
            return false;
        }
    }

    // 接收握手响应
    if (!recv_all(sock, handshake_resp, 2)) {
        return false;
    }

    // 检查版本
    if (handshake_resp[0] != SOCKS5_VERSION) {
        return false;
    }

    // 检查认证方法
    unsigned char auth_method = handshake_resp[1];

    if (auth_method == SOCKS5_AUTH_NO_ACCEPTABLE) {
        return false;
    }

    // 如果需要用户名密码认证
    if (auth_method == SOCKS5_AUTH_PASSWORD) {
        return socks5_auth_password(sock, config->username, config->password);
    }

    return true;
}

// SOCKS5 连接请求
bool Socks5_Connect(SOCKET sock, uint32_t dest_ip, uint16_t dest_port) {
    unsigned char connect_req[10];
    unsigned char connect_resp[10];

    // 构造连接请求
    // +----+-----+-------+------+----------+----------+
    // |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
    // +----+-----+-------+------+----------+----------+
    // | 1  |  1  | X'00' |  1   | Variable |    2     |
    // +----+-----+-------+------+----------+----------+

    connect_req[0] = SOCKS5_VERSION;
    connect_req[1] = SOCKS5_CMD_CONNECT;
    connect_req[2] = 0x00; // 保留字段
    connect_req[3] = SOCKS5_ATYP_IPV4;

    // 目标 IP（网络字节序）
    connect_req[4] = (dest_ip >> 0) & 0xFF;
    connect_req[5] = (dest_ip >> 8) & 0xFF;
    connect_req[6] = (dest_ip >> 16) & 0xFF;
    connect_req[7] = (dest_ip >> 24) & 0xFF;

    // 目标端口（网络字节序）
    connect_req[8] = (dest_port >> 8) & 0xFF;
    connect_req[9] = (dest_port >> 0) & 0xFF;

    // 发送连接请求
    if (!send_all(sock, connect_req, 10)) {
        return false;
    }

    // 接收连接响应
    if (!recv_all(sock, connect_resp, 10)) {
        return false;
    }

    // 检查响应
    if (connect_resp[0] != SOCKS5_VERSION) {
        return false;
    }

    if (connect_resp[1] != SOCKS5_REP_SUCCESS) {
        return false;
    }

    return true;
}

// SOCKS5 UDP 关联
bool Socks5_UdpAssociate(SOCKET sock, uint32_t *relay_ip, uint16_t *relay_port) {
    unsigned char associate_req[10];
    unsigned char associate_resp[10];

    // 构造 UDP ASSOCIATE 请求
    associate_req[0] = SOCKS5_VERSION;
    associate_req[1] = SOCKS5_CMD_UDP_ASSOCIATE;
    associate_req[2] = 0x00;
    associate_req[3] = SOCKS5_ATYP_IPV4;

    // 客户端地址（0.0.0.0:0 表示由服务器决定）
    memset(associate_req + 4, 0, 6);

    // 发送请求
    if (!send_all(sock, associate_req, 10)) {
        return false;
    }

    // 接收响应
    if (!recv_all(sock, associate_resp, 10)) {
        return false;
    }

    // 检查响应
    if (associate_resp[0] != SOCKS5_VERSION) {
        return false;
    }

    if (associate_resp[1] != SOCKS5_REP_SUCCESS) {
        return false;
    }

    // 提取中继服务器地址
    if (associate_resp[3] == SOCKS5_ATYP_IPV4) {
        *relay_ip = (associate_resp[4] << 0) |
                   (associate_resp[5] << 8) |
                   (associate_resp[6] << 16) |
                   (associate_resp[7] << 24);

        *relay_port = (associate_resp[8] << 8) |
                     (associate_resp[9] << 0);

        return true;
    }

    return false;
}
