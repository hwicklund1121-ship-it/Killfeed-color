"""Convert the supplied video to back-panel UV textures without modifying vest geometry."""
import argparse,subprocess,tempfile,json,hashlib
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
from PIL import Image,ImageOps,ImageDraw,ImageFilter
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('video',type=Path);p.add_argument('--variant',choices=['white','black'],required=True);args=p.parse_args()
white=args.variant=='white'
addon='GRZ_AnimatedHexgridWhite' if white else 'GRZ_AnimatedHexgridBlackV2'
item='GRZ_Hexgrid_Animated_White' if white else 'GRZ_Hexgrid_Animated_Black_V2'
label='GRZ Animated Hexgrid White' if white else 'GRZ Animated Hexgrid Black V2'
slug='GRZ-Animated-Hexgrid-White' if white else 'GRZ-Animated-Hexgrid-Black-V2'
colour=(255,255,255) if white else (0,0,0)
fps=8 if white else 6
interval=round(1000/fps)
s=r/addon;art=r/'art'/('hexgrid-animation-white' if white else 'hexgrid-animation-black-v2')
(s/'data').mkdir(parents=True,exist_ok=True);(s/'scripts/4_World').mkdir(parents=True,exist_ok=True);(s/'gui/layouts').mkdir(parents=True,exist_ok=True);art.mkdir(parents=True,exist_ok=True)
# Each variant owns its ticker/cache; it can coexist with the first vest.
template=r/'GRZ_AnimatedHexgrid'
if not (s/'config.cpp').exists():
 config=(template/'config.cpp').read_text().replace('GRZ_Hexgrid_Animated_Anime',item).replace('GRZ_AnimatedHexgrid',addon).replace('GRZ Animated Hexgrid',label)
 (s/'config.cpp').write_text(config)
 code=(template/'scripts/4_World/GRZAnimatedHexgrid.c').read_text().replace('GRZ_Hexgrid_Animated_Anime',item).replace('GRZHexgridAnimationTicker',addon+'Ticker').replace('GRZ_AnimatedHexgrid',addon).replace('GRZ Animated Hexgrid',label).replace('CallLater(Tick, 125, true)',f'CallLater(Tick, {interval}, true)')
 (s/'scripts/4_World/GRZAnimatedHexgrid.c').write_text(code)
 (s/'data/flat_black.rvmat').write_bytes((template/'data/flat_black.rvmat').read_bytes())
 (s/'gui/layouts/frame_cache.layout').write_bytes((template/'gui/layouts/frame_cache.layout').read_bytes())
 (s/'$PBOPREFIX$').write_text(addon+'\n')
# Right-hand main body panel on the supplied 2048px XIMS UV texture.
# Inset from panel seams; front/straps/side islands retain their existing pixels.
box=(1040,96,2032,1200)
back_region=(1024,0,2048,1200)
base=Image.new('RGB',(2048,2048),colour)
assert base.size==(2048,2048)
manifest={'vip_only':True,'class':item,'source_sha256':hashlib.sha256(args.video.read_bytes()).hexdigest(),'fps':fps,'interval_ms':interval,'texture_size':[1024,1024],'back_panel_box_2048':box,'back_clear_region_2048':back_region,'base_colour':'solid white' if white else 'solid black','material':'flat_black.rvmat, neutral normal, zero specular and environment','geometry_modified':False,'armour_inherited':'HEXGRID_Base','in_game_uv_alignment_verified':False,'frames':[]}
previews=[];uv_previews=[]
with tempfile.TemporaryDirectory(prefix='grz-hex-frames-') as d:
 subprocess.run(['ffmpeg','-y','-v','error','-i',str(args.video),'-an','-vf',f'fps={fps}','-start_number','0',d+'/frame_%03d.png'],check=True)
 paths=sorted(Path(d).glob('*.png'))
 for i,p in enumerate(paths):
  frame=Image.open(p).convert('RGB');canvas=base.copy();ImageDraw.Draw(canvas).rectangle((back_region[0],back_region[1],back_region[2]-1,back_region[3]-1),fill=colour)
  panel=frame.resize((box[2]-box[0],box[3]-box[1]),Image.Resampling.LANCZOS)
  # Fade the perimeter into the black fabric, avoiding a sharp rectangular seam.
  edge=Image.new("L",panel.size,0);ImageDraw.Draw(edge).rectangle((4,4,panel.width-5,panel.height-5),fill=255);edge=edge.filter(ImageFilter.GaussianBlur(2))
  canvas.paste(panel,box[:2],edge)
  if i==0:
   canvas.save(art/'uv-first-frame.png')
   # Inspect changed pixels are confined to the declared panel.
   from PIL import ImageChops
   bounds=ImageChops.difference(base,canvas).getbbox();assert bounds and bounds[0]>=back_region[0] and bounds[1]>=back_region[1] and bounds[2]<=back_region[2] and bounds[3]<=back_region[3]
  tex=canvas.resize((1024,1024),Image.Resampling.LANCZOS).convert('RGBA')
  data=encode_paa(tex,'DXT1');name=f'frame_{i:03d}.paa';(s/'data'/name).write_bytes(data)
  manifest['frames'].append({'path':name,'sha256':hashlib.sha256(data).hexdigest()})
  previews.append(panel.resize((320,475),Image.Resampling.LANCZOS));uv_previews.append(canvas.resize((768,768),Image.Resampling.LANCZOS))
