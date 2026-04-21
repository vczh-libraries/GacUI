. $PSScriptRoot\copilotShared.ps1
$cdbpath = GetCDBPath
$debuggerNamedPipe = GetDebuggerNamedPipe
$commandLine = "`"$($cdbpath)`" -remote npipe:server=.,pipe=$debuggerNamedPipe -clines 0 -c `"qq`""
echo $commandLine
cmd.exe /S /C $commandLine
