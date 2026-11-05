param(
 [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ"
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) { throw 'Close DayZ and stop the server before updating.' }
$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'Update.json') -Raw | ConvertFrom-Json
$workRoot = Join-Path ([IO.Path]::GetTempPath()) ('GRZ-Customs-Xims-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $workRoot | Out-Null
$prepared = @()
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
function Copy-Range($inputStream, $outputStream, [long]$offset, [long]$count) {
 $null = $inputStream.Seek($offset, [IO.SeekOrigin]::Begin)
 $buffer = New-Object byte[] 65536
 while ($count -gt 0) {
  $amount = [int][Math]::Min($buffer.Length, $count)
  $read = $inputStream.Read($buffer, 0, $amount)
  if ($read -le 0) { throw 'Update data ended unexpectedly.' }
  $outputStream.Write($buffer, 0, $read)
  $count -= $read
 }
}
try {
 foreach ($root in @($ServerPath,$GamePath)) {
  $path = Join-Path $root '@GRZ_Customs\Addons\GRZ_Customs.pbo'
  if (!(Test-Path -LiteralPath $path)) { throw "Missing $path. Install GRZ Customs 0.2.0 first, then run this update." }
  $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
  if ($hash -eq $manifest.TargetSha256) { Write-Host "GRZ Customs 0.2.1 already installed in $root"; continue }
  $plan = @($manifest.Bases | Where-Object { $_.Sha256 -eq $hash })
  if ($plan.Count -ne 1) { throw "Unrecognized Customs PBO in $root. This update supports the supplied 0.1.9 and 0.2.0 packs. Install the supplied 0.2.0 pack first. No changes made." }
  $built = Join-Path $workRoot ($prepared.Count.ToString() + '.pbo')
  $inputStream = [IO.File]::OpenRead($path)
  $outputStream = [IO.File]::Create($built)
  try {
   foreach ($operation in $plan[0].Operations) {
    if ($operation.Kind -eq 'base') {
     Copy-Range $inputStream $outputStream ([long]$operation.Offset) ([long]$operation.Length)
    } else {
     $payload = Join-Path $PSScriptRoot ('Payload\' + $operation.File)
     $payloadStream = [IO.File]::OpenRead($payload)
     try { Copy-Range $payloadStream $outputStream 0 ([long]$operation.Length) } finally { $payloadStream.Dispose() }
    }
   }
  } finally { $inputStream.Dispose(); $outputStream.Dispose() }
  $newHash = (Get-FileHash -LiteralPath $built -Algorithm SHA256).Hash.ToLowerInvariant()
  if ($newHash -ne $manifest.TargetSha256) { throw 'Rebuilt Customs PBO failed verification. No installed files changed.' }
  $backup = Join-Path $root ('GRZ-Customs-Xims-Backup-' + $stamp)
  $prepared += [pscustomobject]@{ Path=$path; Built=$built; Backup=$backup; ModRoot=(Join-Path $root '@GRZ_Customs') }
 }
 # Both complete new PBOs have been verified before installed files are touched.
 foreach ($target in $prepared) {
  New-Item -ItemType Directory -Path $target.Backup -Force | Out-Null
  Copy-Item -LiteralPath $target.Path -Destination (Join-Path $target.Backup 'GRZ_Customs.pbo')
  foreach ($name in @('MANIFEST.json','mod.cpp')) {
   $oldMetadata = Join-Path $target.ModRoot $name
   if (Test-Path -LiteralPath $oldMetadata) { Copy-Item -LiteralPath $oldMetadata -Destination (Join-Path $target.Backup $name) }
  }
 }
 try {
  foreach ($target in $prepared) {
   Copy-Item -LiteralPath $target.Built -Destination $target.Path -Force
   if ((Get-FileHash -LiteralPath $target.Path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $manifest.TargetSha256) { throw 'Installed PBO hash mismatch.' }
   foreach ($name in @('MANIFEST.json','mod.cpp')) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $target.ModRoot $name) -Force }
  }
 } catch {
  foreach ($target in $prepared) {
   Copy-Item -LiteralPath (Join-Path $target.Backup 'GRZ_Customs.pbo') -Destination $target.Path -Force
   foreach ($name in @('MANIFEST.json','mod.cpp')) {
    $oldMetadata = Join-Path $target.Backup $name
    if (Test-Path -LiteralPath $oldMetadata) { Copy-Item -LiteralPath $oldMetadata -Destination (Join-Path $target.ModRoot $name) -Force }
   }
  }
  throw
 }
 Write-Host 'GRZ Customs 0.2.1 verified on BOTH game and server. Existing server startup options unchanged.'
 Write-Host 'Added XIMS_Tracksuit pants, XIMS AirForces and XIMS Hexgrid. rvk M100 and existing gear retained.'
 Write-Host 'Load the Pitstop AirForce mod on BOTH sides. Keep the existing helmet, SK and M100 dependencies loaded.'
 Write-Host 'Restart both. Search VPP/loadout by the display names above.'
} finally { if (Test-Path -LiteralPath $workRoot) { Remove-Item -LiteralPath $workRoot -Recurse -Force } }
