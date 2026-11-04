param([string[]]$InstallPaths = @())
$ErrorActionPreference = 'Stop'
if (Get-Process -Name 'DayZ_x64', 'DayZServer_x64' -ErrorAction SilentlyContinue) {
    throw 'Close DayZ and stop the DayZ server before removing the old scope overrides.'
}
if ($InstallPaths.Count -eq 0) {
    $steamCommon = Join-Path ${env:ProgramFiles(x86)} 'Steam\steamapps\common'
    $InstallPaths = @($PSScriptRoot, (Join-Path $steamCommon 'DayZ'), (Join-Path $steamCommon 'DayZServer'))
}
foreach ($installation in ($InstallPaths | Select-Object -Unique)) {
    $addons = Join-Path $installation '@GRZ_Local\Addons'
    foreach ($filename in @('GRZ_Quickscope.pbo', 'GRZ_AimControl.pbo')) {
        $path = Join-Path $addons $filename
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            Remove-Item -LiteralPath $path -Force
            Write-Host ('Removed old scope override: ' + $path)
        }
    }
}
Write-Host 'Keep the supplied replacement GRZ_Hitscan.pbo: it removes the old damage-blocking code.'
Write-Host 'Custom weapon PBOs, inventory overrides and player saves were not changed.'
