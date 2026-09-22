param(
    [ValidateSet('CppTest','CppTest_Metaonly','Playground')][string]$Application,
    [ValidateRange(1,65535)][int]$AsPort,
    [int]$ClientProcessId = 0,
    [string]$BusyClick = '',
    [ValidateSet('All','List','Grid','Text','Refresh','Calendar','Layout','Walk','Window','Concurrent','Review','Review2','Review3','Transitions','Lifetime')][string]$Scenario = 'All',
    [switch]$HostedFixture,
    [switch]$GdiFixture,
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$repository = Split-Path $PSScriptRoot -Parent
$solution = Join-Path $PSScriptRoot 'GacUISrc'
if ($ClientProcessId) {
    if ($Scenario -eq 'Lifetime') {
        Add-Type -Path (Join-Path $PSScriptRoot 'UIA_Lifetime.cs')
        try { [GacUILifetimeTests]::Run($ClientProcessId, $AsPort, [bool]$HostedFixture) }
        catch { [Console]::Error.WriteLine($_.Exception.ToString()); exit 1 }
        exit 0
    }
    Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes, WindowsBase
    Add-Type -Path (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.cs') -ReferencedAssemblies UIAutomationClient, UIAutomationTypes, WindowsBase
    [GacUIShowcaseTests]::BusyCommand = $BusyClick
    try { [GacUIShowcaseTests]::Run($ClientProcessId, ($Application -eq 'CppTest' -or $HostedFixture), $Scenario, $AsPort) }
    catch { [Console]::Error.WriteLine($_.Exception.ToString()); exit 1 }
    exit 0
}
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class ShowcaseWindows {
    delegate bool Callback(IntPtr window, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumWindows(Callback callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr window, out uint process);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr window, StringBuilder text, int length);
    public static bool Crashed(int target) {
        bool crashed = false;
        EnumWindows((window, parameter) => { uint process; GetWindowThreadProcessId(window, out process); if (process == target) { var title = new StringBuilder(512); GetWindowText(window, title, title.Capacity); if (title.ToString() == "Microsoft Visual C++ Runtime Library") crashed = true; } return true; }, IntPtr.Zero);
        return crashed;
    }
}
'@
function Assert-Target {
    if ($owned.HasExited) { throw "Target unexpectedly exited: $($owned.ExitCode)" }
    if ([ShowcaseWindows]::Crashed($owned.Id)) { throw "Target PID $($owned.Id) has a blocking native runtime error dialog." }
}
if (!$SkipBuild) {
    Write-Host "TEST $Application / build Debug x64"
    Push-Location $solution
    try { & (Join-Path $repository '.github/Scripts/copilotBuild.ps1') -Configuration Debug -Platform x64 }
    finally { Pop-Location }
    if ($LASTEXITCODE -ne 0) { throw 'Showcase build failed.' }
    $buildLog = Get-Content (Join-Path $repository '.github/Scripts/Build.log') -Raw
    if ($buildLog -notmatch 'Build succeeded\.') { throw 'Completed build log does not report success.' }
}

$userFile = Join-Path $solution "$Application/$Application.vcxproj.user"
$original = if (Test-Path -LiteralPath $userFile) { [IO.File]::ReadAllBytes($userFile) } else { $null }
$launcher = $null
$owned = $null
$client = $null
try {
    if ($original) { [xml]$settings = [Text.Encoding]::UTF8.GetString($original) }
    else { [xml]$settings = '<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" />' }
    # The execution wrapper reads this configuration; preserve all other user settings.
    $group = $settings.CreateElement('PropertyGroup', $settings.DocumentElement.NamespaceURI)
    $group.SetAttribute('Condition', "'`$(Configuration)|`$(Platform)'=='Debug|x64'")
    foreach ($existing in $settings.DocumentElement.ChildNodes) {
        if ($existing.Condition -eq $group.Condition) {
            foreach ($oldArgument in @($existing.ChildNodes | Where-Object LocalName -EQ 'LocalDebuggerCommandArguments')) { $null = $existing.RemoveChild($oldArgument) }
        }
    }
    $argument = $settings.CreateElement('LocalDebuggerCommandArguments', $settings.DocumentElement.NamespaceURI)
    $argument.InnerText = "/AsPort:$AsPort"
    if ($HostedFixture) { $argument.InnerText += ' /UiaHosted' }
    if ($GdiFixture) { $argument.InnerText += ' /UiaGdi' }
    $null = $group.AppendChild($argument)
    $null = $settings.DocumentElement.AppendChild($group)
    $settings.Save($userFile)
    $before = @(Get-Process $Application -ErrorAction SilentlyContinue | ForEach-Object Id)
    Write-Host "TEST $Application / launch /AsPort:$AsPort"
    $wrapper = Join-Path $repository '.github/Scripts/copilotExecute.ps1'
    $launcher = Start-Process powershell.exe -WindowStyle Hidden -WorkingDirectory $solution -ArgumentList @('-NoProfile','-File',"`"$wrapper`"",'-Mode','CLI','-Executable',$Application,'-Configuration','Debug','-Platform','x64') -PassThru
    # Windows PowerShell needs a retained handle to read ExitCode after exit.
    $launcherHandle = $launcher.Handle
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    do {
        if ($launcher.HasExited) { throw "Launcher exited early: $($launcher.ExitCode)" }
        $candidates = @(Get-Process $Application -ErrorAction SilentlyContinue | Where-Object Id -NotIn $before)
        if ($candidates.Count -eq 1) { $owned = $candidates[0]; break }
        Start-Sleep -Milliseconds 100
    } while ([DateTime]::UtcNow -lt $deadline)
    if (!$owned) { throw 'Could not uniquely identify the launched process.' }
    $endpoint = "http://localhost:$AsPort/Automation/$Application"
    do {
        Assert-Target
        try { $state = Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15; break }
        catch { Assert-Target; Start-Sleep -Milliseconds 100 }
    } while ([DateTime]::UtcNow -lt $deadline)
    if (!$state.MainWindow) { throw 'Automation endpoint did not become ready.' }
    # A separate windowless MTA process provides a hard deadline even if a provider blocks.
    $clientArguments = @('-NoProfile','-Mta','-File',"`"$PSCommandPath`"",'-Application',$Application,'-AsPort',$AsPort,'-ClientProcessId',$owned.Id,'-Scenario',$Scenario)
    if ($Scenario -eq 'Review2') {
        function Find-Busy($node) {
            if ($node.elementText -eq 'Busy') { return $node.bounds }
            foreach ($child in $node.children) { $found = Find-Busy $child; if ($found) { return $found } }
        }
        $bounds = Find-Busy $state.MainWindow.composition
        if (!$bounds) { throw 'Missing Busy button in automation layout.' }
        $clientArguments += @('-BusyClick', ('!LeftClick:{0},{1}' -f [int](($bounds.x1+$bounds.x2)/2), [int](($bounds.y1+$bounds.y2)/2)))
    }
    if ($HostedFixture) { $clientArguments += '-HostedFixture' }
    $client = Start-Process powershell.exe -WindowStyle Hidden -ArgumentList $clientArguments -RedirectStandardOutput (Join-Path $env:TEMP "$Application-uia.stdout.txt") -RedirectStandardError (Join-Path $env:TEMP "$Application-uia.stderr.txt") -PassThru
    $clientHandle = $client.Handle
    $lastCount = 0
    # Full showcase coverage includes repeated layout rebuilds and palette replacement.
    $deadline = [DateTime]::UtcNow.AddMinutes(30)
    do {
        $lines = @(Get-Content (Join-Path $env:TEMP "$Application-uia.stdout.txt") -ErrorAction SilentlyContinue)
        $lines | Select-Object -Skip $lastCount | Write-Host
        $lastCount = $lines.Count
        if ($client.WaitForExit(500)) { break }
        Assert-Target
    } while ([DateTime]::UtcNow -lt $deadline)
    if (!$client.HasExited) { $client.Kill(); throw 'MTA UIA test deadline exceeded.' }
    Get-Content (Join-Path $env:TEMP "$Application-uia.stdout.txt") | Select-Object -Skip $lastCount | Write-Host
    if ($client.ExitCode -ne 0) { throw "UIA client exit: $($client.ExitCode). $(Get-Content (Join-Path $env:TEMP "$Application-uia.stderr.txt") -Raw)" }
    if (!$launcher.WaitForExit(30000)) { throw 'Normal UIA Window.Close did not stop the launched application.' }
    if ($launcher.ExitCode -ne 0) { throw "Application exit: $($launcher.ExitCode)" }
    if (Get-Process -Id $owned.Id -ErrorAction SilentlyContinue) { throw 'Owned process survived shutdown.' }
    $stillListening = $false
    try { $null = Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 2; $stillListening = $true } catch { }
    if ($stillListening) { throw 'Owned endpoint survived shutdown.' }
    Write-Host "PASS $Application / normal shutdown and endpoint release"
}
finally {
    if ($client -and !$client.HasExited) { $client.Kill() }
    if ($owned -and !$owned.HasExited) { $owned.Kill() }
    if ($launcher -and !$launcher.HasExited) { $launcher.Kill() }
    if ($original) { [IO.File]::WriteAllBytes($userFile, $original) }
    elseif (Test-Path -LiteralPath $userFile) { Remove-Item -LiteralPath $userFile }
}
