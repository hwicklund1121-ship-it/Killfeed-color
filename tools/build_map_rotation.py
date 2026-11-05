"""Build the three-arena GRZ rotation addon from the supplied DZE placement data."""
import io,json,math,os,hashlib,subprocess,struct,argparse
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
os.environ.setdefault('MPLCONFIGDIR','/tmp/grz-mpl-cache')
os.environ.setdefault('XDG_CACHE_HOME','/tmp/grz-font-cache')
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from scipy.spatial import cKDTree,ConvexHull
from scipy.sparse import coo_matrix
from scipy.sparse.csgraph import connected_components
from PIL import Image
from inspect_grz_dze import read
from build_pbo import build
from build_status_textures import encode_paa
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('dze',type=Path);args=parser.parse_args()
source=root/'GRZ_MapRotation';out=root/'downloads';out.mkdir(exist_ok=True)
dze,sanitized=read(args.dze)
objects=dze['EditorObjects'];points=np.array([[o['Position'][0],o['Position'][2]] for o in objects])
pairs=cKDTree(points).query_pairs(180);rows=[];cols=[]
for a,b in pairs:rows.extend([a,b]);cols.extend([b,a])
_,groups=connected_components(coo_matrix((np.ones(len(rows)),(rows,cols)),shape=(len(points),len(points))),directed=False)
# Match known geographic centres, not unstable cluster numbering.
selections=[('electro_hill','Electro Hill',(8997,2562)),('nwaf','NWAF',(4462,10268)),('neaf','NEAF',(12205,12531))]
settings=dict(RoundSeconds=1800,VoteSeconds=120,PopupSeconds=30,OutsideSeconds=5,StartArena='nwaf',Arenas=[])
report=[]
blueprints=root/'previews/map-rotation';blueprints.mkdir(exist_ok=True)

def inside(point,polygon):
 x,z=point;hit=False;j=len(polygon)-1
 for i,(ax,az) in enumerate(polygon):
  bx,bz=polygon[j]
  if (az>z)!=(bz>z) and x<(bx-ax)*(z-az)/(bz-az)+ax:hit=not hit
  j=i
 return hit

for arena_id,name,centre in selections:
 nearest=int(np.argmin(np.linalg.norm(points-np.array(centre),axis=1)));group=groups[nearest]
 indices=np.flatnonzero(groups==group);p=points[indices];boundary=p[ConvexHull(p).vertices]
 candidates=[]
 for x in np.arange(p[:,0].min()+12,p[:,0].max()-10,18):
  for z in np.arange(p[:,1].min()+12,p[:,1].max()-10,18):
   if not inside((x,z),boundary):continue
   if np.linalg.norm(p-[x,z],axis=1).min()<8:continue
   # Keep proposed spawns away from the placement boundary.
   distances=[]
   for i,a in enumerate(boundary):
    b=boundary[(i+1)%len(boundary)];edge=b-a;t=np.clip(np.dot(np.array([x,z])-a,edge)/np.dot(edge,edge),0,1)
    distances.append(np.linalg.norm(np.array([x,z])-(a+t*edge)))
   if min(distances)<10:continue
   candidates.append([float(x),float(z)])
 assert len(candidates)>=2
 pool=np.array(candidates);chosen=[int(np.argmin(np.linalg.norm(pool-pool.mean(0),axis=1)))];minimum=np.full(len(pool),np.inf)
 while len(chosen)<min(80,len(pool)):
  minimum=np.minimum(minimum,np.linalg.norm(pool-pool[chosen[-1]],axis=1));minimum[chosen]=-1
  chosen.append(int(np.argmax(minimum)))
 settings['Arenas'].append(dict(Id=arena_id,Name=name,Blueprint=f'GRZ_MapRotation/data/{arena_id}.paa',Boundary=[dict(X=round(float(x),3),Z=round(float(z),3)) for x,z in boundary],SpawnCandidates=[dict(X=round(float(pool[i,0]),3),Z=round(float(pool[i,1]),3)) for i in chosen]))
 fig,ax=plt.subplots(figsize=(8,8),facecolor='black');ax.set_facecolor('black')
 closed=np.vstack([boundary,boundary[0]]);ax.plot(closed[:,0],closed[:,1],color='white',linewidth=2)
 for index in indices:
  o=objects[index];x,y,z=o['Position'];typ=o['Type'].lower()
  if 'container' in typ:
   ax.scatter([x],[z],s=22,marker='s',facecolors='none',edgecolors='white',linewidths=.65)
  elif 'wall' in typ or 'barrier' in typ:
   angle=math.radians(o['Orientation'][0]);length=4 if '20' not in typ else 10
   dx=math.cos(angle)*length;dz=math.sin(angle)*length
   ax.plot([x-dx,x+dx],[z-dz,z+dz],color='white',linewidth=.6)
  else:ax.scatter([x],[z],s=6,c='white',marker='.')
 ax.set_aspect('equal');ax.margins(.10);ax.axis('off');fig.subplots_adjust(left=0,right=1,top=1,bottom=0)
 png=blueprints/(arena_id+'-blueprint.png');fig.savefig(png,dpi=128,facecolor='black');fig.savefig(blueprints/(arena_id+'-blueprint.svg'),facecolor='black');plt.close(fig)
 image=Image.open(png).convert('RGBA');assert image.size==(1024,1024)
 (source/'data'/f'{arena_id}.paa').write_bytes(encode_paa(image,'DXT1'))
 report.append(dict(id=arena_id,name=name,objects=len(indices),boundary_points=len(boundary),spawn_candidates=len(chosen),centre=centre))
