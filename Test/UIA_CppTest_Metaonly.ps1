param(
    [ValidateRange(1,65535)][int]$AsPort = 8890,
    [ValidateSet('All','Layout','Responsive','ResponsiveWindow','RawClock','Touching')][string]$Scenario = 'All',
    [switch]$Gdi,
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.ps1') -Application CppTest_Metaonly -AsPort $AsPort -Scenario $Scenario -Gdi:$Gdi -SkipBuild:$SkipBuild
exit $LASTEXITCODE
