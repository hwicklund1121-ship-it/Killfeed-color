"""Check the pinned multi-pack updater and optionally an actual installer fixture."""
from pathlib import Path
from zipfile import ZipFile
import argparse,hashlib,json,re,subprocess
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--fixture-root',type=Path);a=p.parse_args()
with ZipFile(r/'downloads/GRZ-All-Reskins-Update.zip') as z:
 assert z.testzip() is None
 assert set(z.namelist())=={'Install-All-Reskins.ps1','RESKIN-PACKS.json','INSTALL.txt','inventory-builder/InventoryBuilder.cs','inventory-builder/Build-Inventory-Mod.ps1','inventory-builder/INSTALL.txt'}
 assert z.read('Install-All-Reskins.ps1')==(r/'packaging/all-reskins-update/Install-All-Reskins.ps1').read_bytes()
 m=json.loads(z.read('RESKIN-PACKS.json'));assert m['schema']==1
 assert re.fullmatch('[a-f0-9]{40}',m['asset_revision'])
 all_addons={}
 for pack in m['packages']:
  path=r/'downloads'/pack['file'];data=path.read_bytes()
  assert hashlib.sha256(data).hexdigest()==pack['sha256']
  assert '/'+m['asset_revision']+'/downloads/' in pack['url']
  with ZipFile(path) as assets:
   for name,sha in pack['addons'].items():
    assert name not in all_addons
    assert hashlib.sha256(assets.read(name+'.pbo')).hexdigest()==sha
    all_addons[name]=sha
 assert len(m['packages'])==3 and len(all_addons)==18
 assert 'GRZ_XimsClothing' in all_addons and 'GRZ_VanillaClothing' in all_addons and 'GRZ_CSR50' in all_addons
 assert 'GRZ_XimsLayeredShirt' in all_addons and m['pending']==[]
 assert 'GRZ_CSR50' in all_addons and 'GRZ_VanillaClothing' in all_addons
script=(r/'packaging/all-reskins-update/Install-All-Reskins.ps1').read_text()
assert script.index('Archive hash mismatch')<script.index('Expand-Archive')
assert script.index('PBO hash mismatch')<script.index("& (Join-Path $core 'Install-Update.ps1')")
assert 'Find-WindstridePbo' in script and 'SkipLayeredShirt' in script
assert script.index('Original Windstride Clothing PBO not found')<script.index('Invoke-WebRequest')
assert 'finally' in script and 'Remove-Item -LiteralPath $staging' in script
if a.fixture_root:
 for side in ['server','game']:
  for name,sha in all_addons.items():
   assert hashlib.sha256((a.fixture_root/side/'@GRZ_Local/Addons'/(name+'.pbo')).read_bytes()).hexdigest()==sha
 for side in ['server','game']:
  for name in ['GRZ_InventorySizes','GRZ_MovementSpeedData']:
   assert (a.fixture_root/side/'@GRZ_Local/Addons'/(name+'.pbo')).is_file()
 for rel,txt in [('GRZ_Duels/ratings.json','KEEP ELO'),('GRZ_Loadout/players/test.json','KEEP OUTFIT'),('GRZ_Killfeed/styles.json','KEEP PREFIXES'),('GRZ_Killfeed/config.json','KEEP VIP')]:assert (a.fixture_root/'server/profiles'/rel).read_text()==txt
 print('Installer fixture: 36 source PBO copies match and four generated PBO copies exist; outfits, ratings, VIP config and prefixes retained.')
print('Combined archive, three source ZIP hashes, all 18 PBO hashes, URL pinning and pending-item exclusion passed. Live Windows networking/DayZ runtime untested.')
