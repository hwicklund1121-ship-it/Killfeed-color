"""Build a self-contained SFG suppressor compatibility patch using the supplied SFG assets."""
import re
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build
from repair_hexgrid import section, read_pbo, resource_refs
root = Path(__file__).resolve().parents[1]
source = root / 'GRZ_SFGPermanent'
prefix = r'GRZSFGAUTO\SFG50\SFG'
(source / '$PBOPREFIX$').write_text(prefix+'\n')
with ZipFile(root / 'downloads/SFG50-SFG-Sounds.zip') as z:
    files = {n[len('SFG50/SFG/'):]:z.read(n) for n in z.namelist() if n.startswith('SFG50/SFG/') and not n.endswith('/')}
original_config = files['config.cpp'].decode('utf-8-sig')
sections = []
for name in ['CfgVehicles','CfgNonAIVehicles','CfgSlots']:
    a,_,_,b = section(original_config,name)
    sections.append(original_config[a:b])
config = '''class CfgPatches
{
 class GRZ_SFGPermanent
 {
  units[] = {};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"sfg50", "UAV_SFG_Suppressor"};
 };
};
class CfgMods
{
 class GRZ_SFGPermanent
 {
  name = "GRZ SFG Permanent Suppressor";
  author = "Blake";
  type = "mod";
  dependencies[] = {"World"};
  class defs
  {
   class worldScriptModule
   {
    value = "";
    files[] = {"GRZSFGAUTO/SFG50/SFG/scripts/4_World"};
   };
  };
 };
};
class Mode_SemiAuto;
class CfgWeapons
{
 class BoltActionRifle_InnerMagazine_Base;
 class sfg50_Base: BoltActionRifle_InnerMagazine_Base
 {
  model = "GRZSFGAUTO\\SFG50\\SFG\\data\\SFG50.p3d";
  attachments[] = {"weaponWrap", "weaponOpticsHunting", "UAV_SFG_Suppressor"};
  GRZIntegratedSuppressor = 1;
  class Single: Mode_SemiAuto
  {
   soundSetShot[] = {"sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet"};
   soundSetShotExt[] = {{"sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet"}};
   reloadTime = 1.0;
   recoil = "recoil_AKM";
   recoilProne = "recoil_AKM_prone";
  };
 };
};
'''+'\n'.join(sections)+'\n'
config = re.sub('UAVWEAPONS','GRZSFGAUTO',config,flags=re.I)
# Render the muzzle model as an unconditional gun proxy. The hidden server
# attachment still supplies suppressor behavior, but no longer controls visibility.
config, static_proxy_edits = re.subn(
    r'(class\s+Proxysfg_muzzle\s*:\s*ProxyAttachment\s*\{\s*scope\s*=\s*2;\s*)inventorySlot\s*=\s*"UAV_SFG_Suppressor";',
    lambda match: match[1] + 'simulation = "alwaysshow";\n\t\tinventorySlot = "";', config)
assert static_proxy_edits == 1

config = config.replace('\r\n', '\n')
(source / 'config.cpp').write_text(config)
for name,data in files.items():
    if not name.endswith(('.p3d','.paa','.rvmat')):
        continue
    if name.endswith(('.p3d','.rvmat')):
        converted = re.sub(b'uavweapons',b'GRZSFGAUTO',data,flags=re.I)
        assert len(converted)==len(data)
        data=converted
    path=source/name
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(data)
pbo=build(source)
output=root/'downloads/GRZ_SFGPermanent.pbo'
output.write_bytes(pbo)
notes='''GRZ SFG Permanent Suppressor

Copy GRZ_SFGPermanent.pbo into @GRZ_Local/Addons on BOTH game and server.
Keep the SFG weapon pack and latest GRZ_Loadout.pbo installed. Restart both.
Spawn a fresh SFG from VPP or save an SFG loadout.
Requires the original sfg50 weapon addon; does not add a second SFG weapon classname.

Supplies the suppressor config, matching custom slot, visual proxy and required model assets.
The SFG uses an integrated visual suppressor and its existing suppressed firing sound sets directly.
A separate suppressor item is no longer required for loadout creation.
Server equips HuntingOptic after initialization, loading and debug spawning, with retries.
An existing hunting-slot optic is preserved; the scope remains removable.
Slot locking and CanReleaseAttachment prevent normal removal. Inventory slot stays hidden.
Suppressor remains scope 1 so it cannot be selected as an ordinary separate spawn-menu item.
The gun muzzle proxy uses simulation alwaysshow, making the suppressor model permanent on the gun, including local previews.
The permanent visual proxy has no inventory slot; firing mode uses the suppressed sound sets directly.
The loadout recognizes the integrated-suppressor config marker and does not require a separate attachment.
The private asset namespace preserves original geometry bytes.
The latest loadout creates the same suppressor in its local preview.
VPP's own local model preview may omit runtime attachments; check the actual spawned gun.
Ammunition, damage, recoil and reload timing are preserved; firing sounds use the existing suppressed SFG sound sets.
An admin or another server script can still forcibly delete entities; normal player removal is blocked.

Asset references, PBO, ZIP and source checked. Not compiled or tested in DayZ.
Unsigned build; sign with your own server key if signature verification is enabled.
'''
(root/'SFG-PERMANENT-PATCH.md').write_text(notes)
with ZipFile(root/'downloads/GRZ_SFGPermanent.zip','w',ZIP_DEFLATED) as z:
    z.writestr('GRZ_SFGPermanent.pbo',pbo)
    z.writestr('INSTALL.txt',notes)
ext,packed=read_pbo(pbo)
assert ext['prefix']==prefix
mounted={(prefix+'/'+n).replace('\\','/').lower() for n in packed}
for name,data in packed.items():
    if name.endswith('.p3d'):refs=resource_refs(data)
    elif name.endswith(('.cpp','.rvmat')):refs=re.findall(r'"([^"\r\n]+\.(?:p3d|paa|rvmat))"',data.decode(),re.I)
    else:continue
    for ref in refs:
        normalized=ref.replace('\\','/').lstrip('/').lower()
        if normalized.startswith('grzsfgaauto/'):raise AssertionError('Typo')
        if normalized.startswith('grzsfgauto/'):
            assert normalized in mounted,(name,ref)
assert pbo[-20:]==hashlib.sha1(pbo[:-21]).digest()
(root/'downloads/GRZ-SFG-PERMANENT-MANIFEST.json').write_text(json.dumps(dict(prefix=prefix,requires='sfg50',suppressor='UAV_SFG_Sup',slot='UAV_SFG_Suppressor',scope=1,sha256=hashlib.sha256(pbo).hexdigest(),files=len(packed)),indent=2)+'\n')
print('Built',output,len(pbo),'bytes; asset references, equal-length model edits and PBO verified')
