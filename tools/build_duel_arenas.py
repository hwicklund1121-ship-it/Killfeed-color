"""Package the configured five-arena DZE, runtime and transactional Windows installer."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,re
from build_pbo import build
from inspect_grz_dze import read
r=Path(__file__).resolve().parents[1];p=r/'packaging/duels'
old=json.loads((r/'GRZ-MAP-ROTATION-MANIFEST.json').read_text())
m={'maps_configured':True,'maps':[a['Name'] for a in json.loads((p/'duel-config.json').read_text())['Maps']],'pbo':{},'files':{},'previous_map_hashes':[old['original_map_sha256'],old['sanitized_map_sha256']]}
with ZipFile(r/'downloads/GRZ-Duels-Arenas-Update.zip','w',ZIP_DEFLATED) as z:
 for addon in ['GRZ_Loadout','GRZ_MapRotation','GRZ_DeathScreen','GRZ_CustomHud']:
  data=build(r/addon);m['pbo'][addon]={'sha256':hashlib.sha256(data).hexdigest()};z.writestr(addon+'.pbo',data)
 for path in [p/'duel-config.json',p/'EditorFiles/GRZ_BBC_DM_3.dze']:
  data=path.read_bytes();m['files'][path.name]=hashlib.sha256(data).hexdigest();z.writestr(path.name,data)
  assert not re.search(rb'7656119\d{10}',data),'Private Steam ID in payload'
 z.write(p/'Install-Duels.ps1','Install-Duels.ps1')
 z.writestr('MANIFEST.json',json.dumps(m,indent=2)+'\n')
 z.writestr('INSTALL.txt','Close DayZ and the server. Run Install-Duels.ps1 -SteamId YOUR_17_DIGIT_STEAM_ID from this folder. Default mission: dayzOffline.chernarusplus. Optional -ServerPath -GamePath -ProfilePath -MissionName.\n\nInstalls 100 Meters, Shipment, Long Range 1v1, Cargo Containers, Castle 1v1. Terrain resolves spawn height; sides face each other. Backs up and replaces the duel map config and supplied DZE, removes the known older copy from EditorFiles to avoid duplicates, preserves ratings/saved loadouts and other DZE files. Copies four paired PBOs to game and server. Requires existing map rotation config and DayZ Editor Loader with all DZE object dependencies.\n\nOpen loadout with comma, choose DUELS. Both players hold a supported gun; choose player/map/best-of and challenge. No live DayZ runtime is available here; verify both spawn sides in game before opening public duels.\n')
 z.write(p/'duel-arenas-preview.png','duel-arenas-preview.png')
print('Built GRZ-Duels-Arenas-Update.zip with five enabled maps.')
