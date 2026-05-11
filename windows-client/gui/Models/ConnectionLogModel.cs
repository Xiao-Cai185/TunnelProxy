using System;

namespace TunnelProxyGUI.Models;

public class ConnectionLogModel
{
    public string Time { get; set; } = string.Empty;
    public string Protocol { get; set; } = string.Empty;
    public string Process { get; set; } = string.Empty;
    public string Destination { get; set; } = string.Empty;
    public string Action { get; set; } = string.Empty;

    public ConnectionLogModel() { }

    public ConnectionLogModel(string protocol, string process, string destination, string action)
    {
        Time = DateTime.Now.ToString("HH:mm:ss");
        Protocol = protocol;
        Process = process;
        Destination = destination;
        Action = action;
    }
}
