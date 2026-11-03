param([string]$DayZPath)
$ErrorActionPreference = 'Stop'

# Reads PBO file tables and only copies the small requested lighting resources.
# Does not modify the game or upload anything.
function Read-PboString([System.IO.BinaryReader]$Reader) {
    $bytes = [System.Collections.Generic.List[byte]]::new()
    while ($true) {
        $next = $Reader.ReadByte()
        if ($next -eq 0) { break }
        $bytes.Add($next)
        if ($bytes.Count -gt 8192) { throw 'Invalid PBO string length.' }
    }
    return [System.Text.Encoding]::UTF8.GetString($bytes.ToArray())
}

if (-not $DayZPath) {
    Add-Type -AssemblyName System.Windows.Forms
    $dialog = New-Object System.Windows.Forms.FolderBrowserDialog
    $dialog.Description = 'Select your DayZ GAME folder (contains DayZ_x64.exe), not the server folder.'
    $dialog.ShowNewFolderButton = $false
    if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { exit 1 }
    $DayZPath = $dialog.SelectedPath
}
if (-not (Test-Path -LiteralPath (Join-Path $DayZPath 'DayZ_x64.exe'))) {
    throw 'Select the DayZ game installation folder that contains DayZ_x64.exe.'
}
$outputDirectory = Join-Path ([Environment]::GetFolderPath('Desktop')) 'GRZ-Sky-Lighting'
New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
$resourceMatches = [System.Collections.Generic.List[string]]::new()
$copied = [System.Collections.Generic.List[string]]::new()
$archives = @()
foreach ($folder in @('Addons', 'dta')) {
    $path = Join-Path $DayZPath $folder
    if (Test-Path -LiteralPath $path) {
        $archives += Get-ChildItem -LiteralPath $path -File -Filter '*.pbo' -Recurse
    }
}
foreach ($archive in $archives) {
    $reader = $null
    try {
        $stream = [System.IO.File]::Open($archive.FullName, 'Open', 'Read', 'ReadWrite')
        $reader = [System.IO.BinaryReader]::new($stream)
        $entries = [System.Collections.Generic.List[object]]::new()
        for ($index = 0; $index -lt 100000; $index++) {
            $name = Read-PboString $reader
            $method = $reader.ReadUInt32()
            $originalSize = $reader.ReadUInt32()
            $null = $reader.ReadUInt32()
            $null = $reader.ReadUInt32()
            $size = $reader.ReadUInt32()
            if ($name -eq '' -and $method -eq 0x56657273) {
                while ($true) {
                    $key = Read-PboString $reader
                    if ($key -eq '') { break }
                    $null = Read-PboString $reader
                }
                continue
            }
            if ($name -eq '') { break }
            $entries.Add([pscustomobject]@{ Name=$name; Method=$method; Size=$size })
        }
        $offset = $reader.BaseStream.Position
        foreach ($entry in $entries) {
            $normalized = $entry.Name.Replace('/', '\')
            if ($normalized -match '(^|\\)lighting_(default|darknight)\.txt$') {
                $resourceMatches.Add(('PBO: {0}; entry: {1}; method: {2}; bytes: {3}' -f $archive.Name,$entry.Name,$entry.Method,$entry.Size))
                if ($entry.Method -eq 0 -and $entry.Size -gt 0 -and $entry.Size -le 8388608) {
                    if ($offset + $entry.Size -gt $reader.BaseStream.Length) { throw 'Truncated PBO resource.' }
                    $null = $reader.BaseStream.Seek($offset, [System.IO.SeekOrigin]::Begin)
                    $data = $reader.ReadBytes([int]$entry.Size)
                    $baseName = [System.IO.Path]::GetFileName($normalized)
                    $destination = Join-Path $outputDirectory ($archive.BaseName + '-' + $baseName)
                    [System.IO.File]::WriteAllBytes($destination, $data)
                    $copied.Add($destination)
                    Write-Host ('Extracted: ' + $destination)
                } else {
                    $resourceMatches.Add('Compressed resource: send the named PBO (zipped) instead; this helper leaves it unchanged.')
                }
            }
            $offset += $entry.Size
        }
    } catch {
        Write-Warning ('Skipped ' + $archive.Name + ': ' + $_.Exception.Message)
    } finally {
        if ($reader) { $reader.Dispose() }
    }
}
if ($resourceMatches.Count -eq 0) {
    $resourceMatches.Add('No lighting_default.txt or lighting_darknight.txt found in the selected game Addons/dta PBO file tables.')
}
[System.IO.File]::WriteAllLines((Join-Path $outputDirectory 'Lighting-files.txt'), $resourceMatches.ToArray())
if ($copied.Count -gt 0) {
    Write-Host 'Done. Send the extracted lighting TXT files from Desktop\GRZ-Sky-Lighting.'
} else {
    Write-Host 'Send Desktop\GRZ-Sky-Lighting\Lighting-files.txt so the matching archive can be identified.'
}
Start-Process explorer.exe -ArgumentList ('"' + $outputDirectory + '"')