count=len(paths);manifest['frame_count']=count;manifest['loop_seconds']=count*interval/1000
# GIF supports centiseconds: alternate 120/130ms for an exact average of 125ms.
durations=[round((i+1)*interval/10)*10-round(i*interval/10)*10 for i in range(count)]
uv_previews[0].save(art/'uv-preview.gif',save_all=True,append_images=uv_previews[1:],duration=durations,loop=0,disposal=2)
previews[0].save(art/'preview.gif',save_all=True,append_images=previews[1:],duration=durations,loop=0,disposal=2)
script=(s/'scripts/4_World/GRZAnimatedHexgrid.c').read_text().replace('FRAME_COUNT = 0',f'FRAME_COUNT = {count}')
# A repeat build can change the number of frames.
import re
script=re.sub(r'FRAME_COUNT = \d+',f'FRAME_COUNT = {count}',script)
(s/'scripts/4_World/GRZAnimatedHexgrid.c').write_text(script)
for stale in (s/'data').glob('frame_*.paa'):
 if int(stale.stem.split('_')[1])>=count:stale.unlink()
pbo=build(s);assert len(pbo)<100*1024*1024
(r/'downloads'/f'{addon}.pbo').write_bytes(pbo)
manifest['pbo_sha256']=hashlib.sha256(pbo).hexdigest();manifest['pbo_bytes']=len(pbo)
notes=f"""{label} — Made By Blake
Class: {item}
Full uploaded clip stretched across the full back UV area. Surrounding vest: {'solid white' if white else 'solid black'}. Visual animation only; no audio.
{count} frames at {fps} fps; runtime interval {interval}ms. Flat neutral normal map, no specular or environment shine. Shared client image cache holds loaded frames and skips failures. Native geometry unchanged. Armour settings copied unchanged from balanced HEXGRID_Base.
Requires existing HEXGRID addon and GRZ_Loadout. Combined installer copies to both client and server. Select {label} under /grzvip > GUN RESKINS > OTHER. The existing VIP allowlist is required. Spawn class {item} using VPP for admin testing. Existing variants retained. Worn alignment and flicker still require a DayZ runtime test.
"""
(r/f'{slug.upper()}.md').write_text(notes)
(r/'downloads'/f'{slug}-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
with ZipFile(r/'downloads'/f'{slug}.zip','w',ZIP_DEFLATED) as z:
 z.writestr(f'{addon}.pbo',pbo);z.writestr('INSTALL.txt',notes);z.writestr('MANIFEST.json',json.dumps(manifest,indent=2));z.write(art/'preview.gif','preview.gif');z.write(art/'uv-preview.gif','uv-preview.gif')
(r/'downloads'/f'{slug}-Preview.gif').write_bytes((art/'uv-preview.gif').read_bytes())
print(f'Built {label}: {count} frames, {count*interval/1000}s loop, {len(pbo)/1024/1024:.1f} MiB; back-only mapping verified.')
