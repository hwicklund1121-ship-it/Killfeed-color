"""Package the paired duels/leaderboard update and its disabled arena template."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,subprocess,sys
from build_pbo import build
root=Path(__file__).resolve().parents[1]
out=root/'downloads'
manifest={'loadout_version':'0.4.2','rotation_version':'0.1.3','starting_elo':800,'leaderboard_key':'O','challenge_seconds':60,'countdown_seconds':5,'best_of':[1,3,5],'maps_configured':False,'pbo':{}}
for addon in ['GRZ_Loadout','GRZ_MapRotation']:
 data=build(root/addon)
 (out/(addon+'.pbo')).write_bytes(data)
 manifest['pbo'][addon]={'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
notes=(root/'DUELS.md').read_text()
for filename in ['GRZ-Duels-Update.zip','GRZ_Loadout.zip']:
 with ZipFile(out/filename,'w',ZIP_DEFLATED) as z:
  for addon in manifest['pbo']:
   z.write(out/(addon+'.pbo'),addon+'.pbo')
  z.writestr('INSTALL.txt',notes)
  z.write(root/'packaging/duels/duel-config.example.json','duel-config.example.json')
  z.writestr('MANIFEST.json',json.dumps(manifest,indent=2)+'\n')
(out/'GRZ-DUELS-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Built GRZ-Duels-Update.zip and the paired legacy GRZ_Loadout.zip. Arena template is disabled.')
