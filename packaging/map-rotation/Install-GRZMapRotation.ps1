param(
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
 [string]$MissionName = 'dayzOffline.chernarusplus'
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) { throw 'Close DayZ and stop the server first.' }
foreach ($pair in @(@($ServerPath,'DayZServer_x64.exe'), @($GamePath,'DayZ_x64.exe'))) {
 if (!(Test-Path -LiteralPath (Join-Path $pair[0] $pair[1]))) { throw "Cannot find $($pair[1]) in $($pair[0])." }
}
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'MANIFEST.json') -Raw | ConvertFrom-Json
$pbo = Join-Path $PSScriptRoot 'GRZ_MapRotation.pbo'
$map = Join-Path $PSScriptRoot 'EditorFiles\GRZ_BBC_DM_2.dze'
if ((Get-FileHash -LiteralPath $pbo -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.pbo_sha256) { throw 'PBO does not match manifest.' }
if ((Get-FileHash -LiteralPath $map -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.sanitized_map_sha256) { throw 'DZE does not match manifest.' }
$mission = Join-Path $ServerPath ('mpmissions\' + $MissionName)
if (!(Test-Path -LiteralPath $mission)) { throw "Mission folder not found: $mission" }
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
foreach ($target in @($ServerPath,$GamePath)) {
 $addons = Join-Path $target '@GRZ_Local\Addons'
 $destination = Join-Path $addons 'GRZ_MapRotation.pbo'
 New-Item -ItemType Directory -Path $addons -Force | Out-Null
 if (Test-Path -LiteralPath $destination) {
  $backup = Join-Path $target ('GRZ-MapRotation-backup-' + $stamp)
  New-Item -ItemType Directory -Path $backup -Force | Out-Null
  Copy-Item -LiteralPath $destination -Destination $backup
 }
 Copy-Item -LiteralPath $pbo -Destination $destination -Force
 if ((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.pbo_sha256) { throw 'Installed PBO hash mismatch.' }
}
$editorFiles = Join-Path $mission 'EditorFiles'
New-Item -ItemType Directory -Path $editorFiles -Force | Out-Null
$backupMaps = Join-Path $ServerPath ('GRZ-MapRotation-backup-' + $stamp + '\EditorFiles')
foreach ($existing in @(Get-ChildItem -LiteralPath $editorFiles -Recurse -File -Filter '*.dze')) {
 $hash = (Get-FileHash -LiteralPath $existing.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
 $sameMap = $hash -eq $manifest.original_map_sha256 -or $hash -eq $manifest.sanitized_map_sha256
 if ($sameMap -or $existing.FullName -eq (Join-Path $editorFiles 'GRZ_BBC_DM_2.dze')) {
  New-Item -ItemType Directory -Path $backupMaps -Force | Out-Null
  $backupName = [guid]::NewGuid().ToString('N') + '-' + $existing.Name
  Move-Item -LiteralPath $existing.FullName -Destination (Join-Path $backupMaps $backupName)
 }
}
Copy-Item -LiteralPath $map -Destination (Join-Path $editorFiles 'GRZ_BBC_DM_2.dze') -Force
Write-Host 'Installed GRZ_MapRotation.pbo on BOTH game and server; one copy of the supplied DZE installed in EditorFiles.'
Write-Host 'Keep @GRZ_Local loaded on BOTH sides, and Dabs Framework + Editor Loader on the server.'
Write-Host 'Keep all mods used by the DZE loaded. Existing server BAT/profile settings were preserved.'
Write-Host 'Start server and game. Voting can be reopened with !vote or F7. Check RPT for runtime-checked spawn counts.'
