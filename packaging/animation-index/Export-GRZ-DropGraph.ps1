# Read-only collection of vanilla player animation graphs for the GRZ drop fix.
# Compatible with Windows PowerShell 5.1. Does not install or change any mods.
[CmdletBinding()]
param(
    [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
    [string]$OutputDirectory = $PSScriptRoot
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2
$game = [IO.Path]::GetFullPath($GamePath)
if (!(Test-Path -LiteralPath (Join-Path $game 'DayZ_x64.exe') -PathType Leaf)) {
    throw 'DayZ_x64.exe not found. Supply -GamePath pointing to your DayZ game folder.'
}
$native = Join-Path $game 'Addons'
if (!(Test-Path -LiteralPath $native -PathType Container)) { throw 'Vanilla Addons folder not found.' }
$destination = [IO.Path]::GetFullPath($OutputDirectory)
if (!(Test-Path -LiteralPath $destination -PathType Container)) { throw 'OutputDirectory must already exist.' }
# Never place the collection in a game or server folder.
if ($destination.Equals($game, [StringComparison]::OrdinalIgnoreCase) -or
    $destination.StartsWith($game.TrimEnd('\','/') + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Extract this exporter to Downloads or Desktop first; its output must be outside the DayZ folder.'
}
Add-Type -AssemblyName System.IO.Compression
if (!('GRZDropGraphPboV1' -as [type])) {
Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Text;
using System.Collections.Generic;

public sealed class GRZDropGraphEntry {
    public string Path;
    public string Archive;
    public uint Method;
    public uint OriginalSize;
    public uint StoredSize;
    public long Offset;
    public bool Graph;
}

public static class GRZDropGraphPboV1 {
    const uint Compressed = 0x43707273;
    static string CString(BinaryReader reader) {
        var data = new List<byte>();
        for (int i = 0; i < 65536; ++i) {
            byte value = reader.ReadByte();
            if (value == 0) return Encoding.UTF8.GetString(data.ToArray());
            data.Add(value);
        }
        throw new InvalidDataException("Oversized PBO header string.");
    }
    public static GRZDropGraphEntry[] Index(string archive) {
        var all = new List<GRZDropGraphEntry>();
        string prefix = "";
        using (var r = new BinaryReader(File.OpenRead(archive))) {
            bool ended = false;
            for (int i = 0; i < 100000; ++i) {
                string name = CString(r);
                uint method = r.ReadUInt32();
                uint original = r.ReadUInt32();
                r.ReadUInt32(); r.ReadUInt32();
                uint stored = r.ReadUInt32();
                if (name.Length == 0 && method == 0x56657273) {
                    bool extEnded = false;
                    for (int k = 0; k < 1000; ++k) {
                        string key = CString(r);
                        if (key.Length == 0) { extEnded = true; break; }
                        string value = CString(r);
                        if (key.Equals("prefix", StringComparison.OrdinalIgnoreCase)) prefix = value;
                    }
                    if (!extEnded) throw new InvalidDataException("PBO extension header limit exceeded.");
                    continue;
                }
                if (name.Length == 0) { ended = true; break; }
                all.Add(new GRZDropGraphEntry { Path = name, Archive = System.IO.Path.GetFileName(archive), Method = method, OriginalSize = original, StoredSize = stored });
            }
            if (!ended) throw new InvalidDataException("PBO header entry limit exceeded.");
            prefix = prefix.Replace('\\', '/').Trim('/').ToLowerInvariant();
            long offset = r.BaseStream.Position;
            var selected = new List<GRZDropGraphEntry>();
            foreach (var entry in all) {
                entry.Offset = offset;
                offset = checked(offset + entry.StoredSize);
                if (offset > r.BaseStream.Length) throw new InvalidDataException("PBO entry exceeds archive length.");
                string path = entry.Path.Replace('\\', '/').TrimStart('/').ToLowerInvariant();
                if (prefix.Length > 0 && !path.StartsWith(prefix + "/", StringComparison.Ordinal)) path = prefix + "/" + path;
                foreach (string part in path.Split('/')) {
                    if (part == ".." || part == "." || part.IndexOf(':') >= 0) throw new InvalidDataException("Invalid virtual path.");
                }
                entry.Path = path;
                string ext = System.IO.Path.GetExtension(path);
                bool graphExt = ext == ".agr" || ext == ".agf" || ext == ".asi" || ext == ".ast" || ext == ".asy";
                entry.Graph = path.StartsWith("dz/anims/workspaces/player/", StringComparison.Ordinal) && graphExt;
                if (entry.Graph || (path.StartsWith("dz/anims/anm/player/", StringComparison.Ordinal) && ext == ".anm")) selected.Add(entry);
            }
            return selected.ToArray();
        }
    }
    public static byte[] Read(string archive, GRZDropGraphEntry entry) {
        if (!entry.Graph) throw new InvalidDataException("Only native player graph payloads may be exported.");
        if (entry.StoredSize > 16 * 1024 * 1024 || entry.OriginalSize > 16 * 1024 * 1024) throw new InvalidDataException("Graph entry exceeds 16 MiB limit.");
        byte[] bytes;
        using (var r = new BinaryReader(File.OpenRead(archive))) {
            r.BaseStream.Position = entry.Offset;
            bytes = r.ReadBytes((int)entry.StoredSize);
        }
        if (bytes.Length != entry.StoredSize) throw new EndOfStreamException("Truncated graph entry.");
        if (entry.Method == 0) return bytes;
        if (entry.Method != Compressed) throw new InvalidDataException("Unsupported PBO compression method.");
        var output = new byte[entry.OriginalSize];
        int src = 0, dst = 0;
        uint checksum = 0;
        while (dst < output.Length) {
            if (src >= bytes.Length - 4) throw new InvalidDataException("Truncated compressed graph.");
            int flags = bytes[src++];
            for (int bit = 0; bit < 8 && dst < output.Length; ++bit) {
                if ((flags & (1 << bit)) != 0) {
                    if (src >= bytes.Length - 4) throw new InvalidDataException("Truncated compressed literal.");
                    byte value = bytes[src++]; output[dst++] = value;
                    checksum = unchecked(checksum + value);
                } else {
                    if (src + 2 > bytes.Length - 4) throw new InvalidDataException("Truncated compressed reference.");
                    int first = bytes[src++], second = bytes[src++];
                    int distance = first | ((second & 0xf0) << 4);
                    int count = (second & 0xf) + 3;
                    if (distance == 0 || dst + count > output.Length) throw new InvalidDataException("Invalid compressed reference.");
                    for (int k = 0; k < count; ++k) {
                        int index = dst - distance;
                        byte value = index < 0 ? (byte)32 : output[index];
                        output[dst++] = value; checksum = unchecked(checksum + value);
                    }
                }
            }
        }
        if (src + 4 != bytes.Length || BitConverter.ToUInt32(bytes, src) != checksum) throw new InvalidDataException("Graph decompression checksum mismatch.");
        return output;
    }
}
'@
}
$archives = @(Get-ChildItem -LiteralPath $native -Filter '*.pbo' -File | Sort-Object Name)
$graphs = New-Object 'System.Collections.Generic.List[object]'
$animations = New-Object 'System.Collections.Generic.List[object]'
$paths = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::OrdinalIgnoreCase)
foreach ($archive in $archives) {
    foreach ($entry in [GRZDropGraphPboV1]::Index($archive.FullName)) {
        if ($entry.Graph) {
            if (!$paths.Add($entry.Path)) { throw "Duplicate native graph path: $($entry.Path). No collection created." }
            $graphs.Add([pscustomobject]@{ ArchivePath=$archive.FullName; Entry=$entry })
        } else { $animations.Add([ordered]@{ Archive=$entry.Archive; Path=$entry.Path }) }
    }
}
if (!$graphs.Count) { throw 'No native player graphs found. No collection created.' }
if ($graphs.Count -gt 1000) { throw 'Unexpected graph count. No collection created.' }
$stamp = [guid]::NewGuid().ToString('N').Substring(0,8)
$zipPath = Join-Path $destination ("GRZ-DropGraph-$stamp.zip")
$manifest = New-Object 'System.Collections.Generic.List[object]'
$stream = $null; $zip = $null; $complete = $false
try {
    $stream = [IO.File]::Open($zipPath, [IO.FileMode]::CreateNew)
    $zip = New-Object IO.Compression.ZipArchive($stream, [IO.Compression.ZipArchiveMode]::Create, $true)
    [long]$total = 0
    foreach ($item in $graphs) {
        $entry = $item.Entry
        [byte[]]$bytes = [GRZDropGraphPboV1]::Read($item.ArchivePath, $entry)
        $total += $bytes.Length
        if ($total -gt 28MB) { throw 'Player graphs exceed the 28 MiB collection limit.' }
        $hasher = [Security.Cryptography.SHA256]::Create()
        try { $hash = ([BitConverter]::ToString($hasher.ComputeHash($bytes))).Replace('-','').ToLowerInvariant() }
        finally { $hasher.Dispose() }
        $file = $zip.CreateEntry(('native/' + $entry.Path), [IO.Compression.CompressionLevel]::Optimal)
        $writer = $file.Open()
        try { $writer.Write($bytes, 0, $bytes.Length) } finally { $writer.Dispose() }
        $manifest.Add([ordered]@{ Archive=$entry.Archive; Path=$entry.Path; Bytes=$bytes.Length; SHA256=$hash })
    }
    $exe = Get-Item -LiteralPath (Join-Path $game 'DayZ_x64.exe')
    $report = [ordered]@{
        Schema=1; Purpose='GRZ gun-lift to prone transition investigation'
        Source='Installed vanilla DayZ Addons only'; GameVersion=$exe.VersionInfo.FileVersion
        Graphs=@($manifest.ToArray()); Animations=@($animations.ToArray())
    }
    [byte[]]$json = [Text.Encoding]::UTF8.GetBytes(($report | ConvertTo-Json -Depth 6))
    $writer = $zip.CreateEntry('MANIFEST.json').Open()
    try { $writer.Write($json, 0, $json.Length) } finally { $writer.Dispose() }
    $zip.Dispose(); $zip = $null
    $stream.Dispose(); $stream = $null
    $complete = $true
} finally {
    if ($null -ne $zip) { $zip.Dispose() }
    if ($null -ne $stream) { $stream.Dispose() }
    if (!$complete -and (Test-Path -LiteralPath $zipPath)) { Remove-Item -LiteralPath $zipPath }
}
Write-Host "Saved: $zipPath"
Write-Host "Collected $($graphs.Count) vanilla player graph files and $($animations.Count) animation filenames."
Write-Host 'Upload the GRZ-DropGraph-*.zip to this chat.'
Write-Host 'Game/mod files were not changed. No profiles, passwords, or account IDs were collected.'
