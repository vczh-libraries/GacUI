param(
    [ValidateRange(1,65535)][int]$AsPort = 8891,
    [Parameter(Mandatory=$true)][int]$FixtureProcessId
)

$ErrorActionPreference = 'Stop'
# Reuse the established smoke driver's functions without its launch/test loop.
# Keep this extraction guarded so a changed driver cannot silently run that loop.
$smoke = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'Smoke.ps1') -Raw
$boundary = $smoke.IndexOf('$userFile =')
if ($boundary -lt 0) { throw 'Smoke driver helper boundary changed.' }
$definitions = $smoke.Substring(0, $boundary).Replace('$PSScriptRoot', "'$PSScriptRoot'")
. ([scriptblock]::Create($definitions)) -AsPort $AsPort -FixtureProcessId $FixtureProcessId
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class FailureDialogWindows {
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr window, uint message, IntPtr w, IntPtr l);
}
'@

$inspector = Get-Process UiaListApp
$inspectorHandle = $inspector.Handle
. $driver -AsPort $AsPort -Find '<no caption>' | Out-Null
Wait-Ready 0 -AllowNoSelection
& $driver -AsPort $AsPort -SelectProcessId $FixtureProcessId | Out-Null
Click '*synthetic providers*'
Wait-Ready 0
Click Nodes
Inspect 'Synthetic capabilities*'
Wait-Ready 1
Click Actions 1

$field = Getter-Field 'GetItemByName' 'name'
$before = Synthetic-Count 'GetItemByName'
Edit-Getter $field '__uia_unavailable__'
Wait-Getter 'GetItemByName' '*80040201*'
Wait-Ready 1
if ((Synthetic-Count 'GetItemByName') -ne $before + 1) { throw 'Expected failure was retried.' }
Write-Output 'PASS GUI expected UIA failure remains visible and responsive'

$field = Getter-Field 'GetItemByName' 'name'
Edit-Getter $field '__uia_fatal__'
# The GUI intentionally presents a native fatal dialog. Do not poll the blocked
# GacUI Controls endpoint; inspect only windows owned by this inspector process.
$deadline = [DateTime]::UtcNow.AddSeconds(15)
$dialog = $null
do {
    $dialog = @([FixtureWindows]::List($inspector.Id) | Where-Object { $_.Class -eq '#32770' -and $_.Title -eq 'UiaList' }) | Select-Object -First 1
    if ($dialog) { break }
    if ($inspector.HasExited) { throw 'GUI exited before presenting its fatal diagnostic.' }
    Start-Sleep -Milliseconds 100
} while ([DateTime]::UtcNow -lt $deadline)
if (!$dialog) { throw 'GUI fatal diagnostic did not appear.' }
$okButton = [FixtureWindows]::GetDlgItem($dialog.Handle, 1)
if ($okButton -eq [IntPtr]::Zero) { $okButton = [FixtureWindows]::GetDlgItem($dialog.Handle, 2) }
if ($okButton -eq [IntPtr]::Zero) { throw 'Fatal dialog has no OK button.' }
if (![FailureDialogWindows]::PostMessage($okButton, 0xF5, [IntPtr]::Zero, [IntPtr]::Zero)) { throw 'Cannot dismiss fatal dialog.' }
if (!$inspector.WaitForExit(15000)) { throw 'GUI did not exit after its fatal diagnostic.' }
# Debug Direct2D may raise a breakpoint while ExitProcess tears down its live
# graphics objects. The preserved GUI fatal contract requires a nonzero exit.
if ($inspector.ExitCode -eq 0) { throw 'GUI reported success after a fatal provider failure.' }
if ((Synthetic-Count 'GetItemByName') -ne $before + 2) { throw 'Fatal getter was retried.' }
Write-Output "PASS GUI native fatal diagnostic, exact call count and nonzero exit $($inspector.ExitCode)"
