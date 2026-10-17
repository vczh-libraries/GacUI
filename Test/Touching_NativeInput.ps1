param(
    [Parameter(Mandatory=$true)][int]$TargetProcessId,
    [ValidateSet('CppTest','CppTest_Metaonly','CppTest_Reflection','GacUI_Host','RemotingTest_Rendering_Native')][string]$Application = 'CppTest_Metaonly',
    [ValidateRange(1,65535)][int]$AsPort = 8890,
    [ValidateRange(1,65535)][int]$CoreAsPort = 8888
)
$ErrorActionPreference = 'Stop'
# Run against an application already launched through copilotExecute.ps1.
# These are Windows input injections, separate from automation's listener injection.
Add-Type @'
using System;
using System.Text;
using System.Threading;
using System.Runtime.InteropServices;
public static class TouchingNativeInput {
    [StructLayout(LayoutKind.Sequential)] public struct Point { public int X,Y; }
    [StructLayout(LayoutKind.Sequential)] public struct Rect { public int Left,Top,Right,Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct PointerInfo {
        public uint Type,Id,Frame,Flags;
        public IntPtr Device,Target;
        public Point Pixel,Himetric,PixelRaw,HimetricRaw;
        public uint Time,History;
        public int InputData;
        public uint KeyStates;
        public ulong PerformanceCount;
        public uint ButtonChange;
    }
    [StructLayout(LayoutKind.Sequential)] public struct TouchInfo {
        public PointerInfo Pointer;
        public uint Flags,Mask;
        public Rect Contact,ContactRaw;
        public uint Orientation,Pressure;
    }
    delegate bool Callback(IntPtr hwnd, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumWindows(Callback callback, IntPtr data);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint process);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr hwnd, StringBuilder text, int size);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr hwnd, StringBuilder text, int size);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr hwnd);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hwnd);
    [DllImport("user32.dll")] static extern uint GetDpiForWindow(IntPtr hwnd);
    [DllImport("user32.dll", SetLastError=true)] static extern bool ClientToScreen(IntPtr hwnd, ref Point point);
    [DllImport("user32.dll")] public static extern IntPtr SetThreadDpiAwarenessContext(IntPtr context);
    [DllImport("user32.dll", SetLastError=true)] public static extern bool InitializeTouchInjection(uint maximum, uint feedback);
    [DllImport("user32.dll", SetLastError=true)] static extern bool InjectTouchInput(uint count, [In] TouchInfo[] contacts);
    [DllImport("user32.dll", SetLastError=true)] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] static extern void mouse_event(uint flags, uint x, uint y, uint data, UIntPtr extra);
    public static IntPtr Window(int target) {
        IntPtr result = IntPtr.Zero;
        EnumWindows((window, data) => {
            uint process; GetWindowThreadProcessId(window, out process);
            if (process != target) return true;
            var text = new StringBuilder(256); GetWindowText(window, text, text.Capacity);
            if (text.ToString() == "Microsoft Visual C++ Runtime Library") throw new Exception("Target has a blocking native runtime error dialog.");
            GetClassName(window, text, text.Capacity);
            if (text.ToString() == "VczhWindow" && IsWindowVisible(window)) result = window;
            return true;
        }, IntPtr.Zero);
        return result;
    }
    public static Point ScreenPoint(IntPtr window, double x, double y) {
        double dpi = GetDpiForWindow(window) / 96.0;
        var point = new Point { X=(int)Math.Round(x*dpi), Y=(int)Math.Round(y*dpi) };
        if (!ClientToScreen(window, ref point)) throw new Exception("ClientToScreen failed.");
        return point;
    }
    public static TouchInfo Contact(uint id, uint flags, Point point) {
        return new TouchInfo {
            Pointer = new PointerInfo { Type=2,Id=id,Flags=flags,Pixel=point }, Mask=7,
            Contact = new Rect { Left=point.X-8,Top=point.Y-8,Right=point.X+8,Bottom=point.Y+8 },
            Orientation=90,Pressure=512
        };
    }
    public static int Frame(TouchInfo[] contacts) {
        Thread.Sleep(20);
        for (int i=0;i<5;i++) {
            if (InjectTouchInput((uint)contacts.Length, contacts)) return 0;
            int error=Marshal.GetLastWin32Error();
            if (error!=21) return error;
            Thread.Sleep(2);
        }
        return 21;
    }
    public static bool MouseClick(Point point) {
        if (!SetCursorPos(point.X,point.Y)) return false;
        mouse_event(2,0,0,0,UIntPtr.Zero);
        mouse_event(4,0,0,0,UIntPtr.Zero);
        return true;
    }
}
'@
$base = "http://localhost:$AsPort/Automation/$Application"
$remoteRenderer = $Application -eq 'RemotingTest_Rendering_Native'
$controlsBase = if ($remoteRenderer) { "http://localhost:$CoreAsPort/Automation/RemotingTest_Core" } else { $base }
function Get-Nodes($value) {
    if ($null -eq $value) { return }
    if ($value -is [System.Array]) { foreach ($item in $value) { Get-Nodes $item }; return }
    if ($value -is [System.Management.Automation.PSCustomObject]) {
        $value
        foreach ($property in $value.PSObject.Properties) { Get-Nodes $property.Value }
    }
}
function Snapshot {
    $null = [TouchingNativeInput]::Window($TargetProcessId)
    Get-Nodes (Invoke-RestMethod "$controlsBase/Controls" -TimeoutSec 5)
}
function Find-Element([string]$id) { @(Snapshot | Where-Object elementId -EQ $id)[0] }
function Caption([string]$text) { @(Snapshot | Where-Object elementText -EQ $text)[0] }
function Bounds($node) {
    # The Controls dump uses global logical client bounds for every node.
    $node.bounds
}
function Command([string]$command) {
    $response = Invoke-WebRequest "$base/IO" -Method Post -ContentType 'application/json; charset=utf8' -Body ([Text.Encoding]::UTF8.GetBytes($command)) -UseBasicParsing -TimeoutSec 5
    if ($response.Content -ne 'Queued') { throw $response.Content }
}
function Open-Touching {
    for ($i=0; $i -lt 6; $i++) {
        if (Find-Element 'FullControlTest.Touching') { return }
        $node = Caption 'Touching'
        if (!$node) { $node = Caption 'Misc' }
        $b = Bounds $node
        Command "!LeftClick:$([int](($b.x1+$b.x2)/2)),$([int](($b.y1+$b.y2)/2))"
        Start-Sleep -Milliseconds 150
    }
    throw 'Touching page could not be opened.'
}
function State {
    $element = Find-Element 'FullControlTest.Touching'
    if ($remoteRenderer) {
        $matched = $false
        for ($attempt=0; $attempt -lt 20; $attempt++) {
            $raw = @(Get-Nodes (Invoke-RestMethod "$base/Dom" -TimeoutSec 5) | Where-Object rawId -EQ 'FullControlTest.Touching')[0]
            if ($raw.data -eq $element.elementData) { $matched = $true; break }
            Start-Sleep -Milliseconds 25
        }
        if (!$matched) { throw 'Renderer Dom did not receive Core Touching state.' }
    }
    @($element.elementData.Split(',') | ForEach-Object { [double]::Parse($_,[Globalization.CultureInfo]::InvariantCulture) })
}
function Frame($contacts) {
    $errorCode = [TouchingNativeInput]::Frame([TouchingNativeInput+TouchInfo[]]$contacts)
    if ($errorCode) { throw "InjectTouchInput failed with Win32 error $errorCode." }
}
$window = [TouchingNativeInput]::Window($TargetProcessId)
if (!$window) { throw 'No visible owned GacUI HWND.' }
$previousDpi = [TouchingNativeInput]::SetThreadDpiAwarenessContext([IntPtr](-4))
try {
    Open-Touching
    $initial = State
    $b = Bounds (Find-Element 'FullControlTest.Touching')
    $p0 = [TouchingNativeInput]::ScreenPoint($window, $b.x1+20, $b.y1+20)
    $p1 = [TouchingNativeInput]::ScreenPoint($window, $b.x1+50, $b.y1+40)
    $null = [TouchingNativeInput]::SetForegroundWindow($window)
    if (![TouchingNativeInput]::InitializeTouchInjection(10,3)) { throw "InitializeTouchInjection failed with Win32 error $([Runtime.InteropServices.Marshal]::GetLastWin32Error())." }
    Frame @([TouchingNativeInput]::Contact(1,0x10006,$p0))
    Frame @([TouchingNativeInput]::Contact(1,0x20006,$p1))
    Frame @([TouchingNativeInput]::Contact(1,0x40000,$p1))
    Start-Sleep -Milliseconds 200
    $after = State
    if ([Math]::Abs($after[0]-$initial[0]-30) -gt 1 -or [Math]::Abs($after[1]-$initial[1]-20) -gt 1) {
        throw 'Native frames were not observed at the target. Check locked/disconnected desktop and foreground access; listener injection is not equivalent.'
    }
    Write-Output 'PASS native WM_POINTER one-contact translation and completion.'

    $p2 = [TouchingNativeInput]::ScreenPoint($window, $b.x1+120, $b.y1+20)
    $p3 = [TouchingNativeInput]::ScreenPoint($window, $b.x1+220, $b.y1+20)
    Frame @([TouchingNativeInput]::Contact(1,0x10006,$p0))
    Frame @([TouchingNativeInput]::Contact(1,0x20006,$p0), [TouchingNativeInput]::Contact(2,0x10006,$p2))
    Frame @([TouchingNativeInput]::Contact(1,0x20006,$p0), [TouchingNativeInput]::Contact(2,0x20006,$p3))
    Frame @([TouchingNativeInput]::Contact(1,0x40000,$p0), [TouchingNativeInput]::Contact(2,0x20006,$p3))
    Frame @([TouchingNativeInput]::Contact(2,0x48000,$p3))
    Start-Sleep -Milliseconds 200
    $scaled = State
    if ([Math]::Abs($scaled[2]-2*$after[2]) -gt 0.01 -or $scaled[4] -ne 0) { throw 'Native multitouch/canceled-Up result did not match.' }
    Write-Output 'PASS native multiple contacts, original contact lifts first, and canceled Up.'

    $tab = Bounds (Caption 'Raw Clock')
    $tap = [TouchingNativeInput]::ScreenPoint($window, ($tab.x1+$tab.x2)/2, ($tab.y1+$tab.y2)/2)
    Frame @([TouchingNativeInput]::Contact(1,0x10006,$tap))
    Frame @([TouchingNativeInput]::Contact(1,0x20006,$tap))
    Frame @([TouchingNativeInput]::Contact(1,0x40000,$tap))
    Start-Sleep -Milliseconds 200
    if (!(Find-Element 'FullControlTest.Clock')) { throw 'Rejected native touch did not retain compatibility mouse click.' }
    if ($remoteRenderer) { Write-Output 'PASS rejected native touch uses Core mouse fallback through renderer forwarding.' }
    else { Write-Output 'PASS rejected native touch retains OS mouse fallback.' }

    $tab = Bounds (Caption 'Touching')
    $tap = [TouchingNativeInput]::ScreenPoint($window, ($tab.x1+$tab.x2)/2, ($tab.y1+$tab.y2)/2)
    if (![TouchingNativeInput]::MouseClick($tap)) { throw 'Unpromoted mouse injection is unavailable on this desktop.' }
    Start-Sleep -Milliseconds 200
    if (!(Find-Element 'FullControlTest.Touching')) { throw 'Unpromoted mouse input failed after accepted touch.' }
    Write-Output 'PASS independent unpromoted Windows mouse injection. Physical touch/mouse and additional monitor DPI remain hardware checks.'
} finally {
    $null = [TouchingNativeInput]::SetThreadDpiAwarenessContext($previousDpi)
}
