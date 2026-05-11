#include "../include/tunnelproxy.h"
#include "http_connect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Base64 编码表
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64 编码
static int base64_encode(const unsigned char *input, int input_len, char *output, int output_size) {
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (input_len--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                if (j >= output_size - 1) return -1;
                output[j++] = base64_table[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (int k = i; k < 3; k++) {
            char_array_3[k] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (int k = 0; k < i + 1; k++) {
            if (j >= output_size - 1) return -1;
            output[j++] = base64_table[char_array_4[k]];
        }

        while (i++ < 3) {
            if (j >= output_size - 1) return -1;
            output[j++] = '=';
        }
    }

    output[j] = '\0';
    return j;
}

// 发送数据（完整发送）
static bool send_all(SOCKET sock, const char *data, int len) {
    int sent = 0;
    while (sent < len) {
        int ret = send(sock, data + sent, len - sent, 0);
        if (ret == SOCKET_ERROR) {
            return false;
        }
        sent += ret;
    }
    return true;
}

// 接收一行数据（直到 \r\n）
static int recv_line(SOCKET sock, char *buffer, int buffer_size) {
    int pos = 0;
    while (pos < buffer_size - 1) {
        char c;
        int ret = recv(sock, &c, 1, 0);
        if (ret <= 0) {
            return -1;
        }

        buffer[pos++] = c;

        // 检查是否到达行尾
        if (pos >= 2 && buffer[pos - 2] == '\r' && buffer[pos - 1] == '\n') {
            buffer[pos] = '\0';
            return pos;
        }
    }

    return -1;
}

// HTTP CONNECT 握手（不需要单独握手，直接发送 CONNECT 请求）
bool HttpConnect_Handshake(SOCKET sock, const ProxyConfig *config) {
    // HTTP CONNECT 不需要单独握手
    return true;
}

// HTTP CONNECT 连接请求
bool HttpConnect_Connect(SOCKET sock, uint32_t dest_ip, uint16_t dest_port) {
    char request[1024];
    char response[1024];

    // 格式化目标地址
    char dest_addr[32];
    snprintf(dest_addr, sizeof(dest_addr), "%d.%d.%d.%d:%d",
             (dest_ip >> 0) & 0xFF, (dest_ip >> 8) & 0xFF,
             (dest_ip >> 16) & 0xFF, (dest_ip >> 24) & 0xFF,
             dest_port);

    // 构造 CONNECT 请求
    int request_len = snprintf(request, sizeof(request),
                              "CONNECT %s HTTP/1.1\r\n"
                              "Host: %s\r\n"
                              "Proxy-Connection: Keep-Alive\r\n",
                              dest_addr, dest_addr);

    // 如果需要认证，添加 Proxy-Authorization 头
    extern ProxyConfig g_proxy_config;
    if (g_proxy_config.username[0] != '\0') {
        char auth_str[512];
        snprintf(auth_str, sizeof(auth_str), "%s:%s",
                g_proxy_config.username, g_proxy_config.password);

        char auth_base64[1024];
        if (base64_encode((unsigned char *)auth_str, (int)strlen(auth_str),
                         auth_base64, sizeof(auth_base64)) > 0) {
            request_len += snprintf(request + request_len, sizeof(request) - request_len,
                                   "Proxy-Authorization: Basic %s\r\n", auth_base64);
        }
    }

    // 添加空行结束请求
    request_len += snprintf(request + request_len, sizeof(request) - request_len, "\r\n");

    // 发送请求
    if (!send_all(sock, request, request_len)) {
        return false;
    }

    // 接收响应状态行
    if (recv_line(sock, response, sizeof(response)) <= 0) {
        return false;
    }

    // 解析状态码
    // HTTP/1.1 200 Connection established\r\n
    int status_code = 0;
    if (sscanf(response, "HTTP/%*d.%*d %d", &status_code) != 1) {
        return false;
    }

    // 检查状态码
    if (status_code != 200) {
        return false;
    }

    // 读取剩余的响应头（直到空行）
    while (true) {
        if (recv_line(sock, response, sizeof(response)) <= 0) {
            return false;
        }

        // 空行表示响应头结束
        if (strcmp(response, "\r\n") == 0) {
            break;
        }
    }

    return true;
}
