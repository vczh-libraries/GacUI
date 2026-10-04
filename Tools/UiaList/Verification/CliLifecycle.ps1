#requires -Version 7.0
param([Parameter(Mandatory)][int]$FixtureProcessId)
. (Join-Path $PSScriptRoot 'CliCommon.ps1')
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class CliFixtureWindows {
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr window, out uint process);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr window, int mode);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr window, uint message, UIntPtr w, IntPtr l);
    [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr window);
}
'@

function Get-OwnedHandle($Window) {
    $handle = [IntPtr]::new([Convert]::ToInt64($Window.hwnd.Substring(2), 16))
    [uint32]$owner = 0
    [void][CliFixtureWindows]::GetWindowThreadProcessId($handle, [ref]$owner)
    Assert-Uia ($owner -eq $FixtureProcessId) "Only manipulate this run's fixture window"
    return $handle
}

$client = New-UiaClient
$initial = Get-UiaFixtureWindows $client $FixtureProcessId
$native = $initial | Where-Object class -EQ 'UiaFixture' | Select-Object -First 1
$tree = $client.Send('Print-Window', $native.id).result
foreach ($name in @('Open large/deep raw tree','Open navigation windows')) {
    $node = Get-UiaNode $tree $name
    $tree = $client.Send('Run-IInvokeProvider::Invoke', $node.id).result.tree
}
$discovered = Get-UiaFixtureWindows $client $FixtureProcessId
$large = $discovered | Where-Object title -Like '*10,000 siblings*' | Select-Object -First 1
$loaded = $client.Send('Print-Window', $large.id)
$tree = $loaded.result
$before = @{}
foreach ($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter "UiaFixture-$FixtureProcessId.*.txt") {
    $before[$file.FullName] = Read-UiaFixtureLog $file.FullName -AsBytes
}
$preview = $client.Send('Query-Preview', $large.id)
Assert-UiaEqual $preview.generation $loaded.generation
$capture = $preview.result
if ($capture.status -eq 'Ready') {
    $bitmap = [Convert]::FromBase64String($capture.base64)
    Assert-Uia ($bitmap[0] -eq 0x42 -and $bitmap[1] -eq 0x4D) 'BMP signature'
    $width = [BitConverter]::ToInt32($bitmap, 18)
    $height = [BitConverter]::ToInt32($bitmap, 22)
    Assert-Uia ($width -eq $capture.width -and [Math]::Abs($height) -eq $capture.height) 'BMP dimensions'
    Assert-Uia ($capture.dpi -gt 0 -and $capture.timestamp) 'Capture DPI and timestamp'
    $deepest = $tree.nodes | Where-Object name -Like 'Stress node 11045 *' | Select-Object -First 1
    $excluded = $tree.nodes | Where-Object name -Like 'Stress node 11046 *' | Select-Object -First 1
    Assert-Uia ($excluded.offscreen) 'Offscreen stress node'
    Assert-UiaEqual $deepest.bounds $excluded.bounds 'Overlapping bounds'
    $ancestor = Get-UiaNode $tree '1,000 levels'
    $bounds = $deepest.bounds
    Assert-Uia ($bounds.bottom -lt $ancestor.bounds.top) 'Deep child outside ancestor'
    $point = @{x=$bounds.left + 5 - $capture.bounds.left; y=$bounds.top + 5 - $capture.bounds.top}
    $hit = $client.Send('HitTest-Preview', $large.id, $point)
    Assert-UiaEqual $hit.generation $loaded.generation
    Assert-UiaEqual $hit.result @{id=$deepest.id;bounds=$bounds} 'Deepest eligible hit'
    Assert-UiaEqual ($client.Send('HitTest-Preview', $large.id, @{x=-1;y=-1}).result.id) $null
    Write-Host 'PASS BMP dimensions/DPI/generation and deep/overlapping/outside-parent/offscreen hit tests'
} else {
    Assert-UiaEqual $capture.base64 $null
    Write-Host 'LIMIT capture unavailable; successful pixel/hit coverage not claimed'
}
foreach ($path in $before.Keys) {
    Assert-UiaEqual (Read-UiaFixtureLog $path -AsBytes) $before[$path] 'Preview sent target input'
}

$synthetic = $discovered | Where-Object { $_.class -eq 'UiaSyntheticFixture' -and $_.title -notlike '*10,000*' } | Select-Object -First 1
$selected = $client.Send('Print-Window', $synthetic.id)
$current = Get-UiaFixtureWindows $client $FixtureProcessId
foreach ($old in $discovered) {
    $surviving = $current | Where-Object hwnd -CEQ $old.hwnd | Select-Object -First 1
    Assert-UiaEqual $surviving.id $old.id 'Surviving window identity'
}
Assert-UiaEqual ($client.Send('Print-Window', $synthetic.id).generation) $selected.generation
$handle = Get-OwnedHandle $synthetic
[void][CliFixtureWindows]::ShowWindow($handle, 6)
try {
    $refreshed = $client.Send('Refresh-Window', $synthetic.id)
    $unavailable = $client.Send('Query-Preview', $synthetic.id).result
    Assert-Uia ($unavailable.status -eq 'Unavailable' -and $null -eq $unavailable.base64) 'Minimized capture unavailable'
    $capability = Get-UiaNode $refreshed.result 'Synthetic capabilities'
    Assert-Uia ($client.Send('Query-Properties', $capability.id).result.properties.Count -gt 0) 'Properties remain available'
} finally {
    [void][CliFixtureWindows]::ShowWindow($handle, 4) # Restore without activation.
}

$navigation = $current | Where-Object { $_.class -eq 'UiaNavigationFixture' -and $_.title -eq 'Duplicate navigation title' } | Select-Object -First 1
$loaded = $client.Send('Print-Window', $navigation.id)
$stale = $loaded.result.nodes[0].id
$handle = Get-OwnedHandle $navigation
Assert-Uia ([CliFixtureWindows]::PostMessage($handle, 0x10, [UIntPtr]::Zero, [IntPtr]::Zero)) 'Close fixture navigation window'
$deadline = [DateTime]::UtcNow.AddSeconds(5)
while ([CliFixtureWindows]::IsWindow($handle) -and [DateTime]::UtcNow -lt $deadline) { Start-Sleep -Milliseconds 20 }
Assert-Uia (![CliFixtureWindows]::IsWindow($handle)) 'Navigation window closed'
Assert-UiaEqual ($client.Send('Refresh-Window', $navigation.id, $null, $false).error.code) 'ExpiredId'
Assert-Uia (@((Get-UiaFixtureWindows $client $FixtureProcessId) | Where-Object id -CEQ $navigation.id).Count -eq 0) 'Removed window omitted'
Assert-UiaEqual ($client.Send('Query-Node', $stale, $null, $false).error.code) 'ExpiredId'
Assert-UiaEqual ($client.Send('Help-Provider').result.currentTarget) @()
$null = $client.Send('Print-Window', $synthetic.id)
$client.Close()
Write-Host 'PASS surviving and removed identities; unavailable capture leaves inspection usable'
