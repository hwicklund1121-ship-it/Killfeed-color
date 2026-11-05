param(
 [Parameter(Mandatory=$true)][ValidatePattern('^\d{17}$')][string]$SteamId,
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
 [string]$ProfilePath = ''
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) { throw 'Close DayZ and stop the server first.' }
foreach ($pair in @(@($ServerPath,'DayZServer_x64.exe'),@($GamePath,'DayZ_x64.exe'))) {
 if (!(Test-Path -LiteralPath (Join-Path $pair[0] $pair[1]))) { throw "Game executable missing in $($pair[0])" }
}
if (!$ProfilePath) { $ProfilePath = Join-Path $ServerPath 'profiles' }
$configPath = Join-Path $ProfilePath 'GRZ_MapRotation/config.json'
if (!(Test-Path -LiteralPath $configPath)) { throw "Existing map rotation config not found: $configPath. Start rotation once first, or supply -ProfilePath." }
$config = Get-Content -LiteralPath $configPath -Raw | ConvertFrom-Json
if (!$config.PSObject.Properties['AdminSteamIds']) { $config | Add-Member -NotePropertyName AdminSteamIds -NotePropertyValue @() }
$config.AdminSteamIds = @(@($config.AdminSteamIds) + $SteamId | Select-Object -Unique)
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'MANIFEST.json') -Raw | ConvertFrom-Json
$changes = @()
foreach ($package in $manifest.pbo.PSObject.Properties) {
 $source = Join-Path $PSScriptRoot ($package.Name + '.pbo')
 if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant() -ne $package.Value.sha256) { throw "Package hash mismatch: $source" }
 foreach ($target in @($ServerPath,$GamePath)) {
  $changes += [pscustomobject]@{Source=$source; Target=(Join-Path $target ('@GRZ_Local/Addons/' + $package.Name + '.pbo')); Existed=$false; Backup=''}
 }
}
$backup = Join-Path $ServerPath ('GRZ-update-backup-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item -LiteralPath $configPath -Destination (Join-Path $backup 'config.json')
$done = @()
try {
 foreach ($change in $changes) {
  $change.Existed = Test-Path -LiteralPath $change.Target
  $change.Backup = Join-Path $backup ([guid]::NewGuid().ToString('N') + '.pbo')
  if ($change.Existed) { Copy-Item -LiteralPath $change.Target -Destination $change.Backup }
  New-Item -ItemType Directory -Path (Split-Path $change.Target) -Force | Out-Null
  $done += $change
  Copy-Item -LiteralPath $change.Source -Destination $change.Target -Force
  if ((Get-FileHash -LiteralPath $change.Source).Hash -ne (Get-FileHash -LiteralPath $change.Target).Hash) { throw 'Installed PBO hash mismatch.' }
 }
 [IO.File]::WriteAllText($configPath, ($config | ConvertTo-Json -Depth 100), (New-Object System.Text.UTF8Encoding($false)))
} catch {
 foreach ($change in $done) {
  if ($change.Existed) { Copy-Item -LiteralPath $change.Backup -Destination $change.Target -Force }
  elseif (Test-Path -LiteralPath $change.Target) { Remove-Item -LiteralPath $change.Target }
 }
 Copy-Item -LiteralPath (Join-Path $backup 'config.json') -Destination $configPath -Force
 throw
}
Write-Host 'Installed on client and server. Admin !skip is enabled with a 30-second top-screen warning. Restart both.'
Write-Host "Backup: $backup"
