"""Build the supplied XIMS CS5 painted textures as an additive model-inheriting reskin."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import argparse,io,json,hashlib
from PIL import Image
from build_pbo import build
from build_status_textures import encode_paa
p=argparse.ArgumentParser();p.add_argument('textures_zip',type=Path);args=p.parse_args()
r=Path(__file__).resolve().parents[1];s=r/'GRZ_XimsCS5'
m={'class':'GRZ_XIMS_CS5','displayName':'XIMS CS5','descriptionShort':'Made By Blake','base_class':'UAV_CS5_BLACK','selection_order':['color','color2','color3','color4'],'textures':{},'normal_maps':'Native CS5 NOHQ/SMDI and material detail retained; supplied exported normals are constant flat normal maps.','model':'Inherited original CS5 model; FBX recompilation not required.'}
with ZipFile(args.textures_zip) as z:
 for part in ['cs5_receiver','cs5_stock','cs5_barrel','suppressor']:
  names=[n for n in z.namelist() if Path(n).name.startswith(part+'_co.paa_') and n.endswith('_Base_color.png')];assert len(names)==1
  raw=z.read(names[0]);im=Image.open(io.BytesIO(raw)).convert('RGBA');assert im.size==(4096,4096)
  paa=encode_paa(im,'DXT1');(s/'data'/f'{part}_co.paa').write_bytes(paa)
  m['textures'][part]={'source_png_sha256':hashlib.sha256(raw).hexdigest(),'paa_sha256':hashlib.sha256(paa).hexdigest(),'size':list(im.size)}
pbo=build(s);(r/'downloads/GRZ_XimsCS5.pbo').write_bytes(pbo);m['pbo_sha256']=hashlib.sha256(pbo).hexdigest()
notes='''XIMS CS5 — Made By Blake
Class: GRZ_XIMS_CS5
Additive reskin of UAV_CS5_BLACK. Uses the four supplied 4096px painted textures in the native receiver / stock / barrel / suppressor selection order. The existing compiled model, native normal/specular materials, magazine, ammo and weapon behaviour are inherited. The provided FBX does not need recompiling. The supplied exported normal PNGs are flat; native CS5 material details are retained.
Requires the existing CS5 addon (CS5_Base). VIP-only in the existing GRZ VIP gun selector; expected under .50 CAL when the installed base CS5 is configured for .50 CAL. The actual installed base weapon defines ammo and behaviour.
Install combined update on both client and server, restart both, and choose XIMS CS5 in /grzvip > GUN RESKINS. No UAV weapon-pack repack required. Admin spawn class: GRZ_XIMS_CS5.
Metadata, selection order, texture resources and packaging checked; DayZ rendering remains untested.
'''
(r/'XIMS-CS5.md').write_text(notes)
(r/'downloads/XIMS-CS5-MANIFEST.json').write_text(json.dumps(m,indent=2)+'\n')
with ZipFile(r/'downloads/XIMS-CS5.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_XimsCS5.pbo',pbo);z.writestr('INSTALL.txt',notes);z.writestr('MANIFEST.json',json.dumps(m,indent=2))
print('Built XIMS CS5 reskin with exact requested display name/description and four 4K textures.')
