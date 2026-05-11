using System;
using System.Runtime.InteropServices;

namespace TunnelProxyGUI;

/// <summary>
/// P/Invoke 调用 TunnelProxy 核心引擎
/// </summary>
public static class NativeMethods
{
    private const string DllName = "tunnelproxy.dll";

    // 回调函数委托
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void LogCallback([MarshalAs(UnmanagedType.LPStr)] string message);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ConnectionCallback(
        [MarshalAs(UnmanagedType.LPStr)] string protocol,
        [MarshalAs(UnmanagedType.LPStr)] string process,
        [MarshalAs(UnmanagedType.LPStr)] string destAddr,
        ushort destPort,
        [MarshalAs(UnmanagedType.LPStr)] string action);

    // 代理配置结构
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct ProxyConfig
    {
        public int Type; // 0 = SOCKS5, 1 = HTTP
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Host;
        public int Port;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Username;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Password;
    }

    // 策略规则结构
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct PolicyRule
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string AppName;
        public int Action; // 0 = PROXY, 1 = DIRECT, 2 = BLOCK
        [MarshalAs(UnmanagedType.Bool)]
        public bool Enabled;
    }

    // 统计信息结构
    [StructLayout(LayoutKind.Sequential)]
    public struct ConnectionStats
    {
        public long TotalConnections;
        public long ProxiedConnections;
        public long DirectConnections;
        public long BlockedConnections;
    }

    // 核心引擎 API
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_Init(LogCallback logCallback, ConnectionCallback connCallback);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_Start();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void TunnelProxy_Stop();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void TunnelProxy_Cleanup();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_IsRunning();

    // 配置管理 API
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_SetProxyConfig(ref ProxyConfig config);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_GetProxyConfig(ref ProxyConfig config);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_AddPolicy(ref PolicyRule policy);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_RemovePolicy([MarshalAs(UnmanagedType.LPStr)] string appName);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void TunnelProxy_ClearPolicies();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int TunnelProxy_GetPolicyCount();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_GetPolicies(
        [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] PolicyRule[] policies,
        int maxCount);

    // 同步管理 API
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_SetSyncServer(
        [MarshalAs(UnmanagedType.LPStr)] string serverUrl,
        [MarshalAs(UnmanagedType.LPStr)] string deviceId);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_SyncPolicies();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_SendHeartbeat();

    // 统计信息 API
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool TunnelProxy_GetStats(ref ConnectionStats stats);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void TunnelProxy_ResetStats();
}
