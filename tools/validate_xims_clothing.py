"""Check native XIMS item inheritance, texture conversion and pending-shirt isolation."""
from pathlib import Path
from zipfile import ZipFile
import argparse,hashlib,io,json,re,struct
import numpy as np
from PIL import Image
from repair_hexgrid import read_pbo
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
for key in ['sneakers','shorts','shirt']:p.add_argument('--'+key,type=Path,required=True)
a=p.parse_args();s=r/'GRZ_XimsClothing'
pbo=(r/'downloads/GRZ_XimsClothing.pbo').read_bytes();ext,files=read_pbo(pbo)
assert ext['prefix']=='GRZ_XimsClothing' and hashlib.sha1(pbo[:-21]).digest()==pbo[-20:]
assert files=={p.relative_to(s).as_posix():p.read_bytes() for p in s.rglob('*') if p.is_file() and p.name!='$PBOPREFIX$'}
m=json.loads((r/'downloads/XIMS-Clothing-MANIFEST.json').read_text());assert hashlib.sha256(pbo).hexdigest()==m['pbo_sha256']
cfg=files['config.cpp'].decode();assert cfg.count('descriptionShort = "Made By Blake";')==2
assert 'class XIMS_Sneakers: Sneakers_Black' in cfg and 'class XIMS_JeanShorts: ShortJeans_Blue' in cfg
for name in ['XIMS Sneakers','XIMS Jean Shorts']:assert 'displayName = "'+name+'";' in cfg
for key in ['DamageSystem','hitpoints','heatIsolation','itemsCargoSize','inventorySlot','GRZVipOnly','Layered_Shirt','Windstride']:assert key not in cfg
assert 'data/sneakers_nohq.paa' not in files and 'data/sneakers.rvmat' not in files
for path in re.findall(r'"(GRZ_XimsClothing\\[^\"]+)"',cfg+'\n'+files['data/shorts.rvmat'].decode()):assert path.removeprefix('GRZ_XimsClothing\\').replace('\\','/') in files

def decode(data):
 typ=struct.unpack_from('<H',data)[0];pos=2;tags={}
 while data[pos:pos+4]==b'GGAT':
  size=struct.unpack_from('<I',data,pos+8)[0];tags[data[pos+4:pos+8][::-1].decode()]=data[pos+12:pos+12+size];pos+=12+size
 pos+=2;assert struct.unpack('<16I',tags['OFFS'])[0]==pos
 w,h=struct.unpack_from('<HH',data,pos);n=int.from_bytes(data[pos+4:pos+7],'little');raw=data[pos+7:pos+7+n]
 if w&0x8000:
  import lzokay
  w&=0x7fff;raw=lzokay.decompress(raw,w*h//(2 if typ==0xff01 else 1))
 head=io.BytesIO();Image.new('RGBA',(w,h)).save(head,format='DDS',pixel_format='DXT1' if typ==0xff01 else 'DXT5')
 image=np.array(Image.open(io.BytesIO(head.getvalue()[:128]+raw)).convert('RGBA'))
 if 'SWIZ' in tags:
  assert tags['SWIZ']==bytes([5,4,2,3]);alpha=image[:,:,3].copy();red=image[:,:,0].copy();image[:,:,0]=255-alpha;image[:,:,3]=255-red
 return image[:,:,:3]
for key in ['sneakers','shorts','shirt']:
 target=s if key!='shirt' else r/'packaging/pending-xims-layered-shirt'
 meta=next(i['textures'] for i in m['items'] if i['key']==key) if key!='shirt' else json.loads((target/'TEXTURE-MANIFEST.json').read_text())['textures']
 with ZipFile(getattr(a,key)) as z:
  for kind,ending in [('co','_Base_color.png'),('nohq','_Normal.png')]:
   raw=z.read(next(n for n in z.namelist() if n.endswith(ending)))
   assert hashlib.sha256(raw).hexdigest()==meta[kind]['source_sha256']
   image=np.array(Image.open(io.BytesIO(raw)).convert('RGB'))
   if key=='sneakers' and kind=='nohq':assert (image==[128,128,255]).all();continue
   if kind=='nohq':image[:,:,1]=255-image[:,:,1]
   data=(target/'data'/f'{key}_{kind}.paa').read_bytes()
   assert hashlib.sha256(data).hexdigest()==meta[kind]['paa_sha256']
   decoded=decode(data);assert decoded.shape==image.shape
   error=np.abs(decoded.astype(float)-image).mean();assert error<5,(key,kind,error)
   print(key,kind,'PAA error',round(error,3))
assert not (r/'packaging/pending-xims-layered-shirt/config.cpp').exists()
with ZipFile(r/'downloads/XIMS-Clothing.zip') as z:assert z.testzip() is None and z.read('GRZ_XimsClothing.pbo')==pbo
print('XIMS native items, exact metadata, flat-normal fallback, custom shorts normal, all prepared shirt maps and pending-item isolation verified. DayZ runtime untested.')