(source/'data/default-config.json').write_text(json.dumps(settings,indent=2)+'\n')
map_dir=root/'packaging/map-rotation/EditorFiles';map_dir.mkdir(parents=True,exist_ok=True)
(map_dir/'GRZ_BBC_DM_2.dze').write_bytes(sanitized)
assert read(map_dir/'GRZ_BBC_DM_2.dze')[0]==dze
# Own short UI chimes: distinct vote, warning and arena-change sequences.
for name,frequency in [('vote',880),('warning',620),('change',1040)]:
 subprocess.run(['ffmpeg','-v','error','-y','-f','lavfi','-i',f'sine=frequency={frequency}:sample_rate=44100:duration=0.35','-af','afade=t=in:st=0:d=0.015,afade=t=out:st=0.17:d=0.18,volume=1.8','-ac','1','-c:a','libvorbis','-q:a','5',str(source/'sounds'/f'{name}.ogg')],check=True)

def widget(kind,name,x,y,w,h,properties='',children=''):
 return f'{kind} {name}\n{{\n position {x} {y}\n size {w} {h}\n hexactpos 0\n vexactpos 0\n hexactsize 0\n vexactsize 0\n '+properties+'\n'+(' {\n'+children+' }\n' if children else '')+'}\n'
def panel(name,x,y,w,h,color):return widget('PanelWidgetClass',name,x,y,w,h,'ignorepointer 1\n style rover_sim_colorable\n color '+color)
def text(name,value,x,y,w,h,size=24):return widget('TextWidgetClass',name,x,y,w,h,f'ignorepointer 1\n color 1 1 1 1\n font "gui/fonts/sdf_MetronLight{size}"\n "exact text" 1\n "text halign" center\n "text valign" center\n text "{value}"')
children=panel('Background',0,0,1,1,'0.015 0.015 0.02 0.98')+panel('TopAccent',0,0,1,.004,'1 1 1 1')
children+=widget('ImageWidgetClass','Logo',.025,.025,.25,.12,'ignorepointer 1\n color 1 1 1 1\n mode blend\n stretch 1')
children+=text('Title','VOTE FOR THE NEXT ARENA',.31,.05,.6,.065)
children+=widget('ButtonWidgetClass','Close',.94,.03,.035,.05,'color 1 1 1 1',text('CloseLabel','X',0,0,1,1))
children+=text('Countdown','',.10,.18,.80,.045,16)
for i in range(6):
 col=i%3;row=i//3
 card=panel('Backing'+str(i),0,0,1,1,'0.065 0.065 0.075 1')
 card+=widget('ImageWidgetClass','Blueprint'+str(i),.025,.13,.95,.72,'ignorepointer 1\n color 1 1 1 1\n mode blend\n stretch 1')
 card+=text('Name'+str(i),'',.025,.01,.95,.11,24)+text('Votes'+str(i),'',.025,.87,.95,.11,16)
 card+=panel('Selected'+str(i),0,.99,1,.01,'1 1 1 1')
 button=widget('ButtonWidgetClass','Vote'+str(i),0,0,1,1,'color 1 1 1 1',card)
 children+=widget('FrameWidgetClass','Card'+str(i),.035+col*.32,.28+row*.30,.29,.27,'color 0 0 0 0',button)
