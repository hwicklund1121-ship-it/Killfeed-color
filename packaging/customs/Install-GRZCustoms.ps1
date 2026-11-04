param(
    [string]$ServerPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZServer",
    [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
    [string]$BatName = 'StartServer.bat'
)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name DayZ_x64,DayZServer_x64 -ErrorAction SilentlyContinue) {
    throw 'Close DayZ and stop the server before installing.'
}
foreach ($target in @(@($ServerPath,'DayZServer_x64.exe'), @($GamePath,'DayZ_x64.exe'))) {
    if (!(Test-Path (Join-Path $target[0] $target[1]))) {
        throw "Could not find $($target[1]) in $($target[0]). Supply -ServerPath and -GamePath for your installations."
    }
}
$source = Join-Path $PSScriptRoot '@GRZ_Customs'
$manifestPath = Join-Path $source 'MANIFEST.json'
if (!(Test-Path $manifestPath)) { throw 'Missing pack manifest. Extract the entire new ZIP first.' }
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
if ($manifest.version -ne '0.1.7') { throw "This installer needs GRZ Customs 0.1.7; this folder contains $($manifest.version). Extract the latest ZIP into a NEW folder." }
Write-Host "Installing GRZ Customs $($manifest.version) - damage overrides removed."
$names = @('HEXGRID.pbo','GRZ_Customs_Hexgrid.pbo','Blakes_Kit.pbo','GRZ_Tracksuit_Pants.pbo','GRZ_Gunwraps.pbo')
$expectedHashes = @{
    'HEXGRID.pbo' = $manifest.base_pbo_sha256
    'GRZ_Customs_Hexgrid.pbo' = $manifest.pbo_sha256
}
foreach ($addon in $manifest.additional_addons.PSObject.Properties) {
    $expectedHashes[$addon.Name + '.pbo'] = $addon.Value.sha256
}
foreach ($name in $names) {
    if (!(Test-Path (Join-Path $source "Addons\$name"))) { throw "Extract the entire ZIP first. Missing $name." }
    $sourceHash = (Get-FileHash (Join-Path $source "Addons\$name") -Algorithm SHA256).Hash
    if ($sourceHash -ne $expectedHashes[$name]) { throw "The extracted $name does not match the new manifest. Extract the latest ZIP into a NEW folder." }
}
$bat = Join-Path $ServerPath $BatName
if (!(Test-Path $bat)) { throw "Cannot find $bat. Supply -BatName if your server BAT has a different name." }
$original = [IO.File]::ReadAllText($bat)
# Match either "-mod=a;b", -mod="a;b", or an unquoted -mod=a;b.
# Do not match -servermod, and preserve the rest of the existing launch command.
$pattern = '(?i)"-mod=([^"\r\n]*)"|(?<![\w-])-mod="([^"\r\n]*)"|(?<![\w-])-mod=([^\s"\r\n]+)'
$matchesFound = [regex]::Matches($original, $pattern)
if ($matchesFound.Count -ne 1) {
    throw 'Expected one -mod= argument in the server BAT. No changes made. Send your BAT so its launch format can be handled safely.'
}
$match = $matchesFound[0]
$group = 1
if ($match.Groups[2].Success) { $group = 2 }
if ($match.Groups[3].Success) { $group = 3 }
$mods = $match.Groups[$group].Value
if ($mods -match '(?i)(^|;)\s*@GRZ_Customs\s*(;|$)') {
    $updated = $original
} else {
    $replacement = '"-mod=' + $mods.TrimEnd(';') + ';@GRZ_Customs"'
    $updated = $original.Substring(0,$match.Index) + $replacement + $original.Substring($match.Index+$match.Length)
}
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
foreach ($target in @($ServerPath,$GamePath)) {
    $backup = Join-Path $target "GRZ-Customs-backup-$stamp"
    $destination = Join-Path $target '@GRZ_Customs'
    if (Test-Path $destination) {
        New-Item -ItemType Directory -Path $backup -Force | Out-Null
        Copy-Item -LiteralPath $destination -Destination $backup -Recurse
    }
    # Move duplicates of the five included addons outside active mod folders.
    foreach ($name in $names) {
        $old = Join-Path $target "@GRZ_Local\Addons\$name"
        if (Test-Path $old) {
            $oldBackup = Join-Path $backup 'Old-GRZ-Local-Addons'
            New-Item -ItemType Directory -Path $oldBackup -Force | Out-Null
            Move-Item -LiteralPath $old -Destination $oldBackup
            Get-ChildItem -LiteralPath (Split-Path $old) -Filter "$name.*.bisign" | Move-Item -Destination $oldBackup
        }
    }
    New-Item -ItemType Directory -Path $destination -Force | Out-Null
    Copy-Item -Path (Join-Path $source '*') -Destination $destination -Recurse -Force
    foreach ($name in $names) {
        $expected = (Get-FileHash (Join-Path $source "Addons\$name")).Hash
        $actual = (Get-FileHash (Join-Path $destination "Addons\$name")).Hash
        if ($expected -ne $actual) { throw "Copy verification failed: $target $name" }
    }
    Write-Host "Verified all five Customs PBOs in $target"
}
if ($updated -ne $original) {
    Copy-Item -LiteralPath $bat -Destination "$bat.before-customs-$stamp.bak"
    [IO.File]::WriteAllText($bat, $updated, [Text.Encoding]::Default)
}
Write-Host 'Server BAT includes @GRZ_Customs in -mod. Existing server options preserved.'
Write-Host 'CLIENT: DayZ Launcher > Mods > Local mod > select the DayZ game folder @GRZ_Customs, then CHECK it.'
Write-Host 'Also load the helmet and SK mods on both sides. Start the server using its updated BAT.'
Write-Host 'Search VPP for GRZ_Tracksuit_Pants or GRZ_Gunwraps_Red. Their vanilla bases need no SK/helmet addon.'
