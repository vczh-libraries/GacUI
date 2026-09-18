param([ValidateRange(1,65535)][int]$AsPort = 8890)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_Showcase.ps1') -Application CppTest_Metaonly -AsPort $AsPort
exit $LASTEXITCODE
