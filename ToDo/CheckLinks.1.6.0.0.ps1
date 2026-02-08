param(
	[string]$Path = (Join-Path $PSScriptRoot '1.6.0.0.md'),
	[int]$TimeoutSec = 20,
	[int]$MaxRedirect = 5
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Path))
{
	throw "File not found: $Path"
}

$invokeWebRequest = Get-Command Invoke-WebRequest -ErrorAction Stop
$useBasicParsing = $invokeWebRequest.Parameters.ContainsKey('UseBasicParsing')

$content = Get-Content -Raw -LiteralPath $Path
$matches = [regex]::Matches($content, 'https?://[^\s)\]]+')
$links =
	$matches |
	ForEach-Object { $_.Value.TrimEnd('.', ',', ';', ':') } |
	Sort-Object -Unique

Write-Host "FILE=$Path"
Write-Host "LINKS=$($links.Count)"

$results = @()

foreach ($u in $links)
{
	$status = $null
	$final = $null
	$err = $null

	$common = @{
		Uri = $u
		MaximumRedirection = $MaxRedirect
		TimeoutSec = $TimeoutSec
	}
	if ($useBasicParsing)
	{
		$common.UseBasicParsing = $true
	}

	try
	{
		$r = Invoke-WebRequest @common -Method Head
		$status = $r.StatusCode
		$final = $r.BaseResponse.ResponseUri.AbsoluteUri
	}
	catch
	{
		try
		{
			$r = Invoke-WebRequest @common -Method Get
			$status = $r.StatusCode
			$final = $r.BaseResponse.ResponseUri.AbsoluteUri
		}
		catch
		{
			$err = $_.Exception.Message.Split("`n")[0]
		}
	}

	$results += [pscustomobject]@{
		Url    = $u
		Status = $status
		Final  = $final
		Error  = $err
	}
}

foreach ($row in $results)
{
	if ($row.Status -and $row.Status -lt 400)
	{
		if ($row.Final -and $row.Final -ne $row.Url)
		{
			Write-Host "$($row.Status) $($row.Url) -> $($row.Final)"
		}
		else
		{
			Write-Host "$($row.Status) $($row.Url)"
		}
	}
	else
	{
		Write-Host "ERR $($row.Url) :: $($row.Error)"
	}
}

$bad = @($results | Where-Object { -not $_.Status -or $_.Status -ge 400 })
Write-Host "BAD_COUNT=$($bad.Count)"

if ($bad.Count -gt 0)
{
	$bad | Select-Object Url, Status, Final, Error | Format-Table -AutoSize
	exit 1
}
