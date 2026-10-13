param(
    [ValidateRange(1,65535)][int]$AsPort = 8888,
    [ValidateSet('All','Layout','Responsive','ResponsiveWindow')][string]$Scenario = 'All',
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.ps1') -Application CppTest -AsPort $AsPort -Scenario $Scenario -SkipBuild:$SkipBuild
exit $LASTEXITCODE
