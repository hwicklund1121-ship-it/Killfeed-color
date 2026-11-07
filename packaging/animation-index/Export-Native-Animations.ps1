param(
 [string]$GamePath = "${env:ProgramFiles(x86)}\Steam\steamapps\common\DayZ",
 [string]$OutputPath = (Join-Path $PSScriptRoot 'GRZ-Native-Animations.json')
)
$ErrorActionPreference = 'Stop'
if (!(Test-Path -LiteralPath (Join-Path $GamePath 'DayZ_x64.exe'))) { throw 'Set -GamePath to your DayZ GAME folder.' }
if (!('GRZNativeAnimationIndex' -as [type])) {
 Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
public sealed class GRZNativeAnimationEntry {
 public string Archive;
 public string Path;
}
public static class GRZNativeAnimationIndex {
 static string ReadString(BinaryReader reader) {
  var data = new List<byte>();
  for (int i=0;i<65536;i++) {
   byte value=reader.ReadByte();
   if(value==0) return Encoding.UTF8.GetString(data.ToArray());
   data.Add(value);
  }
  throw new InvalidDataException("Oversized PBO header string.");
 }
 public static GRZNativeAnimationEntry[] Read(string archive) {
  var names=new List<string>();
  string prefix="";
  using(var reader=new BinaryReader(File.OpenRead(archive))) {
   bool complete=false;
   for(int i=0;i<100000;i++) {
    string name=ReadString(reader);
    uint method=reader.ReadUInt32();
    reader.ReadUInt32();reader.ReadUInt32();reader.ReadUInt32();reader.ReadUInt32();
    if(name.Length==0 && method==0x56657273) {
     for(int k=0;k<1000;k++) {
      string key=ReadString(reader);
      if(key.Length==0) break;
      string value=ReadString(reader);
      if(key.Equals("prefix",StringComparison.OrdinalIgnoreCase)) prefix=value;
      if(k==999) throw new InvalidDataException("Oversized PBO extension header.");
     }
     continue;
    }
    if(name.Length==0) {complete=true;break;}
    string ext=System.IO.Path.GetExtension(name);
    if(ext.Equals(".anm",StringComparison.OrdinalIgnoreCase) || ext.Equals(".agr",StringComparison.OrdinalIgnoreCase) || ext.Equals(".asi",StringComparison.OrdinalIgnoreCase)) names.Add(name);
   }
   if(!complete) throw new InvalidDataException("PBO entry header limit exceeded.");
   // Stop at the header terminator: no animation or config payload is read.
  }
  prefix=prefix.Replace('\\','/').Trim('/');
  var result=new List<GRZNativeAnimationEntry>();
  foreach(string name in names) {
   string path=name.Replace('\\','/').TrimStart('/');
   if(prefix.Length>0 && !path.StartsWith(prefix+"/",StringComparison.OrdinalIgnoreCase)) path=prefix+"/"+path;
   result.Add(new GRZNativeAnimationEntry { Archive=System.IO.Path.GetFileName(archive),Path=path });
  }
  return result.ToArray();
 }
}
'@
}
$rows = @()
$native = Join-Path $GamePath 'Addons'
if (!(Test-Path -LiteralPath $native -PathType Container)) { throw 'Native Addons folder missing.' }
foreach ($archive in @(Get-ChildItem -LiteralPath $native -Filter '*.pbo' -File | Sort-Object Name)) {
 $rows += @([GRZNativeAnimationIndex]::Read($archive.FullName))
}
if (!$rows.Count) { throw 'No native animation entries found. No index written.' }
$result = [ordered]@{Schema=1; Source='Vanilla DayZ Addons headers only'; Animations=@($rows | Sort-Object Archive,Path)}
$destination=[IO.Path]::GetFullPath($OutputPath)
[IO.File]::WriteAllText($destination,($result | ConvertTo-Json -Depth 5),(New-Object System.Text.UTF8Encoding($false)))
Write-Host "Saved $($rows.Count) animation filenames to $destination"
Write-Host 'Send GRZ-Native-Animations.json. It contains archive names and virtual animation paths only; no game assets, scripts, account IDs or computer folder paths.'
