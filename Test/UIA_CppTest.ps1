param(
    [ValidateRange(1,65535)][int]$AsPort = 8888,
    [ValidateSet('All','Layout')][string]$Scenario = 'All'
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.ps1') -Application CppTest -AsPort $AsPort -Scenario $Scenario
exit $LASTEXITCODE
