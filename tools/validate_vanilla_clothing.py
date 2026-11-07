"""Verify ready clothing assets, inherited stats and supplied paint/normal conversion."""
from pathlib import Path
from zipfile import ZipFile
import argparse,hashlib,io,json,re,struct
import numpy as np
from PIL import Image
from repair_hexgrid import read_pbo
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
for key in ['gloves','shirt','hat']:p.add_argument('--'+key,type=Path,required=True)
a=p.parse_args();source=r/'GRZ_VanillaClothing'
m=json.loads((r/'downloads/GRZ-Vanilla-Clothing-MANIFEST.json').read_text())
payload=(r/'downloads/GRZ_VanillaClothing.pbo').read_bytes();ext,files=read_pbo(payload)
assert ext['prefix']=='GRZ_VanillaClothing' and hashlib.sha1(payload[:-21]).digest()==payload[-20:]
assert hashlib.sha256(payload).hexdigest()==m['pbo_sha256']
expected={p.relative_to(source).as_posix():p.read_bytes() for p in source.rglob('*') if p.is_file() and p.name!='$PBOPREFIX$'}
assert files==expected
cfg=files['config.cpp'].decode()
assert cfg.count('descriptionShort = "made by blake";')==3
for item in m['items']:
 assert 'class '+item['class']+': '+item['base_class'] in cfg
 assert 'displayName = "'+item['displayName']+'";' in cfg
for field in ['DamageSystem','hitpoints','itemsCargoSize','weight','heatIsolation','inventorySlot','GRZVipOnly','class CfgMods']:
 assert field not in cfg,field
dependencies=re.search(r'requiredAddons\[\] = \{([^}]+)\}',cfg).group(1)
assert all(x.startswith('DZ_') for x in re.findall(r'"([^\"]+)"',dependencies))
assert not any('Sneakers' in name or 'Jeans' in name or 'Bandana' in name for name in re.findall(r'class (GRZ_\w+)',cfg))
for resource in re.findall(r'"(GRZ_VanillaClothing\\[^\"]+)"',cfg+'\n'+'\n'.join(v.decode() for k,v in files.items() if k.endswith('.rvmat'))):
 assert resource.removeprefix('GRZ_VanillaClothing\\').replace('\\','/') in files,resource

def decode_top(data):
 fmt=struct.unpack_from('<H',data)[0];pos=2;tags={}
 while data[pos:pos+4]==b'GGAT':
  name=data[pos+4:pos+8][::-1].decode();n=struct.unpack_from('<I',data,pos+8)[0]
  tags[name]=data[pos+12:pos+12+n];pos+=12+n
 offsets=struct.unpack('<16I',tags['OFFS']);pos+=2;assert offsets[0]==pos
 width,height=struct.unpack_from('<HH',data,pos);length=int.from_bytes(data[pos+4:pos+7],'little')
 raw=data[pos+7:pos+7+length];compressed=bool(width&0x8000);width&=0x7fff
 if compressed:
  import lzokay
  raw=lzokay.decompress(raw,width*height//(2 if fmt==0xff01 else 1))
 hdr=io.BytesIO();Image.new('RGBA',(width,height)).save(hdr,format='DDS',pixel_format='DXT1' if fmt==0xff01 else 'DXT5')
 pix=np.array(Image.open(io.BytesIO(hdr.getvalue()[:128]+raw)).convert('RGBA'))
 if 'SWIZ' in tags:
  assert tags['SWIZ']==bytes([5,4,2,3])
  alpha=pix[:,:,3].copy();red=pix[:,:,0].copy()
  pix[:,:,0]=255-alpha;pix[:,:,3]=255-red
 return pix
for key,item in zip(['gloves','shirt','hat'],m['items']):
 with ZipFile(getattr(a,key)) as z:
  for kind,ending in [('co','_Base_color.png'),('nohq','_Normal.png')]:
   name=next(n for n in z.namelist() if n.endswith(ending));raw=z.read(name)
   assert hashlib.sha256(raw).hexdigest()==item['textures'][kind]['source_sha256']
   original=np.array(Image.open(io.BytesIO(raw)).convert('RGB'))
   if kind=='nohq':original[:,:,1]=255-original[:,:,1]
   pixels=decode_top(files['data/'+key+'_'+kind+'.paa'])
   assert pixels.shape[:2]==original.shape[:2]==(4096,4096)
   error=np.abs(pixels[:,:,:3].astype(float)-original).mean();assert error<5,(key,kind,error)
   print(key,kind,'mean PAA error:',round(error,3))
with ZipFile(r/'downloads/GRZ-Vanilla-Clothing.zip') as z:
 assert z.testzip() is None and z.read('GRZ_VanillaClothing.pbo')==payload
 for name,data in expected.items():assert z.read('source/GRZ_VanillaClothing/'+name)==data
print('Three available items: exact names/descriptions, inherited vanilla stats, resource bindings, all six 4K PAA conversions and source/PBO/ZIP matching passed. Three oversized uploads remain pending; DayZ runtime untested.')
