. $PSScriptRoot\copilotShared.ps1
$cdbpath = GetCDBPath
$commandLine = "`"$($cdbpath)`" -remote npipe:server=.,pipe=VlppUnitTest -clines 0 -c `"qq`""
echo $commandLine
cmd.exe /S /C $commandLine
