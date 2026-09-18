param(
    [ValidateRange(1,65535)][int]$AsPort = 8888,
    [string]$Command,
    [int]$Window = 0,
    [string]$Find = '*',
    [string]$ClickLabel,
    [int]$Occurrence = 0,
    [switch]$DoubleClick,
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
    $pending.Push(@{Node=$node; Control=''})
    while ($pending.Count) {
        $entry = $pending.Pop()
        $current = $entry.Node
        $control = if ($current.control) { $current.control } else { $entry.Control }
        if ($current.elementText) { $result.Add([pscustomobject]@{ Text = $current.elementText; Bounds = $current.bounds; Control=$control }) }
        for ($index = $current.children.Count - 1; $index -ge 0; $index--) { $pending.Push(@{Node=$current.children[$index]; Control=$control}) }
    }
    return $result.ToArray()
}

Assert-InspectorAlive
try { $state = Invoke-RestMethod -Uri "$endpoint/Controls" -TimeoutSec 10 }
catch { Assert-InspectorAlive; throw }
$windows = @($state.MainWindow) + @($state.SubWindows) + @($state.Popups)
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
                if ($Window -ne 0) { throw "Caption '$ClickLabel' was not found." }
                # Advance less than one viewport so virtualized rows cannot be skipped.
                $delta = 192
            } else {
                $bounds = $matches[$Occurrence].Bounds
                $top = if ($Window -eq 0) { 0 } else { 142 }
                $bottom = if ($Window -eq 0) { $target.composition.bounds.y2 - 60 } else { 594 }
                if ($bounds.y1 -ge $top -and $bounds.y2 -le $bottom -and $bounds.y2 -gt $bounds.y1) { break }
                $delta = [int](($bounds.y1 + $bounds.y2) / 2) - 360
            }
            $ticks = [Math]::Max(1, [Math]::Floor([Math]::Abs($delta) / 48))
            Send-ScrollCommand '!MouseMove:600,400'
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
    $bounds = $matches[$Occurrence].Bounds
    $x = [int](($bounds.x1 + $bounds.x2) / 2)
    $y = [int](($bounds.y1 + $bounds.y2) / 2)
    if ($bounds.y2 -le $bounds.y1 -or $bounds.x2 -le $bounds.x1 -or $x -lt 0 -or $y -lt 0 -or $x -ge $target.composition.bounds.x2 -or $y -ge $target.composition.bounds.y2) { throw "Caption '$ClickLabel' is clipped. Scroll it into view before clicking." }
    $verb = if ($DoubleClick) { '!LeftDbClick' } else { '!LeftClick' }
    $Command = "${verb}:$x,$y"
}
if ($Command) {
    $url = if ($Window -eq 0) { "$endpoint/IO" } else { "$endpoint/IO/$($target.windowId)" }
    $response = (Invoke-WebRequest -Method Post -Uri $url -Headers @{'Content-Type'='application/json; charset=utf8'} -SkipHeaderValidation -Body ([Text.Encoding]::UTF8.GetBytes($Command)) -TimeoutSec 10).Content
    if ($response -ne 'Queued') { throw "Command rejected: $response" }
    Write-Output "Queued $Command for window $Window. Read Controls to verify completion."
}
if ($Dump -or !$Command) {
    for ($index = 0; $index -lt $windows.Count; $index++) {
        [pscustomobject]@{ Window = $index; Title = $windows[$index].title; Id = $windows[$index].windowId } | ConvertTo-Json -Compress
    }
    $nodes | Where-Object { $_.Text -like $Find } | ForEach-Object { $_ | ConvertTo-Json -Depth 4 -Compress }
}