children+=text('Footer','One vote each. Click another arena to change it. Reopen with !vote or F7.',.04,.925,.92,.045,16)
layout=widget('FrameWidgetClass','GRZVoteRoot',0,0,1,1,'color 0 0 0 0',widget('FrameWidgetClass','Panel',.06,.075,.88,.85,'color 0 0 0 0',children))
(source/'gui/layouts/vote.layout').write_text(layout)
notice=widget('FrameWidgetClass','GRZMapNotice',.12,.08,.76,.065,'visible 0\n ignorepointer 1\n color 0 0 0 0',panel('NoticeBackground',0,0,1,1,'0 0 0 0.85')+text('Message','',.01,.08,.98,.84,24))
(source/'gui/layouts/notice.layout').write_text(notice)
config='''class CfgPatches { class GRZ_MapRotation { units[] = {}; weapons[] = {}; requiredVersion = 0.1; requiredAddons[] = {"DZ_Data","DZ_Scripts"}; }; };
class CfgMods { class GRZ_MapRotation { name = "GRZ Map Rotation"; author = "Blake"; type = "mod"; inputs = "GRZ_MapRotation/inputs.xml"; dependencies[] = {"Game","World","Mission"}; class defs { class gameScriptModule {value = ""; files[] = {"GRZ_MapRotation/scripts/3_Game"};}; class worldScriptModule {value = ""; files[] = {"GRZ_MapRotation/scripts/4_World"};}; class missionScriptModule {value = ""; files[] = {"GRZ_MapRotation/scripts/5_Mission"};};};};};
class CfgSoundShaders {
'''
for name in ['vote','warning','change']:config+=f'class GRZ_Rotation_{name}_Shader {{ samples[] = {{{{"GRZ_MapRotation\\sounds\\{name}",1}}}}; volume = 0.7; range = 10; rangeCurve[] = {{{{0,1}},{{10,1}}}}; }};\n'
config+='};\nclass CfgSoundSets {\n'
for name in ['vote','warning','change']:config+=f'class GRZ_Rotation_{name.capitalize()}_SoundSet {{ soundShaders[] = {{"GRZ_Rotation_{name}_Shader"}}; volumeFactor = 1; frequencyRandomizer = 0; spatial = 0; doppler = 0; loop = 0; }};\n'
config+='};\n';(source/'config.cpp').write_text(config)
pbo=build(source);(out/'GRZ_MapRotation.pbo').write_bytes(pbo)
(root/'GRZ-MAP-ROTATION-MANIFEST.json').write_text(json.dumps(dict(version='0.1.1',arenas=report,round_seconds=1800,vote_opens_remaining=120,vote_locks_remaining=30,popup_seconds=30,outside_seconds=5,pbo_sha256=hashlib.sha256(pbo).hexdigest(),map_objects=len(objects),map_hidden_objects=len(dze['EditorHiddenObjects']),map_author_metadata_removed=True,original_map_sha256=hashlib.sha256(args.dze.read_bytes()).hexdigest(),sanitized_map_sha256=hashlib.sha256(sanitized).hexdigest()),indent=2)+'\n')
print('Built GRZ_MapRotation.pbo',len(pbo),'bytes; three placement-derived blueprint cards and sanitized DZE.')

if (root/'MAP-ROTATION.md').exists():
    with ZipFile(out/'GRZ-MapRotation.zip','w',ZIP_DEFLATED) as archive:
        archive.writestr('GRZ_MapRotation.pbo',pbo)
        archive.writestr('EditorFiles/GRZ_BBC_DM_2.dze',sanitized)
        for filename in ['Install-GRZMapRotation.cmd','Install-GRZMapRotation.ps1']:
            archive.write(root/'packaging/map-rotation'/filename,filename)
        archive.write(root/'GRZ-MAP-ROTATION-MANIFEST.json','MANIFEST.json')
        archive.write(root/'MAP-ROTATION.md','INSTALL.txt')
        for preview in blueprints.glob('*-blueprint.png'):
            archive.write(preview,'Blueprints/'+preview.name)
    print('Built installable GRZ-MapRotation.zip.')
