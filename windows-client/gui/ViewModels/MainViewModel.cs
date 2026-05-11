using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;
using TunnelProxyGUI.Models;

namespace TunnelProxyGUI.ViewModels;

public class MainViewModel : INotifyPropertyChanged
{
    private bool _isRunning;
    private string _deviceId = string.Empty;
    private string _syncServerUrl = string.Empty;
    private string _statusMessage = "就绪";
    private long _totalConnections;
    private long _proxiedConnections;
    private long _directConnections;
    private long _blockedConnections;

    // 回调委托（保持引用防止被GC回收）
    private readonly NativeMethods.LogCallback _logCallback;
    private readonly NativeMethods.ConnectionCallback _connectionCallback;

    public MainViewModel()
    {
        // 初始化回调
        _logCallback = OnLog;
        _connectionCallback = OnConnection;

        // 初始化命令
        StartCommand = new RelayCommand(Start, () => !IsRunning);
        StopCommand = new RelayCommand(Stop, () => IsRunning);
        SyncCommand = new RelayCommand(Sync, () => !string.IsNullOrEmpty(SyncServerUrl));
        AddPolicyCommand = new RelayCommand(AddPolicy);
        RemovePolicyCommand = new RelayCommand<PolicyRuleModel>(RemovePolicy);
        ClearLogsCommand = new RelayCommand(() => ConnectionLogs.Clear());

        // 初始化集合
        Policies = new ObservableCollection<PolicyRuleModel>();
        ConnectionLogs = new ObservableCollection<ConnectionLogModel>();

        // 初始化代理配置
        ProxyConfig = new ProxyConfigModel();

        // 初始化核心引擎
        if (NativeMethods.TunnelProxy_Init(_logCallback, _connectionCallback))
        {
            StatusMessage = "核心引擎初始化成功";
        }
        else
        {
            StatusMessage = "核心引擎初始化失败";
        }
    }

