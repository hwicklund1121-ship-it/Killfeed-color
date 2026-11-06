"""Package upright/inverted cross wrap frames at 750ms, retaining native UV alpha."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json
import numpy as np
from PIL import Image
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1]
a=r/'art/ghillie/cross';s=r/'GRZ_AnimatedGhillie'
alpha=np.asarray(Image.open(r/'art/ghillie/checker/source_alpha.png').convert('L'))
m={'class':'GRZ_Animated_Ghillie_Cross','swap_interval_ms':750,'full_cycle_ms':1500,'frames':{}}
for png,paa in [('frame_upright.png','cross_upright_co.paa'),('frame_upside_down.png','cross_inverted_co.paa')]:
 im=Image.open(a/png).convert('RGBA');assert np.array_equal(np.asarray(im)[:,:,3],alpha)
 data=encode_paa(im,'DXT5',alpha_flags=True);(s/'data'/paa).write_bytes(data)
 m['frames'][png]={'paa_sha256':hashlib.sha256(data).hexdigest(),'png_sha256':hashlib.sha256((a/png).read_bytes()).hexdigest()}
with Image.open(a/'preview.gif') as gif:
 assert gif.n_frames==2
 for i in range(2):gif.seek(i);assert gif.info['duration']==750
pbo=build(s);(r/'downloads/GRZ_AnimatedGhillie.pbo').write_bytes(pbo)
m['pbo_sha256']=hashlib.sha256(pbo).hexdigest()
notes='GRZ Animated Cross Wrap: GRZ_Animated_Ghillie_Cross. Two frames swap every 0.75 seconds. Original 2048px alpha retained. Select in VIP GHILLIE WRAP on compatible guns. Install combined update on server and client; no UAV repack. Existing checker wrap remains available at its original speed. Static/package checks only; DayZ runtime untested.\n'
(r/'ANIMATED-CROSS-GHILLIE.md').write_text(notes)
with ZipFile(r/'downloads/GRZ-Animated-Cross-Wrap.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_AnimatedGhillie.pbo',pbo)
 for n in ['frame_upright.png','frame_upside_down.png','preview.gif']:z.write(a/n,n)
 z.writestr('INSTALL.txt',notes);z.writestr('MANIFEST.json',json.dumps(m,indent=2))
(r/'downloads/GRZ-Cross-Ghillie-Preview.gif').write_bytes((a/'preview.gif').read_bytes())
print('Built cross wrap, preserved alpha, verified 750ms GIF.')
