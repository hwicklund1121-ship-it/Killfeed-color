"""Convert the supplied video to back-panel UV textures without modifying vest geometry."""
import argparse,subprocess,tempfile,json,hashlib
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
from PIL import Image,ImageOps,ImageDraw,ImageFilter
from build_status_textures import encode_paa
from build_pbo import build
r=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('video',type=Path);args=p.parse_args()
s=r/'GRZ_AnimatedHexgrid';art=r/'art/hexgrid-animation'
# Right-hand main body panel on the supplied 2048px XIMS UV texture.
# Inset from panel seams; front/straps/side islands retain their existing pixels.
box=(1040,96,2032,1200)
back_region=(1024,0,2048,1200)
base=Image.new('RGB',(2048,2048),(0,0,0))
assert base.size==(2048,2048)
manifest={'class':'GRZ_Hexgrid_Animated_Anime','source_sha256':hashlib.sha256(args.video.read_bytes()).hexdigest(),'fps':8,'interval_ms':125,'texture_size':[1024,1024],'back_panel_box_2048':box,'back_clear_region_2048':back_region,'base_colour':'solid black','material':'flat_black.rvmat, neutral normal, zero specular and environment','geometry_modified':False,'armour_inherited':'HEXGRID_Base','in_game_uv_alignment_verified':False,'frames':[]}
previews=[];uv_previews=[]
with tempfile.TemporaryDirectory(prefix='grz-hex-frames-') as d:
 subprocess.run(['ffmpeg','-y','-v','error','-i',str(args.video),'-an','-vf','fps=8','-start_number','0',d+'/frame_%03d.png'],check=True)
 paths=sorted(Path(d).glob('*.png'))
 for i,p in enumerate(paths):
  frame=Image.open(p).convert('RGB');canvas=base.copy();ImageDraw.Draw(canvas).rectangle((back_region[0],back_region[1],back_region[2]-1,back_region[3]-1),fill=(0,0,0))
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
count=len(paths);manifest['frame_count']=count;manifest['loop_seconds']=count/8
# GIF supports centiseconds: alternate 120/130ms for an exact average of 125ms.
durations=[120 if i%2==0 else 130 for i in range(count)]
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
(r/'downloads/GRZ_AnimatedHexgrid.pbo').write_bytes(pbo)
manifest['pbo_sha256']=hashlib.sha256(pbo).hexdigest();manifest['pbo_bytes']=len(pbo)
notes='''GRZ Animated Hexgrid — Made By Blake
Class: GRZ_Hexgrid_Animated_Anime
Supplied anime clip loops at 8 texture frames/sec, keeping its approximate original duration. Visual animation only, no audio.
Back-panel artwork is baked into the original UV texture; no floating mesh or model geometry edits. Front, straps and side UV islands are solid black; native material shading retained. 1024px DXT1 textures limit memory/disk cost. All armour and cargo settings inherited from the installed HEXGRID_Base; existing vest classes unchanged.
Custom flat material removes the hex normal pattern, specular shine and environment reflections. All damage stages retain that material. Animation stretches across the full back UV area, with a narrow edge fade into the solid-black vest. A shared hidden image cache retains all 119 frame textures per client; frames that fail to load are skipped instead of applied. Back UV placement uses the right main panel of the supplied XIMS texture and requires a worn male/female in-game check. No DayZ runtime or Enforce compiler available here.
Install combined update on server and client, with existing HEXGRID.pbo present. Select GRZ Animated Hexgrid in /grzvip > GUN RESKINS > OTHER. Requires the existing VIP allowlist. Spawn class using VPP for admin testing. Existing items are not converted.
Client cosmetic timer only, one shared ticker for all copies; no recurring server texture RPCs. Animation phases are local to each viewer.
'''
(r/'ANIMATED-HEXGRID.md').write_text(notes)
(r/'downloads/GRZ-ANIMATED-HEXGRID-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
with ZipFile(r/'downloads/GRZ-Animated-Hexgrid.zip','w',ZIP_DEFLATED) as z:
 z.writestr('GRZ_AnimatedHexgrid.pbo',pbo);z.writestr('INSTALL.txt',notes);z.writestr('MANIFEST.json',json.dumps(manifest,indent=2));z.write(art/'preview.gif','preview.gif');z.write(art/'uv-preview.gif','uv-preview.gif')
(r/'downloads/GRZ-Animated-Hexgrid-Preview.gif').write_bytes((art/'preview.gif').read_bytes())
(r/'downloads/GRZ-Animated-Hexgrid-UV-Preview.gif').write_bytes((art/'uv-preview.gif').read_bytes())
print(f'Built {count} frames, {count/8}s loop, PBO {len(pbo)/1024/1024:.1f} MiB; panel-only changes verified.')