    // 属性
    public bool IsRunning
    {
        get => _isRunning;
        set
        {
            if (_isRunning != value)
            {
                _isRunning = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsNotRunning));
                ((RelayCommand)StartCommand).RaiseCanExecuteChanged();
                ((RelayCommand)StopCommand).RaiseCanExecuteChanged();
            }
        }
    }

    public bool IsNotRunning => !IsRunning;

    public string DeviceId
    {
        get => _deviceId;
        set
        {
            if (_deviceId != value)
            {
                _deviceId = value;
                OnPropertyChanged();
            }
        }
    }

    public string SyncServerUrl
    {
        get => _syncServerUrl;
        set
        {
            if (_syncServerUrl != value)
            {
                _syncServerUrl = value;
                OnPropertyChanged();
                ((RelayCommand)SyncCommand).RaiseCanExecuteChanged();
            }
        }
    }

    public string StatusMessage
    {
        get => _statusMessage;
        set
        {
            if (_statusMessage != value)
            {
                _statusMessage = value;
                OnPropertyChanged();
            }
        }
    }

    public long TotalConnections
    {
        get => _totalConnections;
        set { _totalConnections = value; OnPropertyChanged(); }
    }

    public long ProxiedConnections
    {
        get => _proxiedConnections;
        set { _proxiedConnections = value; OnPropertyChanged(); }
    }

    public long DirectConnections
    {
        get => _directConnections;
        set { _directConnections = value; OnPropertyChanged(); }
    }

    public long BlockedConnections
    {
        get => _blockedConnections;
        set { _blockedConnections = value; OnPropertyChanged(); }
    }

    public ProxyConfigModel ProxyConfig { get; set; }
    public ObservableCollection<PolicyRuleModel> Policies { get; }
    public ObservableCollection<ConnectionLogModel> ConnectionLogs { get; }

    // 命令
    public ICommand StartCommand { get; }
    public ICommand StopCommand { get; }
    public ICommand SyncCommand { get; }
    public ICommand AddPolicyCommand { get; }
    public ICommand RemovePolicyCommand { get; }
    public ICommand ClearLogsCommand { get; }

    // 方法
    private void Start()
    {
        // 设置代理配置
        var proxyConfig = ProxyConfig.ToNative();
        if (!NativeMethods.TunnelProxy_SetProxyConfig(ref proxyConfig))
        {
            StatusMessage = "设置代理配置失败";
            return;
        }

        // 设置同步服务器
        if (!string.IsNullOrEmpty(SyncServerUrl) && !string.IsNullOrEmpty(DeviceId))
        {
            NativeMethods.TunnelProxy_SetSyncServer(SyncServerUrl, DeviceId);
        }

        // 启动核心引擎
        if (NativeMethods.TunnelProxy_Start())
        {
            IsRunning = true;
            StatusMessage = "TunnelProxy 已启动";

            // 启动统计更新定时器
            var timer = new System.Timers.Timer(1000);
            timer.Elapsed += (s, e) =>
            {
                if (!IsRunning)
                {
                    timer.Stop();
                    return;
                }

                var stats = new NativeMethods.ConnectionStats();
                if (NativeMethods.TunnelProxy_GetStats(ref stats))
                {
                    TotalConnections = stats.TotalConnections;
                    ProxiedConnections = stats.ProxiedConnections;
                    DirectConnections = stats.DirectConnections;
                    BlockedConnections = stats.BlockedConnections;
                }
            };
            timer.Start();
        }
        else
        {
            StatusMessage = "启动失败";
        }
    }

    private void Stop()
    {
        NativeMethods.TunnelProxy_Stop();
        IsRunning = false;
        StatusMessage = "TunnelProxy 已停止";
    }

    private void Sync()
    {
        if (string.IsNullOrEmpty(SyncServerUrl) || string.IsNullOrEmpty(DeviceId))
        {
            StatusMessage = "请先配置同步服务器和设备ID";
            return;
        }

        StatusMessage = "正在同步策略...";

        if (NativeMethods.TunnelProxy_SyncPolicies())
        {
            StatusMessage = "策略同步成功";

            // 重新加载策略列表
            LoadPolicies();
        }
        else
        {
            StatusMessage = "策略同步失败";
        }
    }

    private void AddPolicy()
    {
        var policy = new PolicyRuleModel
        {
            AppName = "chrome.exe",
            Action = "PROXY",
            Enabled = true
        };

        var nativePolicy = policy.ToNative();
        if (NativeMethods.TunnelProxy_AddPolicy(ref nativePolicy))
        {
            Policies.Add(policy);
            StatusMessage = $"已添加策略：{policy.AppName}";
        }
        else
        {
            StatusMessage = "添加策略失败";
        }
    }

    private void RemovePolicy(PolicyRuleModel? policy)
    {
        if (policy == null) return;

        if (NativeMethods.TunnelProxy_RemovePolicy(policy.AppName))
        {
            Policies.Remove(policy);
            StatusMessage = $"已删除策略：{policy.AppName}";
        }
        else
        {
            StatusMessage = "删除策略失败";
        }
    }

    private void LoadPolicies()
    {
        Policies.Clear();

        int count = NativeMethods.TunnelProxy_GetPolicyCount();
        if (count > 0)
        {
            var policies = new NativeMethods.PolicyRule[count];
            if (NativeMethods.TunnelProxy_GetPolicies(policies, count))
            {
                foreach (var policy in policies)
                {
                    Policies.Add(PolicyRuleModel.FromNative(policy));
                }
            }
        }
    }

    // 回调处理
    private void OnLog(string message)
    {
        StatusMessage = message;
    }

    private void OnConnection(string protocol, string process, string destAddr, ushort destPort, string action)
    {
        var log = new ConnectionLogModel(protocol, process, $"{destAddr}:{destPort}", action);

        // 在UI线程上添加日志
        Avalonia.Threading.Dispatcher.UIThread.Post(() =>
        {
            ConnectionLogs.Insert(0, log);

            // 限制日志数量
            while (ConnectionLogs.Count > 1000)
            {
                ConnectionLogs.RemoveAt(ConnectionLogs.Count - 1);
            }
        });
    }

    // INotifyPropertyChanged 实现
    public event PropertyChangedEventHandler? PropertyChanged;

    protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}

// 简单的 RelayCommand 实现
public class RelayCommand : ICommand
{
    private readonly Action _execute;
    private readonly Func<bool>? _canExecute;

    public RelayCommand(Action execute, Func<bool>? canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }

    public event EventHandler? CanExecuteChanged;

    public bool CanExecute(object? parameter) => _canExecute?.Invoke() ?? true;

    public void Execute(object? parameter) => _execute();

    public void RaiseCanExecuteChanged() => CanExecuteChanged?.Invoke(this, EventArgs.Empty);
}

public class RelayCommand<T> : ICommand
{
    private readonly Action<T?> _execute;
    private readonly Func<T?, bool>? _canExecute;

    public RelayCommand(Action<T?> execute, Func<T?, bool>? canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }

    public event EventHandler? CanExecuteChanged;

    public bool CanExecute(object? parameter) => _canExecute?.Invoke((T?)parameter) ?? true;

    public void Execute(object? parameter) => _execute((T?)parameter);

    public void RaiseCanExecuteChanged() => CanExecuteChanged?.Invoke(this, EventArgs.Empty);
}
