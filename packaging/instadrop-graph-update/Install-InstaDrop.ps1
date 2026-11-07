param(
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ"
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) { throw 'Close DayZ and stop the server first.' }
$names = @('GRZ_InstaDrop.pbo','GRZ_InstaDropData.pbo','GRZ_InstaDropGraph.pbo')
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'MANIFEST.json') -Raw | ConvertFrom-Json
foreach ($name in @('GRZ_InstaDrop.pbo','GRZ_InstaDropData.pbo')) {
 if ((Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $name) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.$name) { throw "Package hash mismatch: $name. Nothing installed." }
}
foreach ($side in @(@($ServerPath,'DayZServer_x64.exe'),@($GamePath,'DayZ_x64.exe'))) {
 if (!(Test-Path -LiteralPath (Join-Path $side[0] $side[1]) -PathType Leaf)) { throw "Game executable missing in $($side[0])" }
 $exe = Get-Item -LiteralPath (Join-Path $side[0] $side[1])
 $version = [string]$exe.VersionInfo.FileVersion
 if (($version -replace '\D','') -ne '130164289') { throw "This graph targets DayZ 1.30.164.289. Found '$version' in $($side[0]). Nothing installed." }
 $addons = Join-Path $side[0] '@GRZ_Local/Addons' 
 if (!(Test-Path -LiteralPath $addons -PathType Container)) { throw "Existing @GRZ_Local/Addons folder missing in $($side[0])" }
 if (!(Test-Path -LiteralPath (Join-Path $addons 'GRZ_Movement.pbo') -PathType Leaf)) { throw "Existing GRZ Movement missing in $addons. Install into the existing GRZ Local movement setup." }
 if (@(Get-ChildItem -LiteralPath $addons -Filter '*LUGU*' -File).Count -gt 0) { throw "LUGU addon found in $addons. Remove it from the active addon folder before testing GRZ. Nothing installed." }
}
& (Join-Path $PSScriptRoot 'Build-InstaDropGraph.ps1') -GamePath $GamePath -OutputPath (Join-Path $PSScriptRoot 'GRZ_InstaDropGraph.pbo')
foreach ($name in $names) {
 if ((Get-FileHash -LiteralPath (Join-Path $PSScriptRoot $name) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.$name) { throw "Package hash mismatch: $name. Nothing installed." }
}
Write-Host 'Test build for the existing @GRZ_Local setup. This installer does not edit startup scripts.' 
$backup = Join-Path $ServerPath ('GRZ-instadrop-backup-' + [guid]::NewGuid().ToString('N'))
$operations = @()
$written = @()
try {
 New-Item -ItemType Directory -Path $backup | Out-Null
 foreach ($side in @(@('server',$ServerPath),@('client',$GamePath))) {
  foreach ($name in $names) {
   $target = Join-Path $side[1] ('@GRZ_Local/Addons/' + $name)
   if (Test-Path -LiteralPath $target -PathType Container) { throw "Target is a directory: $target" }
   if (@(Get-ChildItem -LiteralPath (Split-Path $target) -Filter ($name + '*.bisign') -File).Count -gt 0) { throw "Existing signature would be stale: $target.bisign. Install a signed release instead." }
   $exists = Test-Path -LiteralPath $target -PathType Leaf
   $saved = Join-Path $backup ($side[0] + '-' + $name)
   if ($exists) { Copy-Item -LiteralPath $target -Destination $saved }
   $operations += [pscustomobject]@{Target=$target; Saved=$saved; Existed=$exists; Source=(Join-Path $PSScriptRoot $name)}
  }
 }
 foreach ($operation in $operations) {
  $written += $operation
  Copy-Item -LiteralPath $operation.Source -Destination $operation.Target -Force
  if ((Get-FileHash -LiteralPath $operation.Target -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath $operation.Source -Algorithm SHA256).Hash) { throw 'Installed PBO verification failed.' }
 }
 $operations | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $backup 'RESTORE.json') -Encoding UTF8
 Write-Host "GRZ InstaDrop prototype installed on BOTH sides. Backups: $backup"
 Write-Host 'Restart both. Test G then prone, and getting back up with your normal stance keys. v0.1.6 retains the roll-to-stand source and selects a patched native graph for direct gun-lift-to-prone. GRZ Movement is not replaced, and its old drop-speed requests are disabled by the data file.'
 Write-Host 'Check client/server script logs for [GRZ InstaDrop] v0.1.6 graph selected. Actual native graph loading and drop/roll playback need in-game verification.'
} catch {
 $originalFailure = $_
 foreach ($operation in $written) {
  try {
   if ($operation.Existed) { Copy-Item -LiteralPath $operation.Saved -Destination $operation.Target -Force }
   elseif (Test-Path -LiteralPath $operation.Target -PathType Leaf) { Remove-Item -LiteralPath $operation.Target -Force }
  } catch { Write-Warning "Rollback failed for $($operation.Target). Restore its backup from $backup" }
 }
 throw $originalFailure
}
