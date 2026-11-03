param([string]$ServerPath = $PSScriptRoot, [string]$MissionName)
$ErrorActionPreference = 'Stop'
if (Get-Process -Name 'DayZServer_x64' -ErrorAction SilentlyContinue) {
    throw 'Stop DayZ Server before applying the empty-world economy setup.'
}
$serverRoot = [System.IO.Path]::GetFullPath($ServerPath)
if (-not $MissionName) {
    $configFile = Join-Path $serverRoot 'serverDZ.cfg'
    $text = [System.IO.File]::ReadAllText($configFile)
    $match = [regex]::Match($text, 'template\s*=\s*"([A-Za-z0-9_.-]+)"')
    if (-not $match.Success) { throw 'Mission template was not found in serverDZ.cfg.' }
    $MissionName = $match.Groups[1].Value
}
if ($MissionName -notmatch '^[A-Za-z0-9_][A-Za-z0-9_.-]*$') { throw 'Invalid mission name.' }
$missionsRoot = Join-Path $serverRoot 'mpmissions'
$missionRoot = [System.IO.Path]::GetFullPath((Join-Path $missionsRoot $MissionName))
if (-not (Test-Path -LiteralPath $missionRoot -PathType Container)) {
    throw ('Installed mission folder not found: ' + $missionRoot)
}
$plans = [System.Collections.Generic.List[object]]::new()
$typeCount = 0
$eventCount = 0
foreach ($file in Get-ChildItem -LiteralPath $missionRoot -Filter '*.xml' -Recurse -File) {
    # Parse all source economy XML safely; do not process persistence storage.
    $relative = $file.FullName.Substring($missionRoot.Length).TrimStart([char[]]@('\','/'))
    if ($relative -match '(^|[\\/])storage_[^\\/]+([\\/]|$)') { continue }
    $settings = New-Object System.Xml.XmlReaderSettings
    $settings.DtdProcessing = [System.Xml.DtdProcessing]::Prohibit
    $settings.XmlResolver = $null
    $probe = [System.Xml.XmlReader]::Create($file.FullName, $settings)
    try { $null = $probe.MoveToContent(); $rootName = $probe.LocalName } finally { $probe.Dispose() }
    if ($rootName -notin @('types','events','variables')) { continue }
    $document = New-Object System.Xml.XmlDocument
    $document.PreserveWhitespace = $true
    $document.XmlResolver = $null
    $reader = [System.Xml.XmlReader]::Create($file.FullName, $settings)
    try { $document.Load($reader) } finally { $reader.Dispose() }
    $changed = $false
    switch ($document.DocumentElement.LocalName) {
        'types' {
            foreach ($entry in $document.DocumentElement.SelectNodes('type')) {
                $typeCount++
                foreach ($name in @('nominal','min')) {
                    $value = $entry.SelectSingleNode($name)
                    if (-not $value) {
                        $value = $document.CreateElement($name)
                        $null = $entry.AppendChild($value)
                        $changed = $true
                    }
                    if ($value.InnerText -ne '0') { $value.InnerText = '0'; $changed = $true }
                }
            }
        }
        'events' {
            foreach ($entry in $document.DocumentElement.SelectNodes('event')) {
                $eventCount++
                $value = $entry.SelectSingleNode('active')
                if (-not $value) {
                    $value = $document.CreateElement('active')
                    $null = $entry.AppendChild($value)
                    $changed = $true
                }
                if ($value.InnerText -ne '0') { $value.InnerText = '0'; $changed = $true }
            }
        }
        'variables' {
            foreach ($value in $document.DocumentElement.SelectNodes('var')) {
                if ($value.GetAttribute('name') -in @('ZombieMaxCount','AnimalMaxCount')) {
                    if ($value.GetAttribute('value') -ne '0') { $value.SetAttribute('value','0'); $changed = $true }
                }
            }
        }
    }
    if ($changed) { $plans.Add([pscustomobject]@{ File=$file.FullName; Relative=$relative; Document=$document }) }
}
if ($typeCount -eq 0 -or $eventCount -eq 0) {
    throw 'Expected loot types and event definitions were not found. No changes were written.'
}
if ($plans.Count -gt 0) {
    $backup = Join-Path (Join-Path (Join-Path $serverRoot 'GRZ-EmptyWorld-Backups') (Get-Date -Format 'yyyyMMdd-HHmmss-fff')) $MissionName
    # Back up every planned file before writing any changes.
    foreach ($plan in $plans) {
        $target = Join-Path $backup $plan.Relative
        $null = New-Item -ItemType Directory -Path (Split-Path $target -Parent) -Force
        Copy-Item -LiteralPath $plan.File -Destination $target
    }
    foreach ($plan in $plans) { $plan.Document.Save($plan.File) }
    Write-Host ('GRZ Empty World: economy backup saved to ' + $backup)
}
Write-Host ('GRZ Empty World: floor loot targets zero (' + $typeCount + ' types); natural NPC/world events disabled (' + $eventCount + ' events).')

$inventoryOverride = Join-Path $serverRoot '@GRZ_Local/Addons/GRZ_InventorySizes.pbo'
if (Test-Path -LiteralPath $inventoryOverride) {
    Write-Host 'GRZ Inventory: generated override PBO is present in the server pack.'
} else {
    Write-Host 'GRZ Inventory: generated GRZ_InventorySizes.pbo is missing from server @GRZ_Local/Addons.'
}
