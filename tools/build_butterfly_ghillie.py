"""Build XIMS butterfly wrap with original UV alpha and a 1000ms color swap."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib
import numpy as np
from PIL import Image,ImageChops
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1];art=r/'art/ghillie/butterfly';src=r/'GRZ_AnimatedGhillie'
alpha=Image.open(r/'art/ghillie/checker/source_alpha.png').convert('L')
sheet=Image.open(art/'motifs.png').convert('RGBA')
motifs=[]
for box in [(0,0,864,660),(0,660,864,1230),(0,1230,864,1821)]:
 m=sheet.crop(box);m=m.crop(m.getbbox());m.thumbnail((270,270),Image.Resampling.LANCZOS);motifs.append(m)
pattern=Image.new('L',alpha.size,0)
for row,y in enumerate(range(0,2048,320)):
 for col,x in enumerate(range(-160 if row%2 else 0,2048,320)):
  m=motifs[(row+col)%3].getchannel('A');pattern.paste(m,(x+(320-m.width)//2,y+(320-m.height)//2))
frames=[];manifest={'class':'GRZ_Animated_Ghillie_XIMS_V2','interval_ms':1000,'cycle_ms':2000,'frames':{}}
for name,bg,fg in [('purple',(181,65,255),(0,0,0)),('black',(0,0,0),(181,65,255))]:
 im=Image.composite(Image.new('RGBA',alpha.size,fg+(255,)),Image.new('RGBA',alpha.size,bg+(255,)),pattern);im.putalpha(alpha)
 assert np.array_equal(np.asarray(im)[:,:,3],np.asarray(alpha))
 im.save(art/f'{name}.png');data=encode_paa(im,'DXT5',alpha_flags=True);(src/'data'/f'butterfly_{name}_co.paa').write_bytes(data)
 manifest['frames'][name]=hashlib.sha256(data).hexdigest()
 preview=Image.new('RGBA',im.size,(112,112,112,255));preview.alpha_composite(im);frames.append(preview.convert('RGB').resize((768,768),Image.Resampling.LANCZOS))
frames[0].save(art/'preview.gif',save_all=True,append_images=frames[1:],duration=1000,loop=0,disposal=2)
with Image.open(art/'preview.gif') as check:
 assert check.n_frames==2
 for i in range(2):check.seek(i);assert check.info['duration']==1000
pbo=build(src);(r/'downloads/GRZ_AnimatedGhillie.pbo').write_bytes(pbo);manifest['pbo_sha256']=hashlib.sha256(pbo).hexdigest()
notes='XIMS Ghillie V2. Description: Made By Blake. Purple with black butterflies / black with purple butterflies. Swap every 1 second. Original 2048px UV alpha preserved. Native wrap model, compatible guns only. Select in VIP GHILLIE WRAP; install on client and server. Existing wraps remain available. Package and image checks passed; DayZ runtime untested.\n'
(r/'ANIMATED-BUTTERFLY-GHILLIE.md').write_text(notes)
with ZipFile(r/'downloads/GRZ-XIMS-Ghillie-V2.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_AnimatedGhillie.pbo',pbo)
 for n in ['purple.png','black.png','preview.gif']:z.write(art/n,n)
 z.writestr('INSTALL.txt',notes);z.writestr('MANIFEST.json',json.dumps(manifest,indent=2))
(r/'downloads/GRZ-XIMS-Ghillie-V2-Preview.gif').write_bytes((art/'preview.gif').read_bytes())
print('Built XIMS Ghillie V2; exact alpha and 1000ms timing verified.')
