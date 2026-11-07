"""Validate all four XIMS skins, verified Windstride bindings and explicit normal fallbacks."""
from pathlib import Path
from zipfile import ZipFile
import argparse,hashlib,io,json,re,struct
import numpy as np
from PIL import Image
from repair_hexgrid import read_pbo
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
for key in ['sneakers','shorts','shirt','shirt-config','canvas-colour']:p.add_argument('--'+key,type=Path,required=True)
a=p.parse_args();m=json.loads((r/'downloads/XIMS-Clothing-MANIFEST.json').read_text())
assert len(m['items'])==4 and m['pending']==[] and m['pending_custom_normals']==['XIMS Canvas Bag']
all_files={};pbos={}
for addon,sha in m['addons'].items():
 source=r/addon;data=(r/'downloads'/(addon+'.pbo')).read_bytes();ext,files=read_pbo(data)
 assert ext['prefix']==addon and hashlib.sha1(data[:-21]).digest()==data[-20:]
 assert hashlib.sha256(data).hexdigest()==sha
 assert files=={p.relative_to(source).as_posix():p.read_bytes() for p in source.rglob('*') if p.is_file() and p.name!='$PBOPREFIX$'}
 pbos[addon]=data;all_files[addon]=files
 cfg=files['config.cpp'].decode()
 for field in ['DamageSystem','hitpoints','heatIsolation','itemsCargoSize','inventorySlot','GRZVipOnly','model =','class ClothingTypes']:
  assert field not in cfg,(addon,field)
 for resource in re.findall(r'"(GRZ_[^\"]+\\[^\"]+)"',cfg+'\n'+'\n'.join(v.decode() for k,v in files.items() if k.endswith('.rvmat'))):
  assert resource.removeprefix(addon+'\\').replace('\\','/') in files,resource
layer_cfg=all_files['GRZ_XimsLayeredShirt']['config.cpp'].decode()
assert 'requiredAddons[] = {"DZ_Data","Layered_Shirt"};' in layer_cfg
original=a.shirt_config.read_text(encoding='utf-8-sig')
assert hashlib.sha256(a.shirt_config.read_bytes()).hexdigest()==m['verified_windstride_config_sha256']
for token in ['class Layered_Shirt','class Layered_Shirt_Base: Clothing','hiddenSelections[] = {"zbytek"}','layered_shirt_m.p3d','layered_shirt_f.p3d','layered_shirt_g.p3d']:assert token in original
assert 'hiddenSelections[] = {"zbytek"};' in layer_cfg
native=all_files['GRZ_XimsClothing'];native_cfg=native['config.cpp'].decode()
assert 'data/sneakers_nohq.paa' not in native and 'data/bag_nohq.paa' not in native
assert 'data/sneakers.rvmat' not in native and 'data/bag.rvmat' not in native
assert 'CanvasBag_Olive' in native_cfg and 'DZ_Characters_Backpacks' in native_cfg

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
 pixels=np.array(Image.open(io.BytesIO(head.getvalue()[:128]+raw)).convert('RGBA'))
 if 'SWIZ' in tags:
  assert tags['SWIZ']==bytes([5,4,2,3]);alpha=pixels[:,:,3].copy();red=pixels[:,:,0].copy();pixels[:,:,0]=255-alpha;pixels[:,:,3]=255-red
 return pixels[:,:,:3]
for item in m['items']:
 key=item['key'];addon='GRZ_XimsLayeredShirt' if key=='shirt' else 'GRZ_XimsClothing';files=all_files[addon];cfg=files['config.cpp'].decode()
 assert 'class '+item['class']+': '+item['base_class'] in cfg
 assert 'displayName = "'+item['displayName']+'";' in cfg and 'descriptionShort = "Made By Blake";' in cfg
 path=a.canvas_colour if key=='bag' else getattr(a,key)
 with ZipFile(path) as z:
  for kind,ending in [('co','_Base_color.png'),('nohq','_Normal.png')]:
   meta=item['textures'][kind]
   if key=='bag' and kind=='nohq':assert 'inherited' in meta['mode'];continue
   raw=z.read(next(n for n in z.namelist() if n.endswith(ending)))
   assert hashlib.sha256(raw).hexdigest()==meta['source_sha256']
   image=np.array(Image.open(io.BytesIO(raw)).convert('RGB'))
   if key=='sneakers' and kind=='nohq':assert (image==[128,128,255]).all();continue
   if kind=='nohq':image[:,:,1]=255-image[:,:,1]
   data=files['data/'+key+'_'+kind+'.paa'];assert hashlib.sha256(data).hexdigest()==meta['paa_sha256']
   decoded=decode(data);assert decoded.shape==image.shape
   error=np.abs(decoded.astype(float)-image).mean();assert error<5,(key,kind,error)
   print(key,kind,'PAA error',round(error,3))
assert not (r/'packaging/pending-xims-layered-shirt').exists()
with ZipFile(r/'downloads/XIMS-Clothing.zip') as z:
 assert z.testzip() is None
 for name,data in pbos.items():assert z.read(name+'.pbo')==data
print('All four XIMS names/descriptions, native stats, confirmed Windstride dependency/models/selection, six PAA conversions and normal fallbacks passed. DayZ runtime untested.')
