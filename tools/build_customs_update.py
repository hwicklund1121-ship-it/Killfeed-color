"""Ship a verified single-PBO Customs update using unchanged bytes from known installed packs."""
import hashlib,io,json
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
from build_pbo import build
from repair_hexgrid import read_pbo
root=Path(__file__).resolve().parents[1]
new=(root/'downloads/GRZ_Customs.pbo').read_bytes()
_,new_files=read_pbo(new)
new_start=len(new)-21-sum(map(len,new_files.values()))
payloads={}
def literal(data):
 name=hashlib.sha256(data).hexdigest()+'.bin'
 payloads[name]=data
 return dict(Kind='payload',File=name,Length=len(data))
header=literal(new[:new_start]);trailer=literal(new[-21:])
bases=[]
for version in ['0.1.9','0.2.0']:
 with ZipFile(root/f'downloads/GRZ-Customs-{version}.zip') as archive:old=archive.read('@GRZ_Customs/Addons/GRZ_Customs.pbo')
 _,old_files=read_pbo(old)
 at=len(old)-21-sum(map(len,old_files.values()));offsets={}
 for name,data in old_files.items():offsets[name]=at;at+=len(data)
 operations=[header]
 for name,data in new_files.items():
  if old_files.get(name)==data:operations.append(dict(Kind='base',Offset=offsets[name],Length=len(data)))
  else:operations.append(literal(data))
 operations.append(trailer)
 # Apply the exact transport recipe in memory and verify byte-for-byte identity.
 reconstructed=b''.join(old[o['Offset']:o['Offset']+o['Length']] if o['Kind']=='base' else payloads[o['File']] for o in operations)
 assert reconstructed==new
 bases.append(dict(Version=version,Sha256=hashlib.sha256(old).hexdigest(),Operations=operations))
manifest=dict(TargetVersion='0.2.1',TargetSha256=hashlib.sha256(new).hexdigest(),Bases=bases)
notes='''GRZ Customs 0.2.1 Xims update

Close DayZ and stop the server. Extract this ZIP into a NEW folder and run Update-GRZCustoms.cmd.
Updates BOTH installed @GRZ_Customs/Addons/GRZ_Customs.pbo files; no extra reskin PBO is added.
Supports the supplied Customs 0.1.9 or 0.2.0 pack. If your PBO is unrecognized, install 0.2.0 first.
Builds and verifies both complete PBOs before changing either installation. Old PBO/metadata backups are retained outside the active mod folders.
Existing server BAT, Editor Loader, saves and profile settings remain unchanged.

Added display names / spawn classes:
XIMS_Tracksuit pants / GRZ_XIMS_Tracksuit_Pants_BlackWhite
XIMS AirForces / GRZ_XIMS_AirForces_White
XIMS Hexgrid / GRZ_XIMS_Hexgrid_BlackWhite

Requires the original PITSTOP_AirForce1 addon on BOTH game and server, plus existing helmet, SK, M100 and HEXGRID dependencies.
rvk M100 and all previous Customs items remain included.
XIMS HEXGRID inherits the shared HEXGRID_Base armour. No armour values are overridden by this reskin.
Covered chest-hit calculations from full health remain .408 third hit / .50 cal second hit / VULCAN first hit, subject to distance, hit zone and other gear.
All supplied colour/normal maps converted at 4096x4096. Visual appearance and live spawning need in-game testing.

The full uncompressed PBO is larger than repository file limits; this package reconstructs the identical verified PBO using unchanged bytes already installed.
'''
(root/'XIMS-KIT-INSTALL.md').write_text(notes)
output=root/'downloads/GRZ-Customs-Xims-Update.zip'
with ZipFile(output,'w',ZIP_DEFLATED) as archive:
 for name,data in payloads.items():archive.writestr('Payload/'+name,data)
 archive.writestr('Update.json',json.dumps(manifest,separators=(',',':')))
 for name in ['Update-GRZCustoms.cmd','Update-GRZCustoms.ps1']:archive.write(root/'packaging/customs-update'/name,name)
 archive.writestr('MANIFEST.json',(root/'GRZ-CUSTOMS-MANIFEST.json').read_bytes())
 archive.writestr('mod.cpp','name = "GRZ Customs";\nauthor = "GRZ";\nversion = "0.2.1";\n')
 archive.writestr('INSTALL.txt',notes)
assert output.stat().st_size<100*1024*1024
with ZipFile(output) as archive:assert archive.testzip() is None
print('Verified transport recipes for BOTH supported base versions; update ZIP:',output.stat().st_size,'bytes.')
