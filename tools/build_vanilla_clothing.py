"""Build the supplied vanilla clothing reskins without modifying base item stats."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import argparse,hashlib,io,json,struct
from PIL import Image,ImageOps
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--gloves',type=Path,required=True)
p.add_argument('--shirt',type=Path,required=True)
p.add_argument('--hat',type=Path,required=True)
a=p.parse_args();s=r/'GRZ_VanillaClothing';(s/'data').mkdir(parents=True,exist_ok=True)
items=[('gloves','GRZ_SurgicalGloves','GRZ Surgical Gloves','SurgicalGloves_White'),('shirt','GRZ_TShirt','GRZ T-Shirt','TShirt_Grey'),('hat','GRZ_CowboyHat','GRZ Cowboy Hat','CowboyHat_Brown')]
m={'descriptionShort':'made by blake','addon':'GRZ_VanillaClothing','items':[],
   'pending_uploads':['GRZ Sneakers','GRZ Bandana','GRZ Jeans'],
   'normal_conversion':'OpenGL tangent normals converted to DirectX with inverted green; normal X packed into alpha with native NOHQ SWIZ [5,4,2,3].',
   'vanilla_stats_preserved':True}
for key,classname,name,parent in items:
 entry={'class':classname,'displayName':name,'base_class':parent,'textures':{}}
 with ZipFile(getattr(a,key)) as z:
  for kind,ending in [('co','_Base_color.png'),('nohq','_Normal.png')]:
   names=[n for n in z.namelist() if n.endswith(ending)];assert len(names)==1
   raw=z.read(names[0]);im=Image.open(io.BytesIO(raw)).convert('RGB');assert im.size==(4096,4096)
   if kind=='co':data=encode_paa(im.convert('RGBA'),'DXT1')
   else:
    red,green,blue=im.split()
    packed=Image.merge('RGBA',(Image.new('L',im.size,0),ImageOps.invert(green),blue,ImageOps.invert(red)))
    data=bytearray(encode_paa(packed,'DXT5'))
    assert data[2:10]==b'GGATSFFO'
    offsets=struct.unpack_from('<16I',data,14)
    struct.pack_into('<16I',data,14,*[n+16 if n else 0 for n in offsets])
    data=bytes(data[:2])+b'GGATZIWS'+struct.pack('<I',4)+bytes([5,4,2,3])+bytes(data[2:])
   (s/'data'/f'{key}_{kind}.paa').write_bytes(data)
   entry['textures'][kind]={'source_sha256':hashlib.sha256(raw).hexdigest(),'paa_sha256':hashlib.sha256(data).hexdigest(),'size':list(im.size)}
 # Conservative fabric material with custom normal; no dependence on non-vanilla addons.
 material=(r/'packaging/customs/GRZ_XimsKit/data/pants.rvmat').read_text()
 material=material.replace('GRZ_XimsKit\\data\\pants_nohq.paa','GRZ_VanillaClothing\\data\\'+key+'_nohq.paa')
 (s/'data'/f'{key}.rvmat').write_text(material)
 m['items'].append(entry)
pbo=build(s);(r/'downloads/GRZ_VanillaClothing.pbo').write_bytes(pbo)
m['pbo_sha256']=hashlib.sha256(pbo).hexdigest()
(r/'downloads/GRZ-Vanilla-Clothing-MANIFEST.json').write_text(json.dumps(m,indent=2)+'\n')
notes='''GRZ Vanilla Clothing — available items
Every description is exactly: made by blake

GRZ Surgical Gloves — GRZ_SurgicalGloves (SurgicalGloves_White)
GRZ T-Shirt — GRZ_TShirt (TShirt_Grey)
GRZ Cowboy Hat — GRZ_CowboyHat (CowboyHat_Brown)

Includes the supplied 4096px colour textures and normal maps converted to PAA. The hat colour image supplied is entirely black; it is retained as provided. Uses a fabric material with your baked normals. Vanilla model, slots, storage, weight, insulation, durability, armour and damage/wear behavior are inherited; no custom DamageSystem or stat overrides. Native wear-state materials remain inherited and can replace the custom pristine material as the item degrades.

Requires only vanilla DayZ assets. Ordinary clothing, not VIP-only. Appears in the existing loadout GLOVE, SHIRT and HAT categories when AllInstalledClothing is enabled (default). VPP can spawn the classnames above. The on-ground, male and female texture/material entries use each item's supplied maps.

Close the game and stop the server. Copy GRZ_VanillaClothing.pbo into BOTH DayZ/@GRZ_Local/Addons and DayZServer/@GRZ_Local/Addons, then restart both. Do not replace or repack the original vanilla PBOs. If running a different custom mod folder, put it in that active folder on both sides instead, without duplicate copies. Source is included for repacking.

Pending: GRZ Sneakers, GRZ Bandana and GRZ Jeans. Their uploaded ZIPs exceed the 32 MiB file transfer limit and could not be read. They are not included, and there are no placeholder items. Resend each colour/normal PNG in a separate ZIP under 32 MiB to finish those pieces.

Metadata, resource bindings, colour/normal conversion and PBO/ZIP integrity are checked. DayZ compilation and worn-model appearance still require an in-game test.
'''
(r/'GRZ-VANILLA-CLOTHING.md').write_text(notes)
with ZipFile(r/'downloads/GRZ-Vanilla-Clothing.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_VanillaClothing.pbo',pbo);z.writestr('INSTALL.txt',notes)
 z.writestr('MANIFEST.json',json.dumps(m,indent=2)+'\n')
 for f in sorted(s.rglob('*')):
  if f.is_file():z.write(f,'source/GRZ_VanillaClothing/'+f.relative_to(s).as_posix())
print('Built three available vanilla clothing reskins:',len(pbo),'bytes. Sneakers/bandana/jeans still need smaller uploads.')
