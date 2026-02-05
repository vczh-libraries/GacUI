param(
    [Parameter(Mandatory=$true)]
    [string]$Command
)

. $PSScriptRoot\copilotShared.ps1
$cdbpath = GetCDBPath
$commandLine = "echo .remote_exit | `"$($cdbpath)`" -remote npipe:server=.,pipe=VlppUnitTest -clines 0 -c `"$Command`""
echo $commandLine
cmd.exe /S /C $commandLine
