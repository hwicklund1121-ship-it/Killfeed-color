param(
 [Parameter(Mandatory=$true)][ValidatePattern('^\d{17}$')][string]$SteamId,
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
 [string]$ProfilePath = '',
 [string]$MissionName = 'dayzOffline.chernarusplus',
 [string]$PackageDirectory = '',
 [string]$WindstrideServerPbo = '',
 [string]$WindstrideClientPbo = '',
 [string[]]$ExtraModPaths = @(),
 [switch]$SkipLayeredShirt
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) { throw 'Close DayZ and stop the server first.' }
foreach ($pair in @(@($ServerPath,'DayZServer_x64.exe'),@($GamePath,'DayZ_x64.exe'))) {
 if (!(Test-Path -LiteralPath (Join-Path $pair[0] $pair[1]))) { throw "Game executable missing in $($pair[0])" }
}
$catalogue = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'RESKIN-PACKS.json') -Raw | ConvertFrom-Json
if ($catalogue.schema -ne 1) { throw 'Unsupported reskin package manifest.' }

function Find-WindstridePbo([string]$root, [string]$specified) {
 if ($specified) {
  if (!(Test-Path -LiteralPath $specified -PathType Leaf) -or [IO.Path]::GetExtension($specified) -ne '.pbo') { throw "Windstride PBO not found: $specified" }
  return $specified
 }
 foreach ($mod in @(Get-ChildItem -LiteralPath $root -Directory -Filter '@*')) {
  foreach ($name in @('WindstrideClothing.pbo','Layered_Shirt.pbo')) {
   $candidate = Join-Path $mod.FullName ('Addons/' + $name)
   if (Test-Path -LiteralPath $candidate -PathType Leaf) { return $candidate }
  }
 }
 $steamapps = Split-Path (Split-Path $root -Parent) -Parent
 if (!$steamapps) { return '' }
 $workshop = Join-Path $steamapps 'workshop/content/221100'
 if (Test-Path -LiteralPath $workshop -PathType Container) {
  foreach ($mod in @(Get-ChildItem -LiteralPath $workshop -Directory)) {
   foreach ($name in @('WindstrideClothing.pbo','Layered_Shirt.pbo')) {
    $candidate = Join-Path $mod.FullName ('Addons/' + $name)
    if (Test-Path -LiteralPath $candidate -PathType Leaf) { return $candidate }
   }
  }
 }
 return ''
}
$windstrideInputs = @()
if (!$SkipLayeredShirt) {
 foreach ($side in @(@($ServerPath,$WindstrideServerPbo),@($GamePath,$WindstrideClientPbo))) {
  $found = Find-WindstridePbo $side[0] $side[1]
  if (!$found) {
   throw "Original Windstride Clothing PBO not found under $($side[0]). A .bisign is insufficient. Install/load Windstride on both sides; or pass -WindstrideServerPbo and -WindstrideClientPbo for external mod paths. Use -SkipLayeredShirt to install the other updates only. Nothing installed."
  }
  $windstrideInputs += $found
 }
}

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
   if ($SkipLayeredShirt -and $entry.Name -eq 'GRZ_XimsLayeredShirt') { continue }
   if ($entry.Name -notmatch '^GRZ_[A-Za-z0-9_]+$') { throw 'Invalid addon name in manifest.' }
   $pbo = Join-Path $expanded ($entry.Name + '.pbo')
   if ((Get-FileHash -LiteralPath $pbo -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Value) { throw "PBO hash mismatch: $($entry.Name). Nothing installed." }
   $addonPaths += [pscustomobject]@{Name=$entry.Name; Source=$pbo; Hash=$entry.Value}
  }
 }
 if (!$core) { throw 'Core archive not found.' }
 # Generate overrides from this installation, before the transactional installer writes anything.
 $inputs = @($addonPaths | ForEach-Object { $_.Source }) + @($windstrideInputs | Select-Object -First 1)
 $replaced = @($addonPaths | ForEach-Object { $_.Name + '.pbo' })
 $localAddons = Join-Path $ServerPath '@GRZ_Local/Addons'
 if (Test-Path -LiteralPath $localAddons) {
  $inputs += @(Get-ChildItem -LiteralPath $localAddons -Filter '*.pbo' -File | Where-Object { $_.Name -notin $replaced -and $_.Name -ne 'GRZ_MovementSpeedData.pbo' } | ForEach-Object { $_.FullName })
 }
 $generated = Join-Path $staging 'generated'
 & (Join-Path $PSScriptRoot 'inventory-builder/Build-Inventory-Mod.ps1') -DayZPath $GamePath -ExtraPboPaths $inputs -ExtraModPaths $ExtraModPaths -OutputPath $generated -NoExplorer
 $nativeArchives = @()
 foreach ($folder in @((Join-Path $GamePath 'Addons'),(Join-Path $GamePath 'dta'))) {
  $nativeArchives += @(Get-ChildItem -LiteralPath $folder -Filter '*.pbo' -File -Recurse | ForEach-Object { $_.FullName })
 }
 $null = [GRZInventoryBuilder]::BuildMovementData([string[]]$nativeArchives, $generated)
 foreach ($name in @('GRZ_InventorySizes','GRZ_MovementSpeedData')) {
  $source = Join-Path $generated ($name + '.pbo')
  $addonPaths += [pscustomobject]@{Name=$name;Source=$source;Hash=(Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant()}
 }
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
 if ($SkipLayeredShirt) { Write-Host 'Layered Shirt skipped as requested; other reskins installed.' }
 else { Write-Host 'All four XIMS clothes installed. Keep original Windstride Clothing loaded on both sides.' }
 Write-Host 'Canvas Bag uses vanilla normal detail pending a smaller custom normal upload.'
} catch {
 $extraConfigs = Join-Path $staging 'generated/ExtraConfigs'
 if (Test-Path -LiteralPath $extraConfigs -PathType Container) {
  try {
   $diagnostics = Join-Path $PSScriptRoot ('GRZ-Install-Diagnostics-' + [guid]::NewGuid().ToString('N'))
   New-Item -ItemType Directory -Path $diagnostics | Out-Null
   Copy-Item -LiteralPath $extraConfigs -Destination $diagnostics -Recurse
   Write-Host "Installation stopped. Small config diagnostics retained at $diagnostics"
  } catch { Write-Warning 'Could not retain diagnostic config copies.' }
 }
 throw
} finally {
 if (Test-Path -LiteralPath $staging) { Remove-Item -LiteralPath $staging -Recurse -Force }
}
