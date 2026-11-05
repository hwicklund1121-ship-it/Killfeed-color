param(
    [string]$ToolsPath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ Tools",
    [string]$CompiledModelPath = '',
    [string]$GameDataPath = 'P:\dz'
)
$ErrorActionPreference = 'Stop'
$addonName = 'GRZ_Container_Openable'
$source = Join-Path $PSScriptRoot $addonName
$destination = Join-Path $PSScriptRoot 'BuildOutput'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
if (!(Test-Path (Join-Path $source 'model\container_openable.p3d'))) { throw 'Extract the entire builder ZIP before running this script.' }
if (!$CompiledModelPath) {
    $binarize = Join-Path $ToolsPath 'Bin\Binarize\binarize.exe'
    if (!(Test-Path $binarize)) { throw "DayZ Tools Binarize was not found: $binarize. Install DayZ Tools or pass -ToolsPath with its location." }
    # Resolve vanilla penetration/roadway resources from the existing DayZ
    # project drive without copying game data into the addon or output PBO.
    $link = Join-Path $PSScriptRoot 'dz'
    if (!(Test-Path $link) -and (Test-Path $GameDataPath)) {
        New-Item -ItemType Junction -Path $link -Target $GameDataPath | Out-Null
    }
    $modelOutput = Join-Path $destination 'BinarizedModel'
    New-Item -ItemType Directory -Path $modelOutput -Force | Out-Null
    $log = Join-Path $destination 'Binarize.log'
    Push-Location $PSScriptRoot
    try {
        $previousPreference = $ErrorActionPreference
        $ErrorActionPreference = 'Continue'
        # Standard Bohemia Binarize invocation: source dir, target dir, file.
        & $binarize '-norecurse' '-always' '-silent' '-maxProcesses=0' (Join-Path $source 'model') $modelOutput 'container_openable.p3d' *> $log
        $exitCode = $LASTEXITCODE
        $ErrorActionPreference = $previousPreference
    }
    finally {
        $ErrorActionPreference = 'Stop'
        Pop-Location
    }
    if ($exitCode -ne 0) { throw "DayZ Binarize failed ($exitCode). Read $log; ensure your DayZ Tools project drive/game data is configured." }
    $CompiledModelPath = Join-Path $modelOutput 'container_openable.p3d'
}
if (!(Test-Path $CompiledModelPath)) { throw "Compiled model not found: $CompiledModelPath" }
$modelBytes = [IO.File]::ReadAllBytes($CompiledModelPath)
if ($modelBytes.Length -lt 8 -or [Text.Encoding]::ASCII.GetString($modelBytes,0,4) -ne 'ODOL') {
    throw 'The model is still MLOD/uncompiled. No installable PBO was produced. Binarize it with model.cfg first.'
}

$entries = @()
foreach ($file in Get-ChildItem -LiteralPath $source -Recurse -File | Sort-Object FullName) {
    if ($file.Name -eq '$PBOPREFIX$' -or $file.Name -eq 'model.cfg') { continue }
    $relative = $file.FullName.Substring($source.Length).TrimStart([char[]]'\/').Replace('/','\')
    $bytes = [IO.File]::ReadAllBytes($file.FullName)
    if ($relative -eq 'model\container_openable.p3d') { $bytes = $modelBytes }
    $entries += [pscustomobject]@{Name=$relative;Bytes=$bytes}
}
$memory = New-Object IO.MemoryStream
$writer = New-Object IO.BinaryWriter($memory)
function Write-Z([string]$value) {
    $writer.Write([Text.Encoding]::UTF8.GetBytes($value))
    $writer.Write([byte]0)
}
function Write-Header([string]$name,[uint32]$method,[uint32]$length) {
    Write-Z $name
    $writer.Write($method)
    $writer.Write($length)
    $writer.Write([uint32]0)
    $writer.Write([uint32]0)
    $writer.Write($length)
}
Write-Header '' ([uint32]0x56657273) 0
Write-Z 'prefix'; Write-Z $addonName; Write-Z ''
foreach ($entry in $entries) { Write-Header $entry.Name 0 ([uint32]$entry.Bytes.Length) }
Write-Header '' 0 0
foreach ($entry in $entries) { $writer.Write([byte[]]$entry.Bytes) }
$writer.Flush()
$body = $memory.ToArray()
$sha = [Security.Cryptography.SHA1]::Create()
$checksum = $sha.ComputeHash($body)
$writer.Write([byte]0)
$writer.Write([byte[]]$checksum)
$writer.Flush()
$pbo = Join-Path $destination ($addonName+'.pbo')
[IO.File]::WriteAllBytes($pbo,$memory.ToArray())
$writer.Dispose(); $memory.Dispose(); $sha.Dispose()
Write-Host "Ready: $pbo"
Write-Host 'Copy this PBO into @GRZ_Local\Addons on BOTH client and server, restart both, and spawn GRZ_Container_Openable.'
Write-Host 'Test Open/Close at the doors, both sides of the doorway, and collision while walking through.'
