param(
    [ValidateRange(1,65535)][int]$AsPort = 8888,
    [string]$Command,
    [int]$Window = 0,
    [string]$Find = '*',
    [string]$ClickLabel,
    [int]$Occurrence = 0,
    [switch]$DoubleClick,
    [switch]$RightClick,
    [switch]$InspectNode,
    [int]$SelectProcessId,
    [switch]$Scroll,
    [switch]$Button,
    [switch]$Dump
)

$ErrorActionPreference = 'Stop'
$endpoint = "http://localhost:$AsPort/Automation/UiaListApp"
Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class InspectorNativeWindows
{
    public delegate bool Callback(IntPtr hwnd, IntPtr state);
    [DllImport("user32.dll")] public static extern bool EnumWindows(Callback callback, IntPtr state);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint pid);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr hwnd, StringBuilder text, int length);
}
'@

function Assert-InspectorAlive {
    $processes = @(Get-Process UiaListApp -ErrorAction SilentlyContinue)
    if ($processes.Count -ne 1) { throw "Expected one UiaListApp process; found $($processes.Count)." }
    $script:crashed = $false
    $inspectorPid = $processes[0].Id
    [InspectorNativeWindows]::EnumWindows({
        param($handle, $state)
        [uint32]$windowPid = 0
        [void][InspectorNativeWindows]::GetWindowThreadProcessId($handle, [ref]$windowPid)
        if ($windowPid -eq $inspectorPid) {
            $title = [Text.StringBuilder]::new(1024)
            [void][InspectorNativeWindows]::GetWindowText($handle, $title, $title.Capacity)
            if ($title.ToString() -eq 'Microsoft Visual C++ Runtime Library') { $script:crashed = $true }
        }
        return $true
    }, [IntPtr]::Zero) | Out-Null
    if ($script:crashed) { throw "UiaListApp PID $inspectorPid has a blocking runtime error dialog." }
}

function Get-TextNodes($node) {
    $pending = [Collections.Generic.Stack[object]]::new()
    $result = [Collections.Generic.List[object]]::new()
    $pending.Push(@{Node=$node; Control=''; Clip=$node.bounds; Scroll=$null})
    while ($pending.Count) {
        $entry = $pending.Pop()
        $current = $entry.Node
        $control = if ($current.control) { $current.control } else { $entry.Control }
        $clip = @{x1=[Math]::Max($entry.Clip.x1,$current.bounds.x1); y1=[Math]::Max($entry.Clip.y1,$current.bounds.y1); x2=[Math]::Min($entry.Clip.x2,$current.bounds.x2); y2=[Math]::Min($entry.Clip.y2,$current.bounds.y2)}
        $scrollBounds = if ($current.control -in @('ScrollView','TreeView','ListView','TextList','DataGrid')) { $clip } else { $entry.Scroll }
        if ($current.elementText) { $result.Add([pscustomobject]@{ Text = $current.elementText; Bounds = $current.bounds; VisibleBounds=$clip; ScrollBounds=$scrollBounds; Control=$control }) }
        for ($index = $current.children.Count - 1; $index -ge 0; $index--) { $pending.Push(@{Node=$current.children[$index]; Control=$control; Clip=$clip; Scroll=$scrollBounds}) }
    }
    return $result.ToArray()
}

