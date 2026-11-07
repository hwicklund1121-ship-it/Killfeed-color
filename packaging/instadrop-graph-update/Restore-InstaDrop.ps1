param([Parameter(Mandatory=$true)][string]$BackupPath)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) { throw 'Close DayZ and stop the server first.' }
$operations = @(Get-Content -LiteralPath (Join-Path $BackupPath 'RESTORE.json') -Raw | ConvertFrom-Json)
foreach ($operation in $operations) {
 if ($operation.Existed -and !(Test-Path -LiteralPath $operation.Saved -PathType Leaf)) { throw "Missing backup: $($operation.Saved). Nothing restored." }
}
foreach ($operation in $operations) {
 if ($operation.Existed) { Copy-Item -LiteralPath $operation.Saved -Destination $operation.Target -Force }
 elseif (Test-Path -LiteralPath $operation.Target -PathType Leaf) { Remove-Item -LiteralPath $operation.Target }
}
Write-Host 'Restored the previous InstaDrop files on both sides. Restart both to use them.'
