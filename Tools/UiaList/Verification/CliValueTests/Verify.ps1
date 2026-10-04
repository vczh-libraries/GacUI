#requires -Version 7.0
# Assert JSON preservation independently of normalization by Windows UIA.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'CliCommon.ps1')

$process = Start-UiaProcess -Solution $PSScriptRoot -Executable CliValueTests
$output = $process.StandardOutput.ReadToEndAsync()
$errors = $process.StandardError.ReadToEndAsync()
Assert-Uia ($process.WaitForExit(120000)) 'Value tests timed out'
Assert-Uia ($process.ExitCode -eq 0) ($errors.GetAwaiter().GetResult())
$value = ConvertFrom-Json -InputObject ($output.GetAwaiter().GetResult()) -AsHashtable -Depth 100
Assert-UiaEqual $value.dimensions @(@{lower=-3;upper=8}) 'Outer array bounds'
$items = @{}
foreach ($item in $value.value) { $items[$item.kind] = $item }
Assert-Uia ($items.Count -eq 12) 'All twelve ValueData kinds'
Assert-UiaEqual $items.Signed.value '-9007199254740993'
Assert-UiaEqual $items.Unsigned.value '18446744073709551615'
Assert-UiaEqual $items.String.value ("A" + [char]0 + [char]1 + [char]11 + [char]31 + '中Z')
Assert-UiaEqual $items.Boolean.value $false
Assert-UiaEqual $items.Real.value @{nonfinite='NaN'}
Assert-UiaEqual $items.Array.dimensions @(@{lower=2;upper=3}) 'Nested array bounds'
Assert-UiaEqual @($items.Array.value | ForEach-Object { $_.value }) @(@{nonfinite='Infinity'},@{nonfinite='-Infinity'})
foreach ($kind in @('Null','Mixed','Unsupported')) { Assert-UiaEqual $items[$kind].value $null $kind }
$process.Dispose()
Write-Host 'PASS all ValueData kinds, full control/NUL/Unicode text, precise integers, nonfinite reals and array bounds'
