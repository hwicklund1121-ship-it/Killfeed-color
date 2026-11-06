"""Make a top-down planning preview from the sanitized DZE and configured sides."""
from pathlib import Path
import json,math
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from inspect_grz_dze import read
r=Path(__file__).resolve().parents[1];p=r/'packaging/duels'
data,_=read(p/'EditorFiles/GRZ_BBC_DM_3.dze');cfg=json.loads((p/'duel-config.json').read_text())
fig,axes=plt.subplots(2,3,figsize=(17,10),facecolor='#0b0b0b')
for ax,m in zip(axes.flat,cfg['Maps']):
 ax.set_facecolor('#0b0b0b');cx,_,cz=m['Center']
 for o in data['EditorObjects']:
  x,y,z=o['Position']
  if math.hypot(x-cx,z-cz)>m['Radius']+45:continue
  if m['Id']=='long_range_1v1' and z<10540:continue
  t=o['Type'].lower();angle=math.radians(o['Orientation'][0])
  if 'water_pond_50x50m' in t:
   side=50*o['Scale']/2;points=[]
   for u,v in [(-side,-side),(side,-side),(side,side),(-side,side)]:points.append((x+u*math.cos(angle)+v*math.sin(angle),z-u*math.sin(angle)+v*math.cos(angle)))
   ax.add_patch(Polygon(points,edgecolor='white',facecolor='#222222',hatch='...',linewidth=.8));continue
  length=12
  if 'wall1_20' in t:length=20
  elif 'wall_ent' in t:length=17
  elif 'rock' in t:length=0
  if length:
   dx=math.cos(angle)*length/2;dz=-math.sin(angle)*length/2;ax.plot([x-dx,x+dx],[z-dz,z+dz],color='white',lw=.8,alpha=.7)
  else:ax.scatter(x,z,s=5,color='#aaaaaa')
 a=m['SpawnA'];b=m['SpawnB'];distance=math.hypot(a[0]-b[0],a[2]-b[2])
 for pt,label,other in [(a,'A',b),(b,'B',a)]:
  ax.scatter(pt[0],pt[2],s=95,facecolor='black',edgecolor='white',zorder=5)
  ax.annotate(label,(pt[0],pt[2]),ha='center',va='center',color='white',fontsize=9,zorder=6)
  ax.arrow(pt[0],pt[2],(other[0]-pt[0])/distance*8,(other[2]-pt[2])/distance*8,color='white',head_width=2,length_includes_head=True)
 ax.set_title(m['Name']+f' | {distance:.0f} m between spawns',color='white');ax.set_aspect('equal');ax.tick_params(colors='#888888',labelsize=7);ax.grid(alpha=.15,color='white')
 for spine in ax.spines.values():spine.set_edgecolor('#555555')
axes.flat[5].axis('off');axes.flat[5].text(.05,.65,'GRZ DUEL ARENAS\n\nA / B = opposing spawn positions\nHatched square = pond\n\nTop-down planning preview\nObject outlines are approximate\nLong-range sides use platform heights',color='white',fontsize=13,transform=axes.flat[5].transAxes)
fig.tight_layout(pad=3);fig.savefig(p/'duel-arenas-preview.png',dpi=140,facecolor=fig.get_facecolor())
print('Built updated duel planning preview.')
