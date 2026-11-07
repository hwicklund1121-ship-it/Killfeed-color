param(
 [Parameter(Mandatory=$true)][ValidatePattern('^\d{17}$')][string]$SteamId,
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
 [string]$ProfilePath = '',
 [string]$MissionName = 'dayzOffline.chernarusplus',
 [string]$PackageDirectory = ''
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) { throw 'Close DayZ and stop the server first.' }
foreach ($pair in @(@($ServerPath,'DayZServer_x64.exe'),@($GamePath,'DayZ_x64.exe'))) {
 if (!(Test-Path -LiteralPath (Join-Path $pair[0] $pair[1]))) { throw "Game executable missing in $($pair[0])" }
}
$catalogue = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'RESKIN-PACKS.json') -Raw | ConvertFrom-Json
if ($catalogue.schema -ne 1) { throw 'Unsupported reskin package manifest.' }
$staging = Join-Path ([IO.Path]::GetTempPath()) ('GRZ-all-reskins-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $staging | Out-Null
try {
 $core = ''
 $addonPaths = @()
 foreach ($pack in $catalogue.packages) {
  if ($pack.file -notmatch '^[A-Za-z0-9_-]+\.zip$') { throw 'Invalid archive name in manifest.' }
  $archive = Join-Path $staging $pack.file
  if ($PackageDirectory) {
   Copy-Item -LiteralPath (Join-Path $PackageDirectory $pack.file) -Destination $archive
  } else {
   if ($pack.url -notmatch '^https://github\.com/hwicklund1121-ship-it/Killfeed-color/raw/[a-f0-9]{40}/downloads/[A-Za-z0-9_-]+\.zip$') { throw 'Unexpected download URL.' }
   Write-Host "Downloading $($pack.file)..."
   [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
   Invoke-WebRequest -Uri $pack.url -OutFile $archive -UseBasicParsing
  }
  if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $pack.sha256) { throw "Archive hash mismatch: $($pack.file). Nothing installed." }
  $expanded = Join-Path $staging ([IO.Path]::GetFileNameWithoutExtension($pack.file))
  Expand-Archive -LiteralPath $archive -DestinationPath $expanded
  if ($pack.role -eq 'core') {
   if ($core) { throw 'More than one core archive.' }
   $core = $expanded
  }
  foreach ($entry in $pack.addons.PSObject.Properties) {
   if ($entry.Name -notmatch '^GRZ_[A-Za-z0-9_]+$') { throw 'Invalid addon name in manifest.' }
   $pbo = Join-Path $expanded ($entry.Name + '.pbo')
   if ((Get-FileHash -LiteralPath $pbo -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Value) { throw "PBO hash mismatch: $($entry.Name). Nothing installed." }
   $addonPaths += [pscustomobject]@{Name=$entry.Name; Source=$pbo; Hash=$entry.Value}
  }
 }
 if (!$core) { throw 'Core archive not found.' }
 $manifestPath = Join-Path $core 'MANIFEST.json'
 $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
 foreach ($addon in $addonPaths) {
  $destination = Join-Path $core ($addon.Name + '.pbo')
  if ($addon.Source -ne $destination) { Copy-Item -LiteralPath $addon.Source -Destination $destination -Force }
  $manifest.pbo | Add-Member -NotePropertyName $addon.Name -NotePropertyValue ([pscustomobject]@{sha256=$addon.Hash}) -Force
 }
 [IO.File]::WriteAllText($manifestPath, ($manifest | ConvertTo-Json -Depth 100), (New-Object System.Text.UTF8Encoding($false)))
 $invoke = @{SteamId=$SteamId; ServerPath=$ServerPath; GamePath=$GamePath; MissionName=$MissionName}
 if ($ProfilePath) { $invoke.ProfilePath=$ProfilePath }
 & (Join-Path $core 'Install-Update.ps1') @invoke
 Write-Host 'Installed the available GRZ and XIMS reskin packs on client and server.'
 Write-Host 'Layered shirt and canvas bag are NOT installed: see INSTALL.txt for the missing source files.'
} finally {
 if (Test-Path -LiteralPath $staging) { Remove-Item -LiteralPath $staging -Recurse -Force }
}
