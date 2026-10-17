#requires -PSEdition Desktop
param(
    [ValidateSet('Pipe','Http','MiniHttp')][string[]]$Transport = @('Pipe','Http','MiniHttp'),
    [ValidateSet('Direct2D','GDI')][string[]]$Backend = @('Direct2D','GDI'),
    [switch]$NativeInjection
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$solution = Join-Path $PSScriptRoot 'GacUISrc'
$wrapper = Join-Path $repo '.github/Scripts/copilotExecute.ps1'
$coreUser = Join-Path $solution 'RemotingTest_Core/RemotingTest_Core.vcxproj.user'
$rendererUser = Join-Path $solution 'RemotingTest_Rendering_Win32/RemotingTest_Rendering_Win32.vcxproj.user'
$coreOriginal = [IO.File]::ReadAllBytes($coreUser)
$rendererOriginal = [IO.File]::ReadAllBytes($rendererUser)
$coreBase = 'http://localhost:8888/Automation/RemotingTest_Core'
$rendererBase = 'http://localhost:8889/Automation/RemotingTest_Rendering_Native'
$owned = [Collections.Generic.List[Diagnostics.Process]]::new()
Add-Type -ReferencedAssemblies System.Drawing @'
using System;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
public static class RemoteTouchProbe {
    delegate bool Callback(IntPtr window, IntPtr data);
    [StructLayout(LayoutKind.Sequential)] struct Rect { public int L,T,R,B; }
    [DllImport("user32.dll")] static extern bool EnumWindows(Callback callback, IntPtr data);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr window, out uint process);
    [DllImport("user32.dll",CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr window,StringBuilder text,int size);
    [DllImport("user32.dll",CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr window,StringBuilder text,int size);
    [DllImport("user32.dll")] static extern bool GetClientRect(IntPtr window,out Rect rect);
    [DllImport("user32.dll")] static extern bool PrintWindow(IntPtr window,IntPtr dc,uint flags);
    public static IntPtr Window(int id) {
        IntPtr result=IntPtr.Zero;
        EnumWindows((window,data)=>{
            uint pid; GetWindowThreadProcessId(window,out pid);
            if(pid!=id) return true;
            var text=new StringBuilder(512);GetWindowText(window,text,text.Capacity);
            if(text.ToString()=="Microsoft Visual C++ Runtime Library") throw new Exception("Blocking native runtime error dialog for owned process.");
            GetClassName(window,text,text.Capacity);
            if(text.ToString()=="VczhWindow") result=window;
            return true;
        },IntPtr.Zero);
        return result;
    }
    public static int[] Pixels(int id,string path) {
        var window=Window(id);Rect rect;
        if(window==IntPtr.Zero || !GetClientRect(window,out rect)) throw new Exception("Renderer HWND unavailable.");
        using(var image=new Bitmap(rect.R,rect.B)) {
            using(var graphics=Graphics.FromImage(image)) {
                var dc=graphics.GetHdc();bool captured;
                try {captured=PrintWindow(window,dc,3);} finally {graphics.ReleaseHdc(dc);}
                if(!captured) throw new Exception("PrintWindow is unavailable in this session.");
            }
            image.Save(path,ImageFormat.Png);
            int sky=0,yellow=0;
            for(int y=0;y<image.Height;y++) for(int x=0;x<image.Width;x++) {
                var c=image.GetPixel(x,y);
                if(c.R==135&&c.G==206&&c.B==235) sky++;
                if(c.R==255&&c.G==255&&c.B==0) yellow++;
            }
            return new[]{sky,yellow};
        }
    }
}
'@
function Set-Arguments($path,$arguments) {
    [xml]$xml = Get-Content -LiteralPath $path -Raw
    $group = $xml.CreateElement('PropertyGroup',$xml.DocumentElement.NamespaceURI)
    $group.SetAttribute('Condition', '''$(Configuration)|$(Platform)''==''Debug|x64''')
    $node = $xml.CreateElement('LocalDebuggerCommandArguments',$xml.DocumentElement.NamespaceURI)
    $node.InnerText = $arguments
    $null = $group.AppendChild($node)
    $null = $xml.DocumentElement.AppendChild($group)
    $xml.Save($path)
}
function Start-Wrapped($name) {
    $before = @(Get-Process -Name $name -ErrorAction SilentlyContinue | ForEach-Object Id)
    if ($before.Count) { throw "An existing $name process would conflict with this test; stop it first." }
    $log = Join-Path $env:TEMP "touch-remote-$PID-$name"
    $runner = Start-Process powershell.exe -WindowStyle Hidden -PassThru -WorkingDirectory $solution -ArgumentList @('-NoProfile','-ExecutionPolicy','Bypass','-File', $wrapper,'-Mode','CLI','-Executable',$name,'-Configuration','Debug','-Platform','x64') -RedirectStandardOutput "$log.out" -RedirectStandardError "$log.err"
    $owned.Add($runner)
    for($i=0;$i -lt 100;$i++) {
        $process = Get-Process -Name $name -ErrorAction SilentlyContinue | Select-Object -First 1
        if($process) {$owned.Add($process); return $process}
        if($runner.HasExited) {throw "Wrapper exited: $(Get-Content "$log.out" -Raw) $(Get-Content "$log.err" -Raw)"}
        Start-Sleep -Milliseconds 100
    }
    throw "Timed out starting $name through the wrapper."
}
function Nodes($value) {
    if($null -eq $value) {return}
    if($value -is [Array]) {foreach($child in $value){Nodes $child};return}
    if($value -is [pscustomobject]) {$value;foreach($property in $value.PSObject.Properties){Nodes $property.Value}}
}
function Controls {Nodes (Invoke-RestMethod "$coreBase/Controls" -TimeoutSec 10)}
function Find-Raw { @(Controls | Where-Object elementId -eq 'FullControlTest.Touching')[0] }
function State { [double[]]((Find-Raw).elementData.Split(',') | ForEach-Object {[double]::Parse($_,[Globalization.CultureInfo]::InvariantCulture)}) }
function IO($endpoint,$command) {
    $answer = Invoke-WebRequest "$endpoint/IO" -Method Post -ContentType 'application/json; charset=utf8' -Body ([Text.Encoding]::UTF8.GetBytes($command)) -UseBasicParsing -TimeoutSec 10
    if($answer.Content -ne 'Queued') {throw "IO $command : $($answer.Content)"}
}
function Wait-For($condition,$description) {
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    do {
        foreach($process in $owned) {if(!$process.HasExited){$null=[RemoteTouchProbe]::Window($process.Id)}}
        try {if(&$condition){return}} catch [System.Net.WebException] { }
        Start-Sleep -Milliseconds 50
    } while([DateTime]::UtcNow -lt $deadline)
    throw "Timed out: $description"
}
function Select-Tab($text) {
    $node = @(Controls | Where-Object elementText -eq $text)[0]
    if(!$node){throw "Missing tab $text"}
    $x=[int](($node.bounds.x1+$node.bounds.x2)/2);$y=[int](($node.bounds.y1+$node.bounds.y2)/2)
    IO $rendererBase "!LeftClick:$x,$y"
}
try {
    foreach($transportName in $Transport) {foreach($backendName in $Backend) {
        $owned.Clear()
        try {
            [IO.File]::WriteAllBytes($coreUser,$coreOriginal)
            [IO.File]::WriteAllBytes($rendererUser,$rendererOriginal)
            Set-Arguments $coreUser "/$transportName /FCT"
            $rendererArgs="/$transportName /AsPort:8889"
            if($backendName -eq 'GDI') {$rendererArgs+=' /Gdi'}
            Set-Arguments $rendererUser $rendererArgs
            $core = Start-Wrapped 'RemotingTest_Core'
            $renderer = Start-Wrapped 'RemotingTest_Rendering_Win32'
            Wait-For { @(Controls | Where-Object elementText -eq 'Misc').Count -gt 0 } 'initial controls'
            Select-Tab 'Misc'
            Wait-For {
                if (@(Controls | Where-Object elementText -eq 'Touching').Count -gt 0) {return $true}
                Start-Sleep -Milliseconds 250
                Select-Tab 'Misc'
                return $false
            } 'Misc tabs'
            Select-Tab 'Touching'
            Wait-For {
                if ($null -ne (Find-Raw)) {return $true}
                Start-Sleep -Milliseconds 250
                Select-Tab 'Touching'
                return $false
            } 'Touching raw element'
            $bounds=(Find-Raw).bounds;$x=[int]$bounds.x1+140;$y=[int]$bounds.y1+120
            foreach($endpoint in @($rendererBase,$coreBase)) {
                $initial=State
                IO $endpoint "!PointDown:10,$x,$y"
                Wait-For {(State)[4] -eq 1} 'first contact'
                if($endpoint -eq $rendererBase) {
                    Start-Sleep -Milliseconds 100
                    $picturePath=Join-Path $env:TEMP "touch-$transportName-$backendName.png"
                    $pixels=[RemoteTouchProbe]::Pixels($renderer.Id,$picturePath)
                    if($pixels[0] -gt 1000 -and $pixels[1] -gt 20) {Write-Output "PASS $transportName $backendName rectangle and contact pixels: $picturePath"}
                    else {Write-Output "UNVERIFIED $transportName $backendName desktop capture has sky=$($pixels[0]), markers=$($pixels[1]); inspect $picturePath"}
                }
                $xx=$x+30;$yy=$y+20
                IO $endpoint "!PointUpdate:10,$xx,$yy"
                Wait-For {[Math]::Abs((State)[0]-$initial[0]-30) -lt 0.01} 'one-contact translation'
                IO $endpoint "!PointDown:20,$($xx+100),$yy"
                Wait-For {(State)[4] -eq 2} 'second contact'
                IO $endpoint "!PointUpdate:20,$xx,$($yy+160)"
                Wait-For {[Math]::Abs((State)[3]-$initial[2]*1.6) -lt 0.01} 'rotation and scaling'
                $transformed=(State)[0..3]
                IO $endpoint "!PointDown:30,$($xx+40),$($yy+40)"
                IO $endpoint "!PointUpdate:30,$($xx+80),$($yy+80)"
                Wait-For {(State)[4] -eq 3} 'three contacts'
                if(((State)[0..3] -join ',') -ne ($transformed -join ',')){throw 'Three-contact transform did not pause.'}
                IO $endpoint "!PointUp:10,$xx,$yy"
                Wait-For {(State)[4] -eq 2} 'sparse contact removal'
                if(((State)[0..3] -join ',') -ne ($transformed -join ',')){throw 'Membership change jumped.'}
                IO $endpoint '!PointCancel:20,CaptureLost'
                IO $endpoint '!PointCancel:30,PlatformCanceled'
                Wait-For {(State)[4] -eq 0} 'cancellation marker cleanup'
                IO $endpoint "!PointDown:10,$x,$y"
                IO $endpoint "!PointUp:10,$x,$y"
                Wait-For {(State)[4] -eq 0} 'fresh sequence'
                Write-Output "PASS $transportName $backendName $endpoint typed touch routing and gestures"
            }
            Wait-For {
                $desc=@(Nodes (Invoke-RestMethod "$rendererBase/Dom" -TimeoutSec 10) | Where-Object rawId -eq 'FullControlTest.Touching')[0]
                $desc.data -eq (Find-Raw).elementData
            } 'renderer receives current Touching state'
            Select-Tab 'Raw Clock'
            Wait-For {@(Controls | Where-Object elementId -eq 'FullControlTest.Clock').Count -gt 0} 'clock painter replacement'
            $tab=@(Controls | Where-Object elementText -eq 'Touching')[0]
            $tx=[int](($tab.bounds.x1+$tab.bounds.x2)/2);$ty=[int](($tab.bounds.y1+$tab.bounds.y2)/2)
            IO $rendererBase "!PointDown:1,$tx,$ty"
            IO $rendererBase "!PointUp:1,$tx,$ty"
            Wait-For {$null -ne (Find-Raw)} 'rejected touch tab fallback'
            if($NativeInjection) { & (Join-Path $PSScriptRoot 'Touching_NativeInput.ps1') -TargetProcessId $renderer.Id -Application RemotingTest_Rendering_Native -AsPort 8889 -CoreAsPort 8888 }
            IO $rendererBase '!Exit'
            Wait-For {$core.HasExited -and $renderer.HasExited} 'graceful exit'
            Write-Output "PASS $transportName $backendName renderer state, fallback and shutdown"
        } finally {
            foreach($process in $owned) {if(!$process.HasExited){Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue}}
        }
    }}
} finally {
    [IO.File]::WriteAllBytes($coreUser,$coreOriginal)
    [IO.File]::WriteAllBytes($rendererUser,$rendererOriginal)
}
