# Build the graph locally from the installed game. No game graph is downloaded.
param(
 [Parameter(Mandatory=$true)][string]$GamePath,
 [Parameter(Mandatory=$true)][string]$OutputPath
)
$ErrorActionPreference = 'Stop'
if (!('GRZDropGraphPboV1' -as [type])) { Add-Type -Path (Join-Path $PSScriptRoot 'GRZ-DropGraphTools.cs') }
if (!('GRZDropGraphPatchV030' -as [type])) { Add-Type -Path (Join-Path $PSScriptRoot 'GRZ-DropGraphPatch.cs') }
$archive = Join-Path $GamePath 'Addons/anims_workspaces.pbo'
if (!(Test-Path -LiteralPath $archive -PathType Leaf)) { throw "Native animation archive missing: $archive" }
$wanted = @{
 'dz/anims/workspaces/player/player_main/player_main.agr' = '89f82f6de2461cf42367b989b2b1392814a4df9272034fbb23be8e5c2106df23'
 'dz/anims/workspaces/player/player_main/locomotion.agf' = '8a7b56ff16aa36e71d15fd0b0563fcff7a2b4f672a67bab0c7011bd95ad1f63f'
}
$files = @{}
foreach ($entry in [GRZDropGraphPboV1]::Index($archive)) {
 if (!$wanted.ContainsKey($entry.Path)) { continue }
 if ($files.ContainsKey($entry.Path)) { throw 'Duplicate native graph path. Nothing built.' }
 [byte[]]$bytes = [GRZDropGraphPboV1]::Read($archive,$entry)
 if ([GRZDropGraphPatchV030]::Hash($bytes) -ne $wanted[$entry.Path]) { throw "Native graph differs from the supported DayZ 1.30.164.289 build: $($entry.Path). Nothing installed." }
 $files[$entry.Path] = $bytes
}
if ($files.Count -ne 2) { throw 'Required native player graphs not found. Nothing built.' }
$encoding = New-Object Text.UTF8Encoding($false,$true)
$base = 'dz/anims/workspaces/player/player_main/'
$root = [GRZDropGraphPatchV030]::Root($encoding.GetString($files[$base+'player_main.agr']))
$locomotion = [GRZDropGraphPatchV030]::Locomotion($encoding.GetString($files[$base+'locomotion.agf']))
[byte[]]$config = [IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'graph-config.cpp'))
[byte[]]$pbo = [GRZDropGraphPatchV030]::Pbo($config,$encoding.GetBytes($locomotion),$encoding.GetBytes($root))
if ([GRZDropGraphPatchV030]::Hash($pbo) -ne 'ff2941f78a5cc708d088a3ebe6b200461fa7d9af9f6120ed73fce9f81a7f83b7') {
 throw 'Generated graph does not match the checked v0.3.0 build. Nothing installed.'
}
[IO.File]::WriteAllBytes([IO.Path]::GetFullPath($OutputPath),$pbo)
Write-Host 'Built the checked v0.3.0 drop graph locally from your vanilla game files.'
