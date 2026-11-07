"""Verify CSR50 metadata, direct ODOL bindings, native recovery and normal packing."""
from pathlib import Path
from zipfile import ZipFile
import argparse
import hashlib
import io
import json
import re
import struct
import numpy as np
from PIL import Image
from repair_hexgrid import read_pbo

r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--paint-zip',type=Path,required=True);a=p.parse_args()
pbo=(r/'downloads/GRZ_CSR50.pbo').read_bytes();ext,files=read_pbo(pbo)
assert ext['prefix']=='GRZ_CSR50' and hashlib.sha1(pbo[:-21]).digest()==pbo[-20:]
source=r/'GRZ_CSR50'
expected_files={p.relative_to(source).as_posix():p.read_bytes() for p in source.rglob('*') if p.is_file() and p.name!='$PBOPREFIX$'}
assert files==expected_files
cfg=files['config.cpp'].decode();assert 'displayName = "GRZ CSR50";' in cfg and 'descriptionShort = "Made By Blake";' in cfg
assert 'class GRZ_CSR50: UAV_CSR50' in cfg and 'requiredAddons[] = {"CSR50_Base"}' in cfg
assert 'GRZVipOnly = 1;' in cfg and 'hiddenSelections[] = {};' in cfg
assert 'hitpoints = 10000' in cfg and cfg.count('"GRZ_CSR50\\data\\csr50_custom_part1.rvmat"')==5
# No ammo, fire mode, sound or recoil overrides.
for field in ['chamberableFrom','soundSetShot','recoilModifier','reloadTime']:
 assert field not in cfg
model=files['data/csr50_grz.p3d'];assert model[:4]==b'ODOL'
m=json.loads((r/'downloads/GRZ-CSR50-MANIFEST.json').read_text());restored=model
oldprefix='uavweapons\\csr50\\csr50\\data\\';newprefix='GRZ_CSR50\\data\\csr50_custom_'
for suffix,count in m['model_path_replacements'].items():
 old=(oldprefix+suffix).encode();new=(newprefix+suffix).encode()
 assert len(old)==len(new) and model.count(new)==count and old not in model
 assert 'data/csr50_custom_'+suffix in files
 restored=restored.replace(new,old)
with ZipFile(r/'downloads/CSR50-Sounds.zip') as z:
 assert restored==z.read('CSR50/CSR50/data/CSR50.p3d')
 assert files['data/csr50_custom_part1_smdi.paa']==z.read('CSR50/CSR50/data/part1_smdi.paa')
assert hashlib.sha256(restored).hexdigest()==m['model_source_sha256']
assert hashlib.sha256(model).hexdigest()==m['model_reskin_sha256']
assert hashlib.sha256(pbo).hexdigest()==m['pbo_sha256']
material=files['data/csr50_custom_part1.rvmat'].decode()
for resource in re.findall(r'"(GRZ_CSR50\\[^\"]+)"',material+cfg):
 assert resource.removeprefix('GRZ_CSR50\\').replace('\\','/') in files,resource

def decode(data):
 kind=struct.unpack_from('<H',data)[0];pos=2;tags={}
 while data[pos:pos+4]==b'GGAT':
  key=data[pos+4:pos+8][::-1].decode();size=struct.unpack_from('<I',data,pos+8)[0]
  tags[key]=data[pos+12:pos+12+size];pos+=12+size
 offsets=struct.unpack('<16I',tags['OFFS']);pos+=2
 width,height=struct.unpack_from('<HH',data,pos);size=int.from_bytes(data[pos+4:pos+7],'little');raw=data[pos+7:pos+7+size]
 assert offsets[0]==pos
 compressed=bool(width&0x8000);width&=0x7fff
 if compressed:
  import lzokay
  raw=lzokay.decompress(raw,width*height//(2 if kind==0xff01 else 1))
 hdr=io.BytesIO();Image.new('RGBA',(width,height)).save(hdr,format='DDS',pixel_format='DXT1' if kind==0xff01 else 'DXT5')
 pixels=np.array(Image.open(io.BytesIO(hdr.getvalue()[:128]+raw)).convert('RGBA'))
 if 'SWIZ' in tags:
  channels=[pixels[:,:,3],pixels[:,:,0],pixels[:,:,1],pixels[:,:,2]];original=[c.copy() for c in channels]
  for idx,(op,channel) in enumerate(zip(tags['SWIZ'],original)):
   if op==idx:continue
   target=channels[op&3]
   if op&8:target[:]=255
   elif op&4:target[:]=255-channel
 return pixels,tags
with ZipFile(a.paint_zip) as z:
 for label,filename in [('colour','CSR50_Reskin_Material_Base_color.png'),('normal','CSR50_Reskin_Material_Normal.png')]:
  raw=z.read('GRZ_CSR50/'+filename);expected=np.array(Image.open(io.BytesIO(raw)).convert('RGB'))
  if label=='normal':expected[:,:,1]=255-expected[:,:,1]
  path='data/csr50_custom_part1_'+('co' if label=='colour' else 'nohq')+'.paa'
  pixels,tags=decode(files[path]);assert pixels.shape[:2]==(4096,4096)
  error=np.abs(pixels[:,:,:3].astype(float)-expected).mean()
  assert error<5.0,(label,error)
  assert hashlib.sha256(raw).hexdigest()==m['textures'][label]['source_png_sha256']
  if label=='normal':assert tags['SWIZ']==bytes([5,4,2,3])
  print(label,'PAA round-trip mean error:',round(error,3))
with ZipFile(r/'downloads/GRZ-CSR50.zip') as z:
 assert z.testzip() is None and z.read('GRZ_CSR50.pbo')==pbo
 for name,value in expected_files.items():
  assert z.read('source/GRZ_CSR50/'+name)==value
with ZipFile(r/'downloads/GRZ-Arena-Menu-Update.zip') as z:
 assert z.read('GRZ_CSR50.pbo')==pbo
print('Exact metadata, native geometry/UV reversibility, colour/normal round trips, native SMDI, every condition material and both deliveries verified. DayZ runtime untested.')
