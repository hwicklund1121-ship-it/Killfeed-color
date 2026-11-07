"""Build the supplied GRZ CSR50 paint and normal as a separate native-model reskin."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import argparse
import hashlib
import io
import json
import struct
from PIL import Image, ImageOps
from build_status_textures import encode_paa
from build_pbo import build

r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('paint_zip',type=Path)
p.add_argument('--base-zip',type=Path,default=r/'downloads/CSR50-Sounds.zip')
a=p.parse_args()
s=r/'GRZ_CSR50';(s/'data').mkdir(parents=True,exist_ok=True)
m={'class':'GRZ_CSR50','displayName':'GRZ CSR50','descriptionShort':'Made By Blake',
   'base_class':'UAV_CSR50','required_patch':'CSR50_Base','VIP_only':True,
   'normal_conversion':'Supplied OpenGL normal (matches kit Blender preview) converted to DirectX by inverting green. Native SWIZ [5,4,2,3] retained with inverted X packed into alpha.',
   'textures':{},'model_path_replacements':{}}
with ZipFile(a.paint_zip) as z:
 for label,ending in [('colour','_Base_color.png'),('normal','_Normal.png')]:
  names=[n for n in z.namelist() if n.endswith(ending)];assert len(names)==1
  raw=z.read(names[0]);im=Image.open(io.BytesIO(raw)).convert('RGB');assert im.size==(4096,4096)
  m['textures'][label]={'source_png_sha256':hashlib.sha256(raw).hexdigest(),'size':list(im.size)}
  if label=='colour':
   texture=encode_paa(im.convert('RGBA'),'DXT1');name='csr50_custom_part1_co.paa'
  else:
   # Native CSR50 normal-channel packing: restored R = 255-alpha, A = 255-red.
   red,green,blue=im.split();zero=Image.new('L',im.size,0)
   packed=Image.merge('RGBA',(zero,ImageOps.invert(green),blue,ImageOps.invert(red)))
   texture=bytearray(encode_paa(packed,'DXT5'))
   assert texture[2:10]==b'GGATSFFO'
   offsets=struct.unpack_from('<16I',texture,14)
   struct.pack_into('<16I',texture,14,*[v+16 if v else 0 for v in offsets])
   texture=bytes(texture[:2])+b'GGATZIWS'+struct.pack('<I',4)+bytes([5,4,2,3])+bytes(texture[2:])
   name='csr50_custom_part1_nohq.paa'
  (s/'data'/name).write_bytes(texture)
  m['textures'][label]['paa_sha256']=hashlib.sha256(texture).hexdigest()
with ZipFile(a.base_zip) as z:
 original=z.read('CSR50/CSR50/data/CSR50.p3d');assert original[:4]==b'ODOL'
 smdi=z.read('CSR50/CSR50/data/part1_smdi.paa')
 material=z.read('CSR50/CSR50/data/part1.rvmat').decode('utf-8-sig').replace('\r\n','\n')
model=original
oldprefix='uavweapons\\csr50\\csr50\\data\\'
newprefix='GRZ_CSR50\\data\\csr50_custom_'
assert len(oldprefix)==len(newprefix)
for suffix in ['part1_co.paa','part1_nohq.paa','part1_smdi.paa','part1.rvmat']:
 old=(oldprefix+suffix).encode();new=(newprefix+suffix).encode()
 count=model.count(old);assert count>0,(suffix,count)
 model=model.replace(old,new);m['model_path_replacements'][suffix]=count
restored=model
for suffix in m['model_path_replacements']:
 restored=restored.replace((newprefix+suffix).encode(),(oldprefix+suffix).encode())
assert restored==original and len(model)==len(original)
(s/'data/csr50_grz.p3d').write_bytes(model)
(s/'data/csr50_custom_part1_smdi.paa').write_bytes(smdi)
material=material.replace('UAVWEAPONS\\CSR50\\CSR50\\data\\',newprefix).replace(oldprefix,newprefix)
(s/'data/csr50_custom_part1.rvmat').write_text(material)
m['model_source_sha256']=hashlib.sha256(original).hexdigest()
m['model_reskin_sha256']=hashlib.sha256(model).hexdigest()
m['geometry_uv_skeleton_proxies_preserved']=True
m['normal_specular_materials']='Supplied baked normal bound in native model and own RVMAT; native CSR50 SMDI retained unchanged.'
pbo=build(s);(r/'downloads/GRZ_CSR50.pbo').write_bytes(pbo)
m['pbo_sha256']=hashlib.sha256(pbo).hexdigest()
notes='''GRZ CSR50
Description: Made By Blake
Spawn classname: GRZ_CSR50

Separate reskin of UAV_CSR50. Requires the installed original CSR50 weapon addon (CSR50_Base). Uses your supplied 4096px painted base colour and baked normal, plus the original specular material. The normal green channel is converted from the Blender/OpenGL orientation back to DayZ/DirectX; original CSR50 normal packing is retained.

Compiled native model copied privately with equal-length texture/material path replacements only. Geometry, UVs, animation data, skeleton, proxies and all 9 LODs remain unchanged. No FBX/MLOD rebuild. Native ammo, damage, firing sounds, recoil, attachments and bolt action are inherited. All five condition material stages keep the reskin.

Close DayZ and the server. Copy GRZ_CSR50.pbo to BOTH DayZ/@GRZ_Local/Addons and DayZServer/@GRZ_Local/Addons alongside your existing PBOs. Restart both. Alternatively use the complete GRZ-Arena-Menu-Update.zip installer. Do not install duplicate copies in multiple active mod folders. No UAV weapon pack repack required.

With the existing GRZ VIP system, it appears in /grzvip > GUN RESKINS > 20MM, based on the installed CSR50 ammo configuration. Existing VIP allowlist applies. VPP spawn classname: GRZ_CSR50. Only the reskin is new; no base weapon files are replaced.

The ZIP includes unpacked addon source for repacking and the ready-to-use PBO. Metadata, original compiled model reversibility, PAA resources, normal orientation/packing and package integrity checked. DayZ rendering and multiplayer runtime still need an in-game test.
'''
(r/'GRZ-CSR50.md').write_text(notes)
(r/'downloads/GRZ-CSR50-MANIFEST.json').write_text(json.dumps(m,indent=2)+'\n')
with ZipFile(r/'downloads/GRZ-CSR50.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_CSR50.pbo',pbo);z.writestr('INSTALL.txt',notes)
 z.writestr('MANIFEST.json',json.dumps(m,indent=2)+'\n')
 for f in sorted(s.rglob('*')):
  if f.is_file():z.write(f,'source/GRZ_CSR50/'+f.relative_to(s).as_posix())
print('Built GRZ CSR50 reskin:',len(pbo),'bytes; native model geometry/UVs preserved.')
