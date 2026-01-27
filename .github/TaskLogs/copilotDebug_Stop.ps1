. $PSScriptRoot\copilotShared.ps1
$cdbpath = GetCDBPath
$commandLine = "`"$($cdbpath)`" -remote npipe:server=.,pipe=VlppUnitTest -c `"qq`""
echo $commandLine
cmd.exe /S /C $commandLine 2>&1
