"""Package verified vanilla XIMS items; include the verified Windstride layered-shirt reskin."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import argparse,hashlib,io,json,struct,shutil
from PIL import Image,ImageOps
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
for key in ['sneakers','shorts','shirt','shirt-model','shirt-config','canvas-colour']:p.add_argument('--'+key,type=Path,required=True)
a=p.parse_args();s=r/'GRZ_XimsClothing';layer=r/'GRZ_XimsLayeredShirt'
for supplied in vars(a).values():
 assert supplied.is_file(),str(supplied)
report={'items':[],'pending':[],'pending_custom_normals':['XIMS Canvas Bag']}

def convert(zip_path,key,target,colour_only=False):
 textures={}
 with ZipFile(zip_path) as z:
  for kind,ending in [('co','_Base_color.png'),('nohq','_Normal.png')]:
   if kind=='nohq' and colour_only:
    textures[kind]={'mode':'custom normal upload oversized; native normal/material inherited'};continue
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
for key,cls,name,parent in [('sneakers','XIMS_Sneakers','XIMS Sneakers','Sneakers_Black'),('shorts','XIMS_JeanShorts','XIMS Jean Shorts','ShortJeans_Blue'),('bag','XIMS_CanvasBag','XIMS Canvas Bag','CanvasBag_Olive')]:
 report['items'].append({'class':cls,'displayName':name,'descriptionShort':'Made By Blake','base_class':parent,'key':key,'textures':convert(a.canvas_colour if key=='bag' else getattr(a,key),key,s,colour_only=(key=='bag'))})
material=(r/'packaging/customs/GRZ_XimsKit/data/pants.rvmat').read_text().replace('GRZ_XimsKit\\data\\pants_nohq.paa','GRZ_XimsClothing\\data\\shorts_nohq.paa')
(s/'data/shorts.rvmat').write_text(material)
original=a.shirt_config.read_text(encoding='utf-8-sig')
for token in ['class Layered_Shirt','class Layered_Shirt_Base: Clothing','hiddenSelections[] = {"zbytek"}', 'layered_shirt_m.p3d','layered_shirt_f.p3d','layered_shirt_g.p3d']:
 assert token in original,token
shirt=convert(a.shirt,'shirt',layer)
material=(r/'packaging/customs/GRZ_XimsKit/data/pants.rvmat').read_text().replace('GRZ_XimsKit\\data\\pants_nohq.paa','GRZ_XimsLayeredShirt\\data\\shirt_nohq.paa')
(layer/'data/shirt.rvmat').write_text(material)
report['items'].append({'class':'XIMS_LayeredShirt','displayName':'XIMS Layered Shirt','descriptionShort':'Made By Blake','base_class':'Layered_Shirt_Base','required_patch':'Layered_Shirt','key':'shirt','textures':shirt})
report['verified_windstride_config_sha256']=hashlib.sha256(a.shirt_config.read_bytes()).hexdigest()
report['provided_female_mlod_sha256']=hashlib.sha256(a.shirt_model.read_bytes()).hexdigest()
report['addons']={}
for addon in ['GRZ_XimsClothing','GRZ_XimsLayeredShirt']:
 data=build(r/addon);(r/'downloads'/(addon+'.pbo')).write_bytes(data)
 report['addons'][addon]=hashlib.sha256(data).hexdigest()
report['pbo_sha256']=report['addons']['GRZ_XimsClothing']
(r/'downloads/XIMS-Clothing-MANIFEST.json').write_text(json.dumps(report,indent=2)+'\n')
old_staging=r/'packaging/pending-xims-layered-shirt'
if old_staging.exists():shutil.rmtree(old_staging)
notes=(r/'packaging/xims-clothing/INSTALL.txt').read_text()
(r/'XIMS-CLOTHING.md').write_text(notes)
with ZipFile(r/'downloads/XIMS-Clothing.zip','w',ZIP_DEFLATED) as z:
 for addon in report['addons']:
  z.write(r/'downloads'/(addon+'.pbo'),addon+'.pbo')
  for f in sorted((r/addon).rglob('*')):
   if f.is_file() and f.suffix!='.paa':z.write(f,'source/'+addon+'/'+f.relative_to(r/addon).as_posix())
 z.writestr('MANIFEST.json',json.dumps(report,indent=2)+'\n');z.writestr('INSTALL.txt',notes)
print('Built all four XIMS clothing items; verified layered-shirt bindings. Canvas bag custom normal remains pending.')
