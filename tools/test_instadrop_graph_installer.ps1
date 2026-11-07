param([Parameter(Mandatory=$true)][string]$PackageZip, [Parameter(Mandatory=$true)][string]$NativeArchive)
$ErrorActionPreference = 'Stop'
$root = Join-Path ([IO.Path]::GetTempPath()) ('GRZ-DropInstallerTest-' + [guid]::NewGuid().ToString('N'))
$package = Join-Path $root 'package'
New-Item -ItemType Directory -Path $package -Force | Out-Null
Expand-Archive -LiteralPath $PackageZip -DestinationPath $package
$game = Join-Path $root 'game'
$server = Join-Path $root 'server'
$global:GRZTestFakeVersion = '1.30.164.289'
$global:GRZTestFailAt = -1
$global:GRZTestWrites = 0
function Get-Process { param($Name, $ErrorAction) return $null }
# Only Windows executable metadata and running-process detection are mocked.
# File hashes, copying, backups, rollback and restore execute on real fixtures.
function Get-Item {
 param([string]$LiteralPath)
 if ($LiteralPath.EndsWith('.exe')) { return [pscustomobject]@{ VersionInfo=[pscustomobject]@{ FileVersion=$global:GRZTestFakeVersion } } }
 Microsoft.PowerShell.Management\Get-Item -LiteralPath $LiteralPath
}
function Copy-Item {
 [CmdletBinding()]
 param([string]$LiteralPath, [string]$Destination, [switch]$Force)
 if ((Split-Path $LiteralPath) -eq $package -and $LiteralPath.EndsWith('.pbo')) {
  $global:GRZTestWrites++
  if ($global:GRZTestWrites -eq $global:GRZTestFailAt) { throw 'Injected copy failure' }
 }
 Microsoft.PowerShell.Management\Copy-Item -LiteralPath $LiteralPath -Destination $Destination -Force:$Force
}
function Assert($value, $message) { if (!$value) { throw $message } }
function Reset-Fixture {
 foreach ($p in @($game,$server)) {
  if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Recurse -Force }
  $addons = Join-Path $p '@GRZ_Local/Addons'
  New-Item -ItemType Directory -Path $addons -Force | Out-Null
  [IO.File]::WriteAllText((Join-Path $addons 'GRZ_Movement.pbo'),'unchanged-movement')
  [IO.File]::WriteAllText((Join-Path $addons 'GRZ_InstaDrop.pbo'),'original-stance')
  [IO.File]::WriteAllText((Join-Path $addons 'GRZ_InstaDropData.pbo'),'original-data')
 }
 New-Item -ItemType Directory -Path (Join-Path $game 'Addons') -Force | Out-Null
 Copy-Item -LiteralPath $NativeArchive -Destination (Join-Path $game 'Addons/anims_workspaces.pbo')
 [IO.File]::WriteAllText((Join-Path $game 'DayZ_x64.exe'),'game-exe')
 [IO.File]::WriteAllText((Join-Path $server 'DayZServer_x64.exe'),'server-exe')
 $global:GRZTestWrites = 0
 $global:GRZTestFailAt = -1
 $global:GRZTestFakeVersion = '1.30.164.289'
}
function Assert-Originals {
 foreach ($p in @($game,$server)) {
  $addons = Join-Path $p '@GRZ_Local/Addons'
  Assert ((Get-Content -LiteralPath (Join-Path $addons 'GRZ_InstaDrop.pbo') -Raw) -eq 'original-stance') 'Stance not restored'
  Assert ((Get-Content -LiteralPath (Join-Path $addons 'GRZ_InstaDropData.pbo') -Raw) -eq 'original-data') 'Data not restored'
  Assert (!(Test-Path -LiteralPath (Join-Path $addons 'GRZ_InstaDropGraph.pbo'))) 'New graph not removed'
  Assert ((Get-Content -LiteralPath (Join-Path $addons 'GRZ_Movement.pbo') -Raw) -eq 'unchanged-movement') 'Movement was changed'
 }
}
function Install { & (Join-Path $package 'Install-InstaDrop.ps1') -GamePath $game -ServerPath $server }

Reset-Fixture
Install
$manifest = Get-Content -LiteralPath (Join-Path $package 'MANIFEST.json') -Raw | ConvertFrom-Json
foreach ($p in @($game,$server)) {
 foreach ($name in @('GRZ_InstaDrop.pbo','GRZ_InstaDropData.pbo','GRZ_InstaDropGraph.pbo')) {
  $hash=(Get-FileHash -LiteralPath (Join-Path $p ('@GRZ_Local/Addons/'+$name))).Hash.ToLowerInvariant()
  Assert ($hash -eq $manifest.$name) 'Installed hash mismatch'
 }
}
$backup = @(Get-ChildItem -LiteralPath $server -Directory -Filter 'GRZ-instadrop-backup-*')[0].FullName
& (Join-Path $package 'Restore-InstaDrop.ps1') -BackupPath $backup
Assert-Originals

Reset-Fixture
$global:GRZTestFailAt=4
$caught=$false
try { Install } catch { $caught=$true; Assert ($_.Exception.Message -match 'Injected copy failure') 'Unexpected error' }
Assert $caught 'Copy failure did not fail install'
Assert-Originals

Reset-Fixture
$global:GRZTestFakeVersion='1.31.999.999'
$caught=$false
try { Install } catch { $caught=$true; Assert ($_.Exception.Message -match 'targets DayZ') 'Unexpected version error' }
Assert $caught 'Incorrect version accepted'
Assert ($global:GRZTestWrites -eq 0) 'Wrong version changed files'
Assert-Originals

Reset-Fixture
[IO.File]::WriteAllText((Join-Path $game '@GRZ_Local/Addons/GRZ_InstaDrop.pbo.owner.bisign'),'signature')
$caught=$false
try { Install } catch { $caught=$true; Assert ($_.Exception.Message -match 'signature would be stale') 'Unexpected signature error' }
Assert $caught 'Stale signature accepted'
Assert ($global:GRZTestWrites -eq 0) 'Signed addon changed before rejection'
Assert-Originals

Reset-Fixture
$pbo=Join-Path $package 'GRZ_InstaDrop.pbo'
$bytes=[IO.File]::ReadAllBytes($pbo)
try {
 [IO.File]::AppendAllText($pbo,'corruption')
 $caught=$false
 try { Install } catch { $caught=$true; Assert ($_.Exception.Message -match 'Package hash mismatch') 'Unexpected hash error' }
 Assert $caught 'Corrupted package accepted'
 Assert ($global:GRZTestWrites -eq 0) 'Corrupt package changed files'
 Assert-Originals
} finally { [IO.File]::WriteAllBytes($pbo,$bytes) }
Write-Host 'PASS: two-sided install/hash verification, backup/restore, injected-copy rollback, version rejection, signature rejection, and package-corruption rejection.'

Remove-Item -LiteralPath $root -Recurse -Force
