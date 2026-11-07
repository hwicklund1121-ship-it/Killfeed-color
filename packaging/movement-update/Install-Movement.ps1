param(
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ"
)
$ErrorActionPreference='Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) {throw 'Close DayZ and stop the server first.'}
foreach ($side in @(@($ServerPath,'DayZServer_x64.exe'),@($GamePath,'DayZ_x64.exe'))) {
 if (!(Test-Path -LiteralPath (Join-Path $side[0] $side[1]))) {throw "Game executable missing in $($side[0])"}
 if (!(Test-Path -LiteralPath (Join-Path $side[0] '@GRZ_Local/Addons/GRZ_Movement.pbo'))) {throw "Existing GRZ movement addon missing in $($side[0]). This updates the existing @GRZ_Local installation."}
}
$manifest=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'MANIFEST.json') -Raw | ConvertFrom-Json
foreach ($name in @('GRZ_Movement.pbo','InventoryBuilder.cs')) {
 if ((Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $name) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.$name) {throw "Package hash mismatch: $name"}
}
if (!('GRZInventoryBuilder' -as [type])) {Add-Type -Path (Join-Path $PSScriptRoot 'InventoryBuilder.cs')}
$archives=@()
foreach ($folder in @((Join-Path $GamePath 'Addons'),(Join-Path $GamePath 'dta'))) {
 if (!(Test-Path -LiteralPath $folder)) {throw "Native game folder missing: $folder"}
 $archives+=@(Get-ChildItem -LiteralPath $folder -Filter '*.pbo' -File -Recurse | ForEach-Object {$_.FullName})
}
$staging=Join-Path ([IO.Path]::GetTempPath()) ('GRZ-movement-'+[guid]::NewGuid().ToString('N'))
$backup=Join-Path $ServerPath ('GRZ-movement-backup-'+[guid]::NewGuid().ToString('N'))
$written=@()
try {
 $null=[GRZInventoryBuilder]::BuildMovementData([string[]]$archives,$staging)
 New-Item -ItemType Directory -Path $backup | Out-Null
 $operations=@()
 foreach ($side in @(@('server',$ServerPath),@('client',$GamePath))) {
  foreach ($name in @('GRZ_Movement.pbo','GRZ_MovementSpeedData.pbo')) {
   $target=Join-Path $side[1] ('@GRZ_Local/Addons/'+$name)
   $saved=Join-Path $backup ($side[0]+'-'+$name)
   $exists=Test-Path -LiteralPath $target
   if ($exists) {Copy-Item -LiteralPath $target -Destination $saved}
   $source=Join-Path $staging $name
   if ($name -eq 'GRZ_Movement.pbo') {$source=Join-Path $PSScriptRoot $name}
   $operations += [pscustomobject]@{Target=$target;Saved=$saved;Existed=$exists;Source=$source}
  }
 }
 foreach ($operation in $operations) {
  $written+=$operation
  Copy-Item -LiteralPath $operation.Source -Destination $operation.Target -Force
  if ((Get-FileHash -LiteralPath $operation.Target).Hash -ne (Get-FileHash -LiteralPath $operation.Source).Hash) {throw 'Installed PBO verification failed.'}
 }
 Write-Host 'Updated existing GRZ movement on client and server: native gait speed request 1.35. Restart both.'
 Write-Host "Backups: $backup"
 Write-Host 'Verify movement in game. The script log reports accepted/total native animation clips; this is not a measured speed guarantee.'
} catch {
 foreach ($operation in $written) {
  if ($operation.Existed) {Copy-Item -LiteralPath $operation.Saved -Destination $operation.Target -Force}
  elseif (Test-Path -LiteralPath $operation.Target) {Remove-Item -LiteralPath $operation.Target -Force}
 }
 throw
} finally {
 if (Test-Path -LiteralPath $staging) {Remove-Item -LiteralPath $staging -Recurse -Force}
}
