#requires -Version 7.0
param([Parameter(Mandatory)][int]$FixtureProcessId)
. (Join-Path $PSScriptRoot 'CliCommon.ps1')

function Select-Fixture($Client) {
    $windows = Get-UiaFixtureWindows $Client $FixtureProcessId
    $window = $windows | Where-Object { $_.class -eq 'UiaSyntheticFixture' -and $_.title -notlike '*10,000*' } | Select-Object -First 1
    $tree = $Client.Send('Print-Window', $window.id).result
    return @{window=$window; element=(Get-UiaNode $tree 'Synthetic capabilities').id}
}
$log = Join-Path $PSScriptRoot "UiaFixture-$FixtureProcessId.synthetic.txt"
function Get-SetterCount {
    return [regex]::Matches((Read-UiaFixtureLog $log), '\tSetValue\(BSTR\)\t').Count
}
function Get-MoveCount {
    return [regex]::Matches((Read-UiaFixtureLog $log), '\tRange.Move\t').Count
}

$client = New-UiaClient
$bytes = [Text.Encoding]::UTF8.GetBytes('Help-Command ') + [byte[]]@(0xFF, 0x0A)
$client.Process.StandardInput.BaseStream.Write($bytes, 0, $bytes.Length)
$client.Process.StandardInput.BaseStream.Flush()
$invalid = $client.Receive('', 'Help-Command <invalid UTF-8>', $false)
Assert-UiaEqual $invalid.error.code 'InvalidEncoding'
$invalid = $client.Send(('Help-Command {"x":"' + [char]0 + '"}'), $null, $null, $false)
Assert-UiaEqual $invalid.error.code 'InvalidJson'
$client.Process.StandardInput.Write("Help-Command`nHelp-Provider`n")
$client.Process.StandardInput.Flush()
foreach ($command in @('Help-Command','Help-Provider')) { $null = $client.Receive($command, $command, $true) }
$selected = Select-Fixture $client
$window = $selected.window
$element = $selected.element
$unavailable = $client.Send('Run-ISpreadsheetProvider::GetItemByName', $element, @{name='__uia_unavailable__'}, $false).error
Assert-Uia ($unavailable.code -eq 'Unavailable' -and $unavailable.phase -eq 'invocation' -and $unavailable.hresult) 'Expected invocation failure'
$null = $client.Send('Help-Command')
$tree = $client.Send('Print-Window', $window.id).result
$document = (Get-UiaNode $tree 'Synthetic document').id
$textRange = $client.Send('Run-ITextProvider::DocumentRange', $document).result.value.value
$before = Get-MoveCount
$readback = $client.Send('Run-IUIAutomationTextRange::Move', $textRange, @{unit=0;count=12345}, $false).error
Assert-Uia ($readback.code -eq 'Unavailable' -and $readback.phase -eq 'readback') 'Expected readback failure'
Assert-Uia ((Get-MoveCount) -eq $before + 1) 'Range edit was retried after readback failure'
$tree = $client.Send('Refresh-Window', $window.id).result
$element = (Get-UiaNode $tree 'Synthetic capabilities').id
$null = $client.Send('Query-Properties', $element)
$other = New-UiaClient
$otherSelected = Select-Fixture $other
$result = $other.Send('Run-IToggleProvider::Toggle', $otherSelected.element).result
$before = Get-SetterCount
$rejected = $client.Send('Set-Property', $element, @{propertyId=30045;value='must not commit'}, $false)
Assert-UiaEqual $rejected.error.code 'CapabilityChanged'
Assert-Uia ((Get-SetterCount) -eq $before) 'Rejected setter reached provider'
for ($index = 0; $index -lt 2; $index++) {
    $otherElement = (Get-UiaNode $result.tree 'Synthetic capabilities').id
    $result = $other.Send('Run-IToggleProvider::Toggle', $otherElement).result
}
$other.Close()
$client.Close()

$fatal = New-UiaClient
$selected = Select-Fixture $fatal
$result = $fatal.Send('Run-ISpreadsheetProvider::GetItemByName', $selected.element, @{name='__uia_fatal__'}, $false)
Assert-Uia ($result.error.code -eq 'Fatal' -and $result.error.phase -eq 'invocation') 'Fatal provider response'
$fatal.Finish($false)
Write-Host 'PASS expected invocation, committed readback failure, changed capability, fatal exit and pipeline framing'
