cmake_minimum_required(VERSION 3.15)
project(TunnelProxy C)

set(CMAKE_C_STANDARD 11)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/windivert
)

# 源文件
set(SOURCES
    src/tunnelproxy.c
    src/windivert/packet_handler.c
    src/utils/process.c
    src/rules/rule_engine.c
    src/proxy/socks5.c
    src/proxy/http_connect.c
    src/relay/tcp_relay.c
    src/relay/udp_relay.c
    src/sync/sync.c
    src/config/config.c
)

# 创建动态链接库
add_library(tunnelproxy SHARED ${SOURCES})

# Windows 特定配置
if(WIN32)
    # 添加预处理器定义，防止 winsock.h 被包含
    target_compile_definitions(tunnelproxy PRIVATE
        WIN32_LEAN_AND_MEAN
        _WINSOCK_DEPRECATED_NO_WARNINGS
        TUNNELPROXY_EXPORTS
    )

    # 链接 Windows 库
    target_link_libraries(tunnelproxy
        ws2_32      # Winsock2
        iphlpapi    # IP Helper API
        winhttp     # WinHTTP
        advapi32    # Advanced API
    )

    # 添加 WinDivert 库路径（需要下载 WinDivert SDK）
    # 假设 WinDivert 在 third_party/WinDivert 目录
    set(WINDIVERT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/WinDivert")

    if(EXISTS ${WINDIVERT_DIR})
        target_include_directories(tunnelproxy PRIVATE ${WINDIVERT_DIR}/include)

        # 根据架构选择库文件
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64-bit
            target_link_libraries(tunnelproxy ${WINDIVERT_DIR}/x64/WinDivert.lib)
            # 复制 WinDivert.dll 到输出目录
            add_custom_command(TARGET tunnelproxy POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${WINDIVERT_DIR}/x64/WinDivert.dll"
                "$<TARGET_FILE_DIR:tunnelproxy>"
            )
            add_custom_command(TARGET tunnelproxy POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${WINDIVERT_DIR}/x64/WinDivert64.sys"
                "$<TARGET_FILE_DIR:tunnelproxy>"
            )
        else()
            # 32-bit
            target_link_libraries(tunnelproxy ${WINDIVERT_DIR}/x86/WinDivert.lib)
            add_custom_command(TARGET tunnelproxy POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${WINDIVERT_DIR}/x86/WinDivert.dll"
                "$<TARGET_FILE_DIR:tunnelproxy>"
            )
            add_custom_command(TARGET tunnelproxy POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${WINDIVERT_DIR}/x86/WinDivert32.sys"
                "$<TARGET_FILE_DIR:tunnelproxy>"
            )
        endif()
    else()
        message(WARNING "WinDivert SDK not found at ${WINDIVERT_DIR}")
        message(WARNING "Please download WinDivert from https://github.com/basil00/Divert/releases")
    endif()
endif()

# 安装规则
install(TARGETS tunnelproxy
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES include/tunnelproxy.h DESTINATION include)
