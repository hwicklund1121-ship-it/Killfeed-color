"""Package verified vanilla XIMS items; prepare layered-shirt art without a guessed dependency."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import argparse,hashlib,io,json,struct
from PIL import Image,ImageOps
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
for key in ['sneakers','shorts','shirt','shirt-model']:p.add_argument('--'+key,type=Path,required=True)
a=p.parse_args();s=r/'GRZ_XimsClothing';pending=r/'packaging/pending-xims-layered-shirt'
report={'items':[],'pending':['XIMS Layered Shirt: Windstride CfgPatches/config and texture selection order required.','XIMS Canvas Bag: uploaded archive exceeds 32 MiB transfer limit.']}

def convert(zip_path,key,target):
 textures={}
 with ZipFile(zip_path) as z:
  for kind,ending in [('co','_Base_color.png'),('nohq','_Normal.png')]:
   names=[n for n in z.namelist() if n.endswith(ending)];assert len(names)==1
   raw=z.read(names[0]);im=Image.open(io.BytesIO(raw)).convert('RGB');assert im.size==(4096,4096)
   meta={'source_sha256':hashlib.sha256(raw).hexdigest(),'size':list(im.size),'source_filename':Path(names[0]).name}
   if kind=='nohq' and im.getextrema()==((128,128),(128,128),(255,255)):
    meta['mode']='flat export; vanilla normal/material inherited';textures[kind]=meta;continue
   if kind=='co':data=encode_paa(im.convert('RGBA'),'DXT1')
   else:
    red,green,blue=im.split()
    rgba=Image.merge('RGBA',(Image.new('L',im.size,0),ImageOps.invert(green),blue,ImageOps.invert(red)))
    data=bytearray(encode_paa(rgba,'DXT5'))
    offsets=struct.unpack_from('<16I',data,14)
    struct.pack_into('<16I',data,14,*[v+16 if v else 0 for v in offsets])
    data=bytes(data[:2])+b'GGATZIWS'+struct.pack('<I',4)+bytes([5,4,2,3])+bytes(data[2:])
   (target/'data'/f'{key}_{kind}.paa').write_bytes(data)
   meta['paa_sha256']=hashlib.sha256(data).hexdigest();meta['mode']='supplied painted colour' if kind=='co' else 'OpenGL to DirectX green conversion; native SWIZ normal packing'
   textures[kind]=meta
 return textures
for key,cls,name,parent in [('sneakers','XIMS_Sneakers','XIMS Sneakers','Sneakers_Black'),('shorts','XIMS_JeanShorts','XIMS Jean Shorts','ShortJeans_Blue')]:
 report['items'].append({'class':cls,'displayName':name,'descriptionShort':'Made By Blake','base_class':parent,'key':key,'textures':convert(getattr(a,key),key,s)})
material=(r/'packaging/customs/GRZ_XimsKit/data/pants.rvmat').read_text().replace('GRZ_XimsKit\\data\\pants_nohq.paa','GRZ_XimsClothing\\data\\shorts_nohq.paa')
(s/'data/shorts.rvmat').write_text(material)
shirt=convert(a.shirt,'shirt',pending)
(pending/'TEXTURE-MANIFEST.json').write_text(json.dumps({'displayName':'XIMS Layered Shirt','descriptionShort':'Made By Blake','base_class':'Layered_Shirt_Base','textures':shirt,'provided_female_mlod_sha256':hashlib.sha256(a.shirt_model.read_bytes()).hexdigest()},indent=2)+'\n')
(pending/'README.txt').write_text('XIMS Layered Shirt — Made By Blake\nTextures are converted and ready. The supplied female MLOD references windstrideclothing/models/layered_shirt. Public Expansion market settings confirm Layered_Shirt_Base. To safely build this reskin, the original Windstride config.cpp is still required to verify its CfgPatches dependency and hidden-selection/material order for ground, male and female models. Do not install this staging folder as an addon. No active config/PBO is shipped until those bindings are verified. The canvas bag textures are also pending a smaller upload.\n')
pbo=build(s);(r/'downloads/GRZ_XimsClothing.pbo').write_bytes(pbo);report['pbo_sha256']=hashlib.sha256(pbo).hexdigest()
(r/'downloads/XIMS-Clothing-MANIFEST.json').write_text(json.dumps(report,indent=2)+'\n')
notes='''XIMS Clothing — available pieces
XIMS Sneakers — XIMS_Sneakers
XIMS Jean Shorts — XIMS_JeanShorts
Descriptions: Made By Blake

Vanilla Sneakers_Black and ShortJeans_Blue bases, with original models, slots, storage, insulation, durability and protection. No stat/DamageSystem overrides. Sneakers use your painted colour plus native material/normal because the supplied exported normal is entirely flat. Shorts use your supplied all-black colour and custom baked normal, converted from OpenGL to DirectX. Fabric material used for pristine shorts; native wear-state materials are inherited.

Ready PBO: GRZ_XimsClothing.pbo. Copy to BOTH client and server @GRZ_Local/Addons and restart both, alongside GRZ_VanillaClothing.pbo. Ordinary clothing, not VIP-only. The existing loadout discovers SHOES/PANTS after restart. Never replace vanilla PBOs. Source config/material files included; unpack the PBO for full texture assets.

Layered shirt remains pending: its source model is Windstride, not vanilla. Prepared colour/normal assets are staged in packaging/pending-xims-layered-shirt in the repository, but an original Windstride config.cpp is required before an installable shirt addon can be built safely. Canvas bag remains pending because its ZIP exceeded the 32 MiB transfer limit; resend separate texture ZIPs under that limit.

Use the combined GRZ-All-Reskins-Update installer to apply this alongside the prior GRZ clothing, CSR50, CS5, animated reskins and gameplay update. Existing mod dependencies remain required. Static/resource/package checks only; worn appearance requires a DayZ test.
'''
(r/'XIMS-CLOTHING.md').write_text(notes)
with ZipFile(r/'downloads/XIMS-Clothing.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_XimsClothing.pbo',pbo);z.writestr('MANIFEST.json',json.dumps(report,indent=2)+'\n');z.writestr('INSTALL.txt',notes)
 for f in sorted(s.rglob('*')):
  if f.is_file() and f.suffix!='.paa':z.write(f,'source/GRZ_XimsClothing/'+f.relative_to(s).as_posix())
print('Built XIMS Sneakers and Jean Shorts; layered shirt textures prepared, original config still required.')