Assert-InspectorAlive
try { $state = Invoke-RestMethod -Uri "$endpoint/Controls" -TimeoutSec 10 }
catch { Assert-InspectorAlive; throw }
if ($SelectProcessId) {
    $pending = [Collections.Generic.Stack[object]]::new(); $pending.Push($state.MainWindow.composition)
    $combo = $null
    while ($pending.Count) { $node=$pending.Pop(); if ($node.control -eq 'ComboBox') { $combo=$node; break }; foreach ($child in $node.children) { $pending.Push($child) } }
    if (!$combo) { throw 'Process combo box was not found. Select Processes first.' }
    $point = '!LeftClick:{0},{1}' -f [int](($combo.bounds.x1+$combo.bounds.x2)/2),[int](($combo.bounds.y1+$combo.bounds.y2)/2)
    $reply=(Invoke-WebRequest -Method Post -Uri "$endpoint/IO/$($state.MainWindow.windowId)" -Headers @{'Content-Type'='application/json; charset=utf8'} -SkipHeaderValidation -Body ([Text.Encoding]::UTF8.GetBytes($point)) -TimeoutSec 10).Content
    if ($reply -ne 'Queued') { throw $reply }
    $state=Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15
    if (@($state.Popups).Count -ne 1) { throw 'Process dropdown did not open.' }
    $deadline=[DateTime]::UtcNow.AddSeconds(5)
    while (!(Get-TextNodes $state.Popups[0].composition).Count -and [DateTime]::UtcNow -lt $deadline) {
        Start-Sleep -Milliseconds 50
        $state=Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15
    }
    $Window=1+@($state.SubWindows).Count
    $ClickLabel='*' + [WildcardPattern]::Escape("[$SelectProcessId]")
    $Scroll=$true
}
$windows = @($state.MainWindow) + @($state.SubWindows) + @($state.Popups)
if ($Window -gt 0) {
    $deadline=[DateTime]::UtcNow.AddSeconds(5)
    while ($Window -ge $windows.Count -and [DateTime]::UtcNow -lt $deadline) {
        Start-Sleep -Milliseconds 50
        $state=Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15
        $windows=@($state.MainWindow)+@($state.SubWindows)+@($state.Popups)
    }
}
if ($Window -lt 0 -or $Window -ge $windows.Count) { throw "Window index $Window is out of range." }
$target = $windows[$Window]
$nodes = @(Get-TextNodes $target.composition)
if ($ClickLabel) {
    if ($Scroll) {
        $url = "$endpoint/IO/$($target.windowId)"
        function Send-ScrollCommand([string]$text) {
            $response = (Invoke-WebRequest -Method Post -Uri $url -Headers @{'Content-Type'='application/json; charset=utf8'} -SkipHeaderValidation -Body ([Text.Encoding]::UTF8.GetBytes($text)) -TimeoutSec 10).Content
            if ($response -ne 'Queued') { throw $response }
        }
        for ($attempt = 0; $attempt -lt 240; $attempt++) {
            $matches = @($nodes | Where-Object { $_.Text -like $ClickLabel -and (!$Button -or $_.Control -eq 'Button') })
            if ($Occurrence -ge $matches.Count) {
                # Advance less than one viewport so virtualized rows cannot be skipped.
                $surface=@($nodes | Where-Object ScrollBounds | Select-Object -First 1)[0].ScrollBounds
                if (!$surface) { throw "No scroll surface for '$ClickLabel'." }
                $delta = if ($attempt -eq 0) { -48000 } else { [Math]::Max(48,($surface.y2-$surface.y1)/2) }
            } else {
                $bounds = $matches[$Occurrence].Bounds
                $visible=$matches[$Occurrence].VisibleBounds
                if ($visible.y2 -gt $visible.y1 -and $visible.x2 -gt $visible.x1 -and $visible.y1 -eq $bounds.y1 -and $visible.y2 -eq $bounds.y2) { break }
                $surface=$matches[$Occurrence].ScrollBounds
                if (!$surface) { throw "Caption '$ClickLabel' is outside a scroll surface." }
                $delta = [int](($bounds.y1 + $bounds.y2 - $surface.y1 - $surface.y2) / 2)
            }
            $ticks = [Math]::Max(1, [Math]::Floor([Math]::Abs($delta) / 48))
            Send-ScrollCommand ('!MouseMove:{0},{1}' -f [int](($surface.x1+$surface.x2)/2),[int](($surface.y1+$surface.y2)/2))
            Send-ScrollCommand "!MouseWheel$(if ($delta -gt 0) {'Down'} else {'Up'}):$ticks"
            Start-Sleep -Milliseconds 150
            $state = Invoke-RestMethod -Uri "$endpoint/Controls" -TimeoutSec 10
            $windows = @($state.MainWindow) + @($state.SubWindows) + @($state.Popups)
            $target = $windows[$Window]
            $nodes = @(Get-TextNodes $target.composition)
        }
    }
    $matches = @($nodes | Where-Object { $_.Text -like $ClickLabel -and (!$Button -or $_.Control -eq 'Button') })
    if ($Occurrence -ge $matches.Count) { throw "Caption '$ClickLabel' occurrence $Occurrence is not visible." }
    $bounds = $matches[$Occurrence].VisibleBounds
    $x = [int](($bounds.x1 + $bounds.x2) / 2)
    $y = [int](($bounds.y1 + $bounds.y2) / 2)
    if ($bounds.y2 -le $bounds.y1 -or $bounds.x2 -le $bounds.x1 -or $x -lt 0 -or $y -lt 0 -or $x -ge $target.composition.bounds.x2 -or $y -ge $target.composition.bounds.y2) { throw "Caption '$ClickLabel' is clipped. Scroll it into view before clicking." }
    $verb = if ($RightClick -or $InspectNode) { '!RightClick' } elseif ($DoubleClick) { '!LeftDbClick' } else { '!LeftClick' }
    $Command = "${verb}:$x,$y"
}
if ($Command) {
    $url = if ($Window -eq 0) { "$endpoint/IO" } else { "$endpoint/IO/$($target.windowId)" }
    $response = (Invoke-WebRequest -Method Post -Uri $url -Headers @{'Content-Type'='application/json; charset=utf8'} -SkipHeaderValidation -Body ([Text.Encoding]::UTF8.GetBytes($Command)) -TimeoutSec 10).Content
    if ($response -ne 'Queued') { throw "Command rejected: $response" }
    Write-Output "Queued $Command for window $Window. Read Controls to verify completion."
    if ($InspectNode) {
        $state=Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15
        $menus=@($state.Popups | Where-Object { @(Get-TextNodes $_.composition | Where-Object Text -EQ 'Inspect').Count })
        $deadline=[DateTime]::UtcNow.AddSeconds(5)
        while ($menus.Count -ne 1 -and [DateTime]::UtcNow -lt $deadline) {
            Start-Sleep -Milliseconds 50
            $state=Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15
            $menus=@($state.Popups | Where-Object { @(Get-TextNodes $_.composition | Where-Object Text -EQ 'Inspect').Count })
        }
        if ($menus.Count -ne 1) { throw 'Node Inspect menu did not open.' }
        $item=@(Get-TextNodes $menus[0].composition | Where-Object Text -EQ 'Inspect')[0].VisibleBounds
        $point='!LeftClick:{0},{1}' -f [int](($item.x1+$item.x2)/2),[int](($item.y1+$item.y2)/2)
        $reply=(Invoke-WebRequest -Method Post -Uri "$endpoint/IO/$($menus[0].windowId)" -Headers @{'Content-Type'='application/json; charset=utf8'} -SkipHeaderValidation -Body ([Text.Encoding]::UTF8.GetBytes($point)) -TimeoutSec 10).Content
        if ($reply -ne 'Queued') { throw $reply }
        $null=Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15
    }
}
if ($Dump -or !$Command) {
    for ($index = 0; $index -lt $windows.Count; $index++) {
        [pscustomobject]@{ Window = $index; Title = $windows[$index].title; Id = $windows[$index].windowId } | ConvertTo-Json -Compress
    }
    $nodes | Where-Object { $_.Text -like $Find } | ForEach-Object { $_ | ConvertTo-Json -Depth 4 -Compress }
}
