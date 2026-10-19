#requires -PSEdition Desktop
param(
    [ValidateSet('Pipe','Http','MiniHttp')][string[]]$Transport = @('Pipe','Http','MiniHttp'),
    [ValidateSet('Direct2D','GDI')][string[]]$Backend = @('Direct2D','GDI'),
    [switch]$NativeInjection,
    [switch]$Browser
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
function Test-SwipeAndText {
    Select-Tab 'TouchingList'
    Wait-For { @(Controls | Where-Object elementText -eq 'Add 5 Items').Count -eq 1 } 'TouchingList page'
    Select-Tab 'Clear'
    Wait-For { @(Controls | Where-Object elementText -like 'Item *').Count -eq 0 } 'Clear populated list'
    Select-Tab 'Clear'
    Select-Tab 'Add 5 Items'
    Select-Tab 'Add 5 Items'
    Wait-For { @(Controls | Where-Object elementText -like 'Item 7 *').Count -eq 1 } 'new middle row'
    $row = @(Controls | Where-Object elementText -like 'Item 7 *')[0]
    $name = $row.elementText
    $left = [int]$row.bounds.x1
    $x = $left + 180
    $y = [int]$row.bounds.y1 + 25
    $label = @(Controls | Where-Object { $_.elementText -eq 'DELETE IT' -and $_.bounds.y1 -le $row.bounds.y1 -and $_.bounds.y2 -ge $row.bounds.y2 })[0]
    $maximum = [int]($label.bounds.x2 - $label.bounds.x1 + 32)
    if ($maximum -le 32) { throw 'Missing measured deletion label.' }
    function Assert-Reveal($distance) {
        Wait-For { [Math]::Abs(@(Controls | Where-Object elementText -eq $name)[0].bounds.x1 - ($left - $distance)) -le 1 } "reveal $distance"
    }
    IO $rendererBase "!PointDown:101,$x,$y"
    IO $rendererBase "!PointUpdate:101,$($x+50),$y"
    Assert-Reveal 0
    IO $rendererBase "!PointUp:101,$($x+50),$y"
    $half = [int][Math]::Floor($maximum / 2)
    IO $rendererBase "!PointDown:102,$x,$y"
    IO $rendererBase "!PointUpdate:102,$($x-$half),$y"
    Assert-Reveal $half
    IO $rendererBase "!PointUpdate:102,$($x-$half+15),$y"
    Assert-Reveal ($half-15)
    IO $rendererBase "!PointUp:102,$($x-$half+15),$y"
    Assert-Reveal 0
    IO $rendererBase "!PointDown:103,$x,$y"
    IO $rendererBase "!PointUpdate:103,$($x-$maximum-50),$y"
    Assert-Reveal $maximum
    IO $rendererBase "!PointDown:104,$($x+20),$y"
    Assert-Reveal 0
    IO $rendererBase "!PointUp:104,$($x+20),$y"
    IO $rendererBase "!PointUp:103,$($x-$maximum-50),$y"
    Assert-Reveal 0
    IO $rendererBase "!PointDown:105,$x,$y"
    IO $rendererBase "!PointUpdate:105,$($x-$maximum),$y"
    Assert-Reveal $maximum
    IO $rendererBase '!PointCancel:105,PlatformCanceled'
    Assert-Reveal 0
    IO $rendererBase "!PointUp:105,$($x-$maximum),$y"
    IO $rendererBase "!PointDown:106,$x,$y"
    IO $rendererBase "!PointUp:106,$($x-$maximum),$y"
    Wait-For { @(Controls | Where-Object elementText -eq $name).Count -eq 0 } 'deferred middle row removal'
    $next = @(Controls | Where-Object elementText -like 'Item 8 *')[0]
    if ([Math]::Abs($next.bounds.y1 - $row.bounds.y1) -gt 1 -or $next.bounds.x1 -ne $left) { throw 'Replacement row inherited a stale index or reveal.' }
    IO $rendererBase "!PointDown:107,$x,$y"
    IO $rendererBase "!PointUpdate:107,$x,$($y-80)"
    IO $rendererBase "!PointUp:107,$x,$($y-80)"
    Wait-For { @(Controls | Where-Object elementText -like 'Item 8 *')[0].bounds.y1 -lt $next.bounds.y1 } 'built-in vertical list scrolling'
    IO $rendererBase "!PointDown:108,$x,$y"
    IO $rendererBase "!PointUpdate:108,$x,$($y+1000)"
    IO $rendererBase "!PointUp:108,$x,$($y+1000)"
    Wait-For { @(Controls | Where-Object elementText -like 'Item 8 *')[0].bounds.x1 -eq $left } 'recycled rows reset'
    Select-Tab 'Clear'
    Wait-For { @(Controls | Where-Object elementText -like 'Item *').Count -eq 0 } 'Clear after deletion'
    IO $rendererBase "!PointDown:109,$x,$y"
    IO $rendererBase "!PointUp:109,$($x-$maximum),$y"
    Select-Tab 'Add 5 Items'
    Wait-For { @(Controls | Where-Object elementText -like 'Item 16 *').Count -eq 1 } 'empty-space swipe cannot remove a replacement item'
    Write-Output 'PASS renderer IO / TouchingList partial, reversal, cancellation, extra contact, final Up, identity removal and scrolling'

    Select-Tab 'List'
    Wait-For { @(Controls | Where-Object elementText -eq 'ListView').Count -gt 0 } 'List page'
    Select-Tab 'ListView'
    Wait-For { @(Controls | Where-Object control -eq 'ListView').Count -gt 0 } 'ListView page'
    $list = @(Controls | Where-Object control -eq 'ListView')[0]
    $task = @(Nodes $list | Where-Object elementText -eq 'Task')[0]
    $tx=[int]$task.bounds.x1+8; $ty=[int]$task.bounds.y1+8
    IO $rendererBase "!PointDown:120,$tx,$ty"
    IO $rendererBase "!PointUp:120,$tx,$ty"
    function List-Rendering {
        $current = @(Controls | Where-Object control -eq 'ListView')[0]
        (Nodes $current | Where-Object element | ForEach-Object { "$($_.element)|$($_.elementText)" }) -join "`n"
    }
    $beforeRendering = List-Rendering
    $lx=[int]$list.bounds.x1+100; $ly=[int]$list.bounds.y1+150
    IO $rendererBase "!PointDown:121,$lx,$ly"
    IO $rendererBase "!PointUpdate:121,$lx,$($ly-60)"
    Wait-For {
        $current = @(Nodes (@(Controls | Where-Object control -eq 'ListView')[0]) | Where-Object elementText -eq 'Task')
        $current.Count -eq 0 -or $current[0].bounds.y1 -lt $task.bounds.y1
    } 'ordinary ListView scrolls'
    IO $rendererBase "!PointUpdate:121,$lx,$ly"
    IO $rendererBase "!PointUp:121,$lx,$ly"
    Wait-For { (List-Rendering) -eq $beforeRendering } 'ListView reversal preserves selection rendering without activation'
    Write-Output 'PASS renderer IO / ordinary ListView scrolling and selection rendering'

    for ($pass=0; $pass -lt 2; $pass++) {
        Select-Tab 'Control'
        Wait-For { @(Controls | Where-Object elementText -eq 'TextBox').Count -gt 0 } 'Control page'
        Select-Tab 'TextBox'
        Wait-For { @(Controls | Where-Object elementDocument -like '*I am the bone*').Count -gt 0 } 'multiline content'
        $document = @(Controls | Where-Object elementDocument -like '*I am the bone*')[0]
        $dx=[int]$document.bounds.x1+8; $dy=[int]$document.bounds.y1+8
        IO $rendererBase "!PointDown:110,$dx,$dy"
        IO $rendererBase "!PointUpdate:110,$($dx+100),$($dy+25)"
        Wait-For {
            $selection = @(Controls | Where-Object elementDocument -like '*I am the bone*')[0].element
            $selection -match 'Selection(\([^)]*\))-(\([^)]*\))' -and $Matches[1] -ne $Matches[2]
        } 'nonempty touch text selection'
        $selected = @(Controls | Where-Object elementDocument -like '*I am the bone*')[0].element
        IO $rendererBase "!PointUpdate:110,$($dx+180),$($dy+50)"
        Wait-For { @(Controls | Where-Object elementDocument -like '*I am the bone*')[0].element -ne $selected } 'changing touch text selection'
        IO $rendererBase "!PointUp:110,$($dx+180),$($dy+50)"
        if ($pass -eq 0) {
            Select-Tab 'Window Manager'
            Wait-For { @(Controls | Where-Object elementText -eq 'Aurora').Count -gt 0 } 'palette controls'
            Select-Tab 'Aurora'
            $null = Controls
        }
    }
    Write-Output 'PASS renderer IO / changing text selection before and after template replacement'
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
            if ($Browser) {
                if ($transportName -eq 'Pipe') { throw 'Browser touch requires Http or MiniHttp.' }
                & node (Join-Path $PSScriptRoot 'Touching_Browser.mjs') $transportName
                if ($LASTEXITCODE -ne 0) { throw 'Browser touch verification failed.' }
                IO $coreBase '!Exit'
                Wait-For { $core.HasExited } 'browser Core shutdown'
                Write-Output "PASS $transportName Chromium CDP touch and Core shutdown"
                continue
            }
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
            Test-SwipeAndText
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
