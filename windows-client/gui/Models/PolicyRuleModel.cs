namespace TunnelProxyGUI.Models;

public class PolicyRuleModel
{
    public string AppName { get; set; } = string.Empty;
    public string Action { get; set; } = "PROXY";
    public bool Enabled { get; set; } = true;

    public PolicyRuleModel() { }

    public PolicyRuleModel(string appName, string action, bool enabled)
    {
        AppName = appName;
        Action = action;
        Enabled = enabled;
    }

    public NativeMethods.PolicyRule ToNative()
    {
        return new NativeMethods.PolicyRule
        {
            AppName = AppName,
            Action = Action switch
            {
                "PROXY" => 0,
                "DIRECT" => 1,
                "BLOCK" => 2,
                _ => 0
            },
            Enabled = Enabled
        };
    }

    public static PolicyRuleModel FromNative(NativeMethods.PolicyRule native)
    {
        return new PolicyRuleModel
        {
            AppName = native.AppName,
            Action = native.Action switch
            {
                0 => "PROXY",
                1 => "DIRECT",
                2 => "BLOCK",
                _ => "PROXY"
            },
            Enabled = native.Enabled
        };
    }
}
