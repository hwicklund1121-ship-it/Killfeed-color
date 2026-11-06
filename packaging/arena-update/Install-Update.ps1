param(
 [Parameter(Mandatory=$true)][ValidatePattern('^\d{17}$')][string]$SteamId,
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
 [string]$ProfilePath = '',
 [string]$MissionName = 'dayzOffline.chernarusplus'
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
$mission = Join-Path $ServerPath ('mpmissions/' + $MissionName)
if (!(Test-Path -LiteralPath $mission)) { throw "Mission folder not found: $mission" }
$editor = Join-Path $mission 'EditorFiles'
foreach ($entry in ,@('GRZ_BBC_DM_3.dze',(Join-Path $editor 'GRZ_BBC_DM_3.dze'))) {
 $source = Join-Path $PSScriptRoot $entry[0]
 $expected = $manifest.files.PSObject.Properties[$entry[0]].Value
 if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw "Invalid configuration or DZE: $source" }
 $changes += [pscustomobject]@{Source=$source; Target=$entry[1]; Existed=$false; Backup=''}
}
$duelConfigPath = Join-Path $ProfilePath 'GRZ_Duels/config.json'
if (!(Test-Path -LiteralPath $duelConfigPath)) { throw "Duel config not found: $duelConfigPath. Start the server once first." }
$duelConfig = Get-Content -LiteralPath $duelConfigPath -Raw | ConvertFrom-Json
if (!$duelConfig.PSObject.Properties['Maps']) { throw 'Duel config has no Maps array; existing config preserved.' }
$mapUpdatePath = Join-Path $PSScriptRoot 'duel-map-update.json'
if ((Get-FileHash -LiteralPath $mapUpdatePath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.files.'duel-map-update.json') { throw 'Duel map update hash mismatch.' }
$mapUpdate = Get-Content -LiteralPath $mapUpdatePath -Raw | ConvertFrom-Json
if ($mapUpdate.Id -ne 'long_range_1v1') { throw 'Unexpected duel map ID; existing config preserved.' }
$foundDuelMap = $false
$duelConfig.Maps = @($duelConfig.Maps | ForEach-Object {
 if ($_.Id -eq $mapUpdate.Id) { $foundDuelMap = $true; $mapUpdate } else { $_ }
})
if (!$foundDuelMap) { $duelConfig.Maps = @($duelConfig.Maps) + $mapUpdate }
$backup = Join-Path $ServerPath ('GRZ-update-backup-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $backup | Out-Null
Copy-Item -LiteralPath $configPath -Destination (Join-Path $backup 'config.json')
$stagedDuelConfig = Join-Path $backup 'duel-config.updated.json'
[IO.File]::WriteAllText($stagedDuelConfig, ($duelConfig | ConvertTo-Json -Depth 100), (New-Object System.Text.UTF8Encoding($false)))
$changes += [pscustomobject]@{Source=$stagedDuelConfig; Target=$duelConfigPath; Existed=$false; Backup=''}
$done = @()
$movedMaps = @()
try {
 if (Test-Path -LiteralPath $editor) {
  foreach ($old in @(Get-ChildItem -LiteralPath $editor -Filter '*.dze' -File -Recurse)) {
   if ($old.FullName -eq (Join-Path $editor 'GRZ_BBC_DM_3.dze')) { continue }
   $oldHash = (Get-FileHash -LiteralPath $old.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
   if ($old.Name -in @('GRZ_BBC_DM_2.dze','GRZ_BBC_DM_3.dze','GRZ_BBC_DM_3POND.dze') -or $oldHash -in $manifest.previous_map_hashes -or $oldHash -eq $manifest.files.'GRZ_BBC_DM_3.dze') {
    $saved = Join-Path $backup ([guid]::NewGuid().ToString('N') + '.dze')
    Move-Item -LiteralPath $old.FullName -Destination $saved
    $movedMaps += [pscustomobject]@{Original=$old.FullName; Saved=$saved}
   }
  }
 }
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
 foreach ($moved in $movedMaps) { Move-Item -LiteralPath $moved.Saved -Destination $moved.Original -Force }
 Copy-Item -LiteralPath (Join-Path $backup 'config.json') -Destination $configPath -Force
 throw
}
Write-Host 'Installed VIP menu, loadout updates and POND DZE. Long Range 1v1 updated; other arenas, VIP access, prefixes, ratings and loadouts preserved. Restart server and client.'
Write-Host "Backup: $backup"
