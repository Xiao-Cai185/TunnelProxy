namespace TunnelProxyGUI.Models;

public class ProxyConfigModel
{
    public string Type { get; set; } = "SOCKS5";
    public string Host { get; set; } = "127.0.0.1";
    public int Port { get; set; } = 1080;
    public string Username { get; set; } = string.Empty;
    public string Password { get; set; } = string.Empty;

    public NativeMethods.ProxyConfig ToNative()
    {
        return new NativeMethods.ProxyConfig
        {
            Type = Type == "SOCKS5" ? 0 : 1,
            Host = Host,
            Port = Port,
            Username = Username,
            Password = Password
        };
    }

    public static ProxyConfigModel FromNative(NativeMethods.ProxyConfig native)
    {
        return new ProxyConfigModel
        {
            Type = native.Type == 0 ? "SOCKS5" : "HTTP",
            Host = native.Host,
            Port = native.Port,
            Username = native.Username,
            Password = native.Password
        };
    }
}
