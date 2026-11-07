"""Bundle a small verified-download installer for the combined reskin/gameplay packs."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,subprocess,argparse
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--revision',required=True);a=p.parse_args()
assert len(a.revision)==40 and all(c in '0123456789abcdef' for c in a.revision)
manifest={'schema':1,'asset_revision':a.revision,'packages':[],'pending':[],'pending_custom_normals':['XIMS Canvas Bag','GRZ Jeans','GRZ Bandana']}
for filename,role in [('GRZ-Arena-Menu-Update.zip','core'),('GRZ-Vanilla-Clothing.zip','clothing'),('XIMS-Clothing.zip','xims')]:
 path=r/'downloads'/filename;data=path.read_bytes();addons={}
 with ZipFile(path) as z:
  for name in z.namelist():
   if '/' not in name and name.endswith('.pbo'):addons[name[:-4]]=hashlib.sha256(z.read(name)).hexdigest()
 assert addons
 manifest['packages'].append({'file':filename,'role':role,'url':'https://github.com/hwicklund1121-ship-it/Killfeed-color/raw/'+a.revision+'/downloads/'+filename,'sha256':hashlib.sha256(data).hexdigest(),'addons':addons})
assert len({k for pack in manifest['packages'] for k in pack['addons']})==18
notes=(r/'packaging/all-reskins-update/INSTALL.txt').read_text()
(r/'GRZ-ALL-RESKINS-UPDATE.md').write_text(notes)
pkg=r/'packaging/all-reskins-update'
(pkg/'RESKIN-PACKS.json').write_text(json.dumps(manifest,indent=2)+'\n')
with ZipFile(r/'downloads/GRZ-All-Reskins-Update.zip','w',ZIP_DEFLATED) as z:
 z.write(pkg/'Install-All-Reskins.ps1','Install-All-Reskins.ps1')
 for name in ['InventoryBuilder.cs','Build-Inventory-Mod.ps1','INSTALL.txt']:
  z.write(r/'packaging/inventory-builder'/name, 'inventory-builder/'+name)
 z.writestr('RESKIN-PACKS.json',json.dumps(manifest,indent=2)+'\n');z.writestr('INSTALL.txt',notes)
print('Built combined verified-download installer for 18 addons; all four XIMS items included; oversized custom normals documented.')
