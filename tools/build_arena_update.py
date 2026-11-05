from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json
from build_pbo import build
r=Path(__file__).resolve().parents[1]
m={'pbo':{}}
with ZipFile(r/'downloads/GRZ-Arena-Menu-Update.zip','w',ZIP_DEFLATED) as z:
 for addon in ['GRZ_Loadout','GRZ_MapRotation','GRZ_DeathScreen','GRZ_CustomHud']:
  data=build(r/addon)
  assert b'76561199208787139' not in data
  m['pbo'][addon]={'sha256':hashlib.sha256(data).hexdigest()}
  z.writestr(addon+'.pbo',data)
 z.writestr('MANIFEST.json',json.dumps(m,indent=2)+'\n')
 z.write(r/'packaging/arena-update/Install-Update.ps1','Install-Update.ps1')
 z.writestr('INSTALL.txt','Close DayZ and server. Run Install-Update.ps1 -SteamId YOUR_17_DIGIT_ID in PowerShell. The installer preserves existing arena configuration and adds this ID to AdminSteamIds. Requires existing GRZ_MapRotation/config.json.\n\nAdmin !skip warns players at the top, then switches to the next arena after 30 seconds. Duel players remain in their match. Loadout adds BAG and locks gameplay inputs in menus. Death screen waits 2 seconds, fades over 0.8 seconds, and hides gameplay HUD.\nStatic/package checks only; DayZ runtime untested.\n')
print('Built GRZ-Arena-Menu-Update.zip')
