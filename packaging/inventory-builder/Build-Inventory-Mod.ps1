param(
    [string]$DayZPath,
    [string[]]$ExtraModPaths = @(),
    [string[]]$ExtraPboPaths = @(),
    [string]$OutputPath,
    [switch]$NoExplorer
)
$ErrorActionPreference = 'Stop'
$interactiveSelection = -not $DayZPath
if (-not $DayZPath) {
    Add-Type -AssemblyName System.Windows.Forms
    $dialog = New-Object System.Windows.Forms.FolderBrowserDialog
    $dialog.Description = 'Select the DayZ GAME installation folder containing DayZ_x64.exe.'
    $dialog.ShowNewFolderButton = $false
    if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { exit 1 }
    $DayZPath = $dialog.SelectedPath
}
if (-not (Test-Path -LiteralPath (Join-Path $DayZPath 'DayZ_x64.exe'))) {
    throw 'Choose the DayZ GAME folder containing DayZ_x64.exe.'
}
if (-not $OutputPath) {
    $OutputPath = Join-Path ([Environment]::GetFolderPath('Desktop')) 'GRZ-InventorySizes'
}
$archives = [System.Collections.Generic.List[string]]::new()
$folders = @((Join-Path $DayZPath 'Addons'), (Join-Path $DayZPath 'dta')) + $ExtraModPaths
foreach ($folder in $folders) {
    if (-not (Test-Path -LiteralPath $folder -PathType Container)) {
        throw ('Folder not found: ' + $folder)
    }
    foreach ($file in Get-ChildItem -LiteralPath $folder -Filter '*.pbo' -File -Recurse) {
        $archives.Add($file.FullName)
    }
}
if ($interactiveSelection -and $ExtraPboPaths.Count -eq 0 -and $ExtraModPaths.Count -eq 0) {
    $extraDialog = New-Object System.Windows.Forms.OpenFileDialog
    $extraDialog.Title = 'Select extra gun/clothing PBOs, e.g. UAVWEAPONS.pbo (Cancel for vanilla only)'
    $extraDialog.Filter = 'DayZ addon PBO (*.pbo)|*.pbo'
    $extraDialog.Multiselect = $true
    $extraDialog.InitialDirectory = $DayZPath
    if ($extraDialog.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) {
        $ExtraPboPaths = $extraDialog.FileNames
    }
}
foreach ($pbo in $ExtraPboPaths) {
    if (-not (Test-Path -LiteralPath $pbo -PathType Leaf) -or [System.IO.Path]::GetExtension($pbo) -ne '.pbo') {
        throw ('Additional PBO file not found: ' + $pbo)
    }
    $archives.Add([System.IO.Path]::GetFullPath($pbo))
}
if ($archives.Count -eq 0) { throw 'No PBO archives were found.' }
Write-Host 'GRZ Inventory builder v0.1.3'
Write-Host 'Reading installed clothing and gun class definitions. Game files remain unchanged...'
if (-not ('GRZInventoryBuilder' -as [type])) {
    Add-Type -Path (Join-Path $PSScriptRoot 'InventoryBuilder.cs')
}
foreach ($pbo in $ExtraPboPaths) {
    $exported = [GRZInventoryBuilder]::ExportConfig($pbo, (Join-Path $OutputPath 'ExtraConfigs'))
    if ($exported) { Write-Host ('Extra mod config copy: ' + $exported) }
}
try {
    $report = [GRZInventoryBuilder]::Build($archives.ToArray(), $OutputPath)
} catch {
    if ($ExtraPboPaths.Count -gt 0) {
        Write-Host ('If this extra mod config is unsupported, send the small files from ' + (Join-Path $OutputPath 'ExtraConfigs') + ' for integration. No new PBO was built.')
    }
    throw
}
$report | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $OutputPath 'Inventory-catalog.json') -Encoding UTF8
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'INSTALL.txt') -Destination (Join-Path $OutputPath 'INSTALL.txt') -Force
Write-Host ('Clothing classes: ' + $report.Clothing.Count + ' -> 10 wide x 10 high CARGO')
Write-Host ('Other clothing classes kept at original cargo: ' + $report.NormalClothing.Count)
Write-Host ('Gun classes: ' + $report.Guns.Count + ' -> 10 wide x 2 high ITEM SIZE')
Write-Host ('Ready PBO: ' + $report.PboPath)
Write-Host 'Copy GRZ_InventorySizes.pbo into @GRZ_Local\Addons on BOTH client and server, then restart both.'
if (-not $NoExplorer) {
    Start-Process explorer.exe -ArgumentList ('"' + $OutputPath + '"')
}
