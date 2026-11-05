"""Correct the supplied CS5 .50-cal ammo references and detachable-magazine config."""
import argparse
import hashlib
import json
import re
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('source_zip', type=Path)
args = parser.parse_args()
config_name = 'CS5/CS5/config.cpp'
with ZipFile(args.source_zip) as archive:
    files = {n:archive.read(n) for n in archive.namelist() if not n.endswith('/')}
original = dict(files)
config = files[config_name].decode('utf-8-sig')
config, chamber = re.subn(r'chamberableFrom\[\]\s*=\s*\{[^}]*\};', 'chamberableFrom[] = {"UAV_Ammo_50CAL"};', config)
config, ammo_items = re.subn(r'ammoItems\[\]\s*=\s*\{[^}]*\};', 'ammoItems[] = {"UAV_Ammo_50CAL"};', config)
assert chamber == ammo_items == 1
assert 'class CS5_Base: BoltActionRifle_InnerMagazine_Base' in config
config = config.replace('class BoltActionRifle_InnerMagazine_Base;', 'class BoltActionRifle_Base;').replace('class CS5_Base: BoltActionRifle_InnerMagazine_Base', 'class CS5_Base: BoltActionRifle_Base')
config = config.replace('Holds up to 10 rounds of .408', 'Holds up to 10 rounds of .50 CAL').replace('with .50CAL Win rounds', 'with .50 CAL rounds')
assert 'ammo = "UAV_Bullet_50CAL";' in config
assert 'magazines[] = {"CS5_Magazine_10RND"};' in config
assert 'class UAV_CS5_BLACK: CS5_Base' in config
files[config_name] = config.encode()
notes = '''CS5 .50 CAL correction

Replace the complete CS5 folder in your unpacked UAVWEAPONS pack with this CS5 folder.
Repack the weapon pack, update BOTH game and server copies, and restart both.
Spawn a fresh UAV_CS5_BLACK and fresh CS5_Magazine_10RND magazines.
Already loaded rounds in existing magazines/chambers are not converted by config changes.
The UAV ammo addon defining UAV_Ammo_50CAL and UAV_Bullet_50CAL must be loaded.

CS5 chambers UAV_Ammo_50CAL; CS5_Magazine_10RND accepts UAV_Ammo_50CAL and fires UAV_Bullet_50CAL.
The config now inherits BoltActionRifle_Base, matching the script's CZ527 detachable-magazine mechanism.
Magazine capacity remains 10. Models, sounds, scripts and other weapon stats unchanged.
The GRZ loadout automatically detects this scope=2 bolt-action as .50 cal after restart.
The current loadout gives a compatible scope, selected wrap, ten ammo stacks and six full spare magazines.
No new loadout PBO is needed for this gun fix.

Config/package checks passed; not compiled or tested in DayZ.
'''
(root / 'CS5-50CAL.md').write_text(notes)
output = root / 'downloads/CS5-50CAL.zip'
report = dict(weapon='UAV_CS5_BLACK',category='.50 cal',ammo_pile='UAV_Ammo_50CAL',bullet='UAV_Bullet_50CAL',magazine='CS5_Magazine_10RND',magazine_capacity=10,changed_files=[config_name],unchanged_files=len(files)-1)
with ZipFile(output,'w',ZIP_DEFLATED) as archive:
    for name,data in files.items():archive.writestr(name,data)
    archive.writestr('INSTALL.txt',notes)
with ZipFile(output) as archive:
    assert archive.testzip() is None
    for name,data in original.items():
        if name != config_name:assert archive.read(name)==data,name
report['zip_sha256'] = hashlib.sha256(output.read_bytes()).hexdigest()
(root / 'downloads/CS5-50CAL-MANIFEST.json').write_text(json.dumps(report,indent=2)+'\n')
print('Built',output,'bytes',output.stat().st_size,'only changed source:',config_name)
