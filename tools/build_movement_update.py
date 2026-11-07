"""Build the focused updater for the existing GRZ deathmatch movement addon."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json
from build_pbo import build
r=Path(__file__).resolve().parents[1]
files={'GRZ_Movement.pbo':build(r/'GRZ_Movement'),'InventoryBuilder.cs':(r/'packaging/inventory-builder/InventoryBuilder.cs').read_bytes(),'Install-Movement.ps1':(r/'packaging/movement-update/Install-Movement.ps1').read_bytes()}
files['MANIFEST.json']=json.dumps({n:hashlib.sha256(b).hexdigest() for n,b in files.items() if n!='Install-Movement.ps1'},indent=2).encode()
files['INSTALL.txt']=b'Close DayZ and stop the server. Extract this ZIP, open PowerShell here, run:\r\npowershell -ExecutionPolicy Bypass -File .\\Install-Movement.ps1\r\nUpdates existing @GRZ_Local movement on both sides with backups; native game animation header paths are generated locally. No clothing/loadout/profile updates. Gait request 1.35 requires an in-game speed check. HK/prone drop is not part of this update.\r\n'
with ZipFile(r/'downloads/GRZ-Movement-Update.zip','w',ZIP_DEFLATED) as z:
 for n,b in files.items():z.writestr(n,b)
print('Built GRZ-Movement-Update.zip')
