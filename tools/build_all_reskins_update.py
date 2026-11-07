"""Bundle a small verified-download installer for the combined reskin/gameplay packs."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,subprocess,argparse
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--revision',required=True);a=p.parse_args()
assert len(a.revision)==40 and all(c in '0123456789abcdef' for c in a.revision)
manifest={'schema':1,'asset_revision':a.revision,'packages':[],'pending':['XIMS Layered Shirt: original Windstride config required.','XIMS Canvas Bag: smaller texture uploads required.']}
for filename,role in [('GRZ-Arena-Menu-Update.zip','core'),('GRZ-Vanilla-Clothing.zip','clothing'),('XIMS-Clothing.zip','xims')]:
 path=r/'downloads'/filename;data=path.read_bytes();addons={}
 with ZipFile(path) as z:
  for name in z.namelist():
   if '/' not in name and name.endswith('.pbo'):addons[name[:-4]]=hashlib.sha256(z.read(name)).hexdigest()
 assert addons
 manifest['packages'].append({'file':filename,'role':role,'url':'https://github.com/hwicklund1121-ship-it/Killfeed-color/raw/'+a.revision+'/downloads/'+filename,'sha256':hashlib.sha256(data).hexdigest(),'addons':addons})
assert len({k for pack in manifest['packages'] for k in pack['addons']})==17
notes='''GRZ + XIMS Reskin Update — available packs combined

Extract this small ZIP to its own folder. Close DayZ and stop the server. In PowerShell run:
powershell -ExecutionPolicy Bypass -File .\\Install-All-Reskins.ps1 -SteamId "YOUR_17_DIGIT_STEAM_ID"

Downloads three SHA256-verified archives from a pinned repository commit, merges their 17 PBOs and runs the existing transactional installer. Internet access is required. Alternatively place the three named ZIPs in one folder and add -PackageDirectory "C:\\path\\to\\zips" for offline installation. ServerPath/GamePath/ProfilePath/MissionName accept the same overrides as Install-Update.ps1. Windows Steam default paths are used otherwise.

Includes the prior arena/menu/gameplay update, CSR50, XIMS CS5, animated wraps/Hexgrids, six GRZ vanilla clothing pieces, XIMS Sneakers and XIMS Jean Shorts. Names/descriptions for the new XIMS clothes: XIMS Sneakers / XIMS Jean Shorts, Made By Blake. Clothing is discovered by the existing loadout after restarting client and server. Original weapon/HEXGRID and other existing dependencies remain required. Existing Customs PBOs are retained; this does not rebuild or replace your old consolidated GRZ_Customs addon.

Preserves profiles/VIP access/prefixes/ratings/saved outfits using the existing backup/rollback installer. Also includes the previously authorized POND DZE/Long Range duel update and appends the supplied Steam ID to the admin map-skip list. Installs all PBOs to @GRZ_Local/Addons on BOTH sides. Do not run the old and new clothing PBOs from different active folders.

Pending and intentionally NOT installed:
- XIMS Layered Shirt: model references Windstride Clothing. Layered_Shirt_Base is confirmed in public class lists, but the original Windstride config.cpp is needed to verify the addon dependency and texture bindings. Colour/normal textures are converted and staged; the female MLOD alone is not a complete ground/male/female addon.
- XIMS Canvas Bag: the uploaded ZIP exceeded the 32 MiB transfer limit. Resend colour/normal textures separately in ZIPs under 32 MiB each.
- GRZ Jeans and GRZ Bandana currently use their supplied colours plus vanilla normals. Their custom normal ZIPs were still oversized; 2048px copies or split uploads can finish those later.

New XIMS sneakers use native material detail because the supplied exported normal is flat. Jean shorts use the supplied custom baked normal and all-black colour. The GRZ cowboy hat is black and GRZ bandana white, exactly as supplied. No new clothing armour, durability, slots or storage changes.

Static/asset checks and local PowerShell fixture validation only. Live Windows downloads, DayZ compilation and model appearance require an in-game check.
'''
(r/'GRZ-ALL-RESKINS-UPDATE.md').write_text(notes)
pkg=r/'packaging/all-reskins-update'
(pkg/'RESKIN-PACKS.json').write_text(json.dumps(manifest,indent=2)+'\n')
with ZipFile(r/'downloads/GRZ-All-Reskins-Update.zip','w',ZIP_DEFLATED) as z:
 z.write(pkg/'Install-All-Reskins.ps1','Install-All-Reskins.ps1')
 z.writestr('RESKIN-PACKS.json',json.dumps(manifest,indent=2)+'\n');z.writestr('INSTALL.txt',notes)
print('Built combined verified-download installer for 17 addons; shirt and canvas bag explicitly pending.')
