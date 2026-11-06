from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,re
from build_pbo import build
r=Path(__file__).resolve().parents[1]
old=json.loads((r/'GRZ-MAP-ROTATION-MANIFEST.json').read_text())
m={'pbo':{},'files':{},'previous_map_hashes':[old['original_map_sha256'],old['sanitized_map_sha256']]+json.loads((r/'packaging/duels/previous-map-hashes.json').read_text())}
with ZipFile(r/'downloads/GRZ-Arena-Menu-Update.zip','w',ZIP_DEFLATED) as z:
 for addon in ['GRZ_Loadout','GRZ_MapRotation','GRZ_DeathScreen','GRZ_CustomHud','GRZ_SFGPermanent','GRZ_AR50Sounds']:
  data=build(r/addon)
  assert not re.search(rb'7656119\d{10}', data)
  m['pbo'][addon]={'sha256':hashlib.sha256(data).hexdigest()}
  z.writestr(addon+'.pbo',data)
 data=(r/'packaging/duels/EditorFiles/GRZ_BBC_DM_3.dze').read_bytes()
 m['files']['GRZ_BBC_DM_3.dze']=hashlib.sha256(data).hexdigest()
 z.writestr('GRZ_BBC_DM_3.dze',data)
 z.writestr('MANIFEST.json',json.dumps(m,indent=2)+'\n')
 z.write(r/'packaging/arena-update/Install-Update.ps1','Install-Update.ps1')
 z.writestr('INSTALL.txt','Close DayZ and server. Run Install-Update.ps1 -SteamId YOUR_17_DIGIT_ID in PowerShell. The installer preserves existing arena configuration and adds this ID to AdminSteamIds. Requires existing GRZ_MapRotation/config.json and mission folder. Also installs the updated GRZ_BBC_DM_3.dze with replacement airfield walls. Backs up known old map copies outside EditorFiles to avoid duplicates; preserves current duel spawn configuration and ratings.\n\nClothing Save and Apply now uses authoritative networked inventory moves, so worn clothes and transferred contents update without respawning. Saved character appearance is still used at respawn. AR50 audio: uploaded UAV_Ak50 shot with dedicated close/mid/far layers, installed through GRZ_AR50Sounds.pbo on both sides. No UAV repack required. Siphon: killing another player or survivor bot fully restores the living killer health, blood, shock and body damage, and clears bleeding. No reward for suicide. Gear is unchanged. Includes GRZ_SFGPermanent.pbo on BOTH sides: SFG uses a native weaponMuzzleM4 proxy and matching hidden, locked suppressor attachment for held-weapon rendering. Suppressed firing sounds remain built into its firing mode. VPP/loadout SFGs get a hunting optic, and the loadout preview uses the integrated model. Requires the original sfg50 weapon addon. Admin !skip warns players at the top, then switches to the next arena after 30 seconds. Duel players remain in their match. Loadout adds BAG, locks gameplay inputs in menus, and supports left-click dragging the clothing character through 360 degrees. The viewing angle stays when changing clothes. Death screen waits 2 seconds, fades over 0.8 seconds, and hides gameplay HUD. Right-side RESPAWN opens SPAWN CUSTOM (saved character) or SPAWN RANDOM (random character). Both restore the same saved clothes and weapon and use the existing active-arena spawn system.\nStatic/package checks only; DayZ runtime untested.\n')
(r/'downloads/GRZ-SFG-Native-Muzzle-Update.zip').write_bytes((r/'downloads/GRZ-Arena-Menu-Update.zip').read_bytes())
print('Built GRZ-Arena-Menu-Update.zip and GRZ-SFG-Native-Muzzle-Update.zip')
