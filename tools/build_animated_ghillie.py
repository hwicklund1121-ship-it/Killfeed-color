"""Build two exact inverse checker frames using the user-approved original alpha mask."""
import argparse,hashlib,io,json,struct
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import numpy as np
from PIL import Image
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--source-png',type=Path)
a=p.parse_args()
art=r/'art/ghillie/checker';art.mkdir(parents=True,exist_ok=True)
if a.source_png:
 original=Image.open(a.source_png).convert('RGBA');alpha=original.getchannel('A')
 alpha.save(art/'source_alpha.png')
else:alpha=Image.open(art/'source_alpha.png').convert('L')
w,h=alpha.size;assert (w,h)==(2048,2048)
mask=np.asarray(alpha)
y,x=np.indices((h,w));board=(((x//256+y//256)%2)*255).astype(np.uint8)
source=r/'GRZ_AnimatedGhillie';(source/'data').mkdir(parents=True,exist_ok=True)
frames=[];report={}
for name,values in [('a',board),('b',255-board)]:
 rgba=np.empty((h,w,4),dtype=np.uint8);rgba[:,:,:3]=values[:,:,None];rgba[:,:,3]=mask
 image=Image.fromarray(rgba)
 png=art/f'checker_{name}.png';image.save(png)
 paa=encode_paa(image,'DXT5',alpha_flags=True);(source/f'data/checker_{name}_co.paa').write_bytes(paa)
 # Decode the actual largest PAA mip and check its RGB and alpha against the PNG.
 offset=struct.unpack_from('<I',paa,14)[0];mw,mh=struct.unpack_from('<HH',paa,offset);length=int.from_bytes(paa[offset+4:offset+7],'little');blocks=paa[offset+7:offset+7+length]
 assert (mw,mh)==(w,h) and len(blocks)==w*h
 dds=io.BytesIO();Image.new('RGBA',(w,h)).save(dds,format='DDS',pixel_format='DXT5')
 decoded=np.asarray(Image.open(io.BytesIO(dds.getvalue()[:128]+blocks)).convert('RGBA'))
 assert np.array_equal(decoded,rgba),'Encoded PAA changed frame pixels'
 frames.append(image)
 report[name]={'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'paa_sha256':hashlib.sha256(paa).hexdigest()}
a0=np.asarray(frames[0]);b0=np.asarray(frames[1]);assert np.array_equal(a0[:,:,3],mask) and np.array_equal(b0[:,:,3],mask)
assert np.all(a0[:,:,:3].astype(np.uint16)+b0[:,:,:3].astype(np.uint16)==255)
# Neutral gray is used only behind the GIF preview; PNGs and PAAs retain alpha.
gif_frames=[]
for frame in frames:
 preview=Image.alpha_composite(Image.new('RGBA',(w,h),(112,112,112,255)),frame).convert('RGB')
 gif_frames.append(preview)
preview=art/'checker_preview.gif'
gif_frames[0].save(preview,save_all=True,append_images=gif_frames[1:],duration=[250,250],loop=0,disposal=2,optimize=False)
with Image.open(preview) as check:
 assert check.n_frames==2 and check.info['duration']==250
 check.seek(1);assert check.info['duration']==250
script=source/'scripts/4_World/GRZAnimatedCheckerWrap.c';text=script.read_text().replace('INTERVAL_MS = 500','INTERVAL_MS = 250');script.write_text(text)
manifest={'class':'GRZ_Animated_Ghillie_Checker','size':[w,h],'checker_grid':[8,8],'cell_pixels':256,'swap_interval_ms':250,'full_cycle_ms':500,'alpha_preserved_exactly':True,'encoded_top_mips_lossless_against_png':True,'source_alpha_sha256':hashlib.sha256(mask.tobytes()).hexdigest(),'frames':report}
pbo=build(source);(r/'downloads/GRZ_AnimatedGhillie.pbo').write_bytes(pbo)
manifest['pbo_sha256']=hashlib.sha256(pbo).hexdigest()
notes='''GRZ Animated Checker Wrap — Made by Blake

Gun-wrap class: GRZ_Animated_Ghillie_Checker
Display name: GRZ Animated Checker Wrap
VIP-only in the existing loadout ghillie selector. Requires the current GRZ_Loadout addon.
Native GhillieAtt_ColorBase model and weaponWrap slot are inherited.

Two 2048x2048 textures use the original alpha mask pixel-for-pixel.
Frame B is the exact RGB inverse of frame A: black and white swap in place.
Swap every 0.25 seconds, full A/B cycle every 0.5 seconds. Eight by eight checks.
PNG and PAA transparency is retained. The GIF has a neutral gray preview background only.

One client-cosmetic timer serves all active wraps, including local gun previews.
No dedicated-server animation timer or recurring animation RPCs. Visual phases are local to each viewer.
Timer unregisters each deleted wrap and stops when none remain.

The combined GRZ-Arena-Menu-Update.zip installer copies this addon to both client and server.
Alternatively copy only GRZ_AnimatedGhillie.pbo to @GRZ_Local/Addons on both sides, keeping GRZ_Loadout installed.
Restart server and client. Open the VIP GUN RESKINS tab and choose the checker wrap under GHILLIE WRAP.
No UAV weapon-pack repack required. Existing gun must support a native ghillie wrap.

Exact alpha/RGB inversion, decoded PAA pixels, GIF timing, asset paths and package checked.
No DayZ runtime or Enforce compiler available; verify held-gun appearance and animation in game.
'''
(r/'ANIMATED-CHECKER-GHILLIE.md').write_text(notes)
(r/'downloads/GRZ-ANIMATED-GHILLIE-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
with ZipFile(r/'downloads/GRZ-Animated-Checker-Wrap.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_AnimatedGhillie.pbo',pbo)
 for name in ['checker_a.png','checker_b.png','checker_preview.gif']:z.write(art/name,name)
 z.writestr('INSTALL.txt',notes);z.writestr('MANIFEST.json',json.dumps(manifest,indent=2)+'\n')
(r/'downloads/GRZ-Checker-Ghillie-Preview.gif').write_bytes(preview.read_bytes())
print('Built inverse 2048px frames, exact-alpha DXT5 PAAs, 250ms GIF, PBO and texture ZIP.')
