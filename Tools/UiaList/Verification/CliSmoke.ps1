#requires -Version 7.0
param(
    [Parameter(Mandatory)][int]$FixtureProcessId,
    [ValidateSet('x64','Win32')][string]$Platform = 'x64',
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$Solution = (Split-Path $PSScriptRoot -Parent)
)
. (Join-Path $PSScriptRoot 'CliCommon.ps1')

$client = New-UiaClient -Solution $Solution -Configuration $Configuration -Platform $Platform
$client.Process.StandardInput.Write(" `t`n`n")
$client.Process.StandardInput.Flush()
$null = $client.Send('Help-Command')
$catalog = $client.Send('Help-Provider').result.catalog
Assert-Uia ($catalog.Count -eq 37) 'Provider catalog count'
$null = $client.Send('Help-Command {broken}', $null, $null, $false)
$null = $client.Send('Bogus', $null, $null, $false)
$null = $client.Send('List-Process', $null, @{unexpected=0}, $false)
$null = $client.Send('Query-Node', 'missing', $null, $false)
$windows = Get-UiaFixtureWindows $client $FixtureProcessId
$window = $windows | Where-Object { $_.class -eq 'UiaSyntheticFixture' -and $_.title -notlike '*10,000*' } | Select-Object -First 1
$tree = $client.Send('Print-Window', $window.id)
Assert-Uia ($tree.result.nodes.Count -ge 45) 'Complete synthetic tree'
$capability = Get-UiaNode $tree.result 'Synthetic capabilities'
$document = Get-UiaNode $tree.result 'Synthetic document'
$inspected = $client.Send('Query-Node', $capability.id).result
Assert-Uia ($inspected.providers.Count -ge 30) 'Supported providers'
Assert-Uia (@($inspected.properties | Where-Object { $_.value.kind -eq 'Boolean' -and $_.value.value -ceq $false }).Count -gt 0) 'Supported false value'
Assert-Uia (@($inspected.properties | Where-Object { $_.value.kind -eq 'Signed' -and $_.value.value -ceq '0' }).Count -gt 0) 'Supported zero value'
Assert-Uia (@($inspected.properties | Where-Object { $_.value.kind -eq 'String' -and $_.value.value -ceq '' }).Count -gt 0) 'Supported empty value'
$invalid = $client.Send('Set-Property', $capability.id, @{propertyId=30045;value=1}, $false)
Assert-UiaEqual $invalid.error.code 'InvalidType'
$old = $capability.id
$textValue = "CLI 日本語 中文`nsecond line"
$edited = $client.Send('Set-Property', $old, @{propertyId=30045;value=$textValue})
Assert-Uia ($edited.generation -cne $tree.generation) 'Mutation advances generation'
Assert-Uia (@($edited.result.readback.properties | Where-Object { $_.propertyId -eq 30045 -and $_.value.value -ceq $textValue }).Count -eq 1) 'Actual text readback'
$null = $client.Send('Query-Node', $old, $null, $false)
$capability = Get-UiaNode $edited.result.tree 'Synthetic capabilities'
$document = Get-UiaNode $edited.result.tree 'Synthetic document'
$null = $client.Send('Select-Node', $capability.id)
$null = $client.Send('Query-Providers', $document.id)
$textRange = $client.Send('Run-ITextProvider::DocumentRange', $document.id).result.value.value
$null = $client.Send('Query-Range', $textRange)
$clone = $client.Send('Run-IUIAutomationTextRange::Clone', $textRange).result.value.value
$same = $client.Send('Run-IUIAutomationTextRange::Compare', $textRange, @{range=$clone})
Assert-UiaEqual $same.result.value.value $true
$text = $client.Send('Run-IUIAutomationTextRange::GetText', $textRange, @{maxLength=-1}).result.value
Assert-Uia ($text.kind -eq 'String' -and $text.value.Length -gt 0) 'Range text'
$preview = $client.Send('Query-Preview', $window.id).result
if ($preview.status -eq 'Ready') {
    $bitmap = [Convert]::FromBase64String($preview.base64)
    Assert-Uia ($bitmap[0] -eq 0x42 -and $bitmap[1] -eq 0x4D) 'BMP signature'
}
$null = $client.Send('HitTest-Preview', $window.id, @{x=-1;y=-1})
$null = $client.Send('Refresh-Window', $window.id)
$expired = $client.Send('Query-Range', $textRange, $null, $false)
Assert-UiaEqual $expired.error.code 'ExpiredId'
$client.Close()
$eof = New-UiaClient -Solution $Solution -Configuration $Configuration -Platform $Platform
$null = $eof.Send('Help-Command')
$eof.Close($true)
