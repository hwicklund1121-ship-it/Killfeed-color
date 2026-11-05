"""Convert supplied death artwork and build the standalone addon."""
import io
import sys
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from PIL import Image
from build_status_textures import encode_paa
from build_pbo import build
root = Path(__file__).resolve().parents[1]
source = root / 'GRZ_DeathScreen'
with ZipFile(sys.argv[1]) as z:
    names = [n for n in z.namelist() if n.lower().endswith('.png')]
    if len(names) != 1:
        raise SystemExit('Expected one PNG')
    original = z.read(names[0])
    image = Image.open(io.BytesIO(original)).convert('RGBA')
(source / 'data/death.paa').write_bytes(encode_paa(image.resize((2048,1024), Image.Resampling.LANCZOS), 'DXT1'))
pbo = build(source)
(root / 'downloads/GRZ_DeathScreen.pbo').write_bytes(pbo)
notes = '''GRZ Death Screen 0.2.0 — Made By Blake

Copy GRZ_DeathScreen.pbo into @GRZ_Local/Addons on both game and server.
Restart both and enable @GRZ_Local in the client launcher.
Uses the supplied artwork; no new text is added.
Shows the death camera for 2 seconds, then fades the artwork in over 0.8 seconds.
Repeated death/cleanup notifications do not restart the transition.
The respawn button and mouse focus activate when the fade finishes.
The gameplay HUD root is hidden throughout death/respawn, then its prior visibility is restored.
Death artwork has a visible RESPAWN button using the native InGameMenu.GameRespawn(false) flow.
Mouse cursor/focus is available while dead; ESC/native menus remain accessible.
Server respawn hooks determine the location: GRZ Map Rotation places the player in the active arena.
Keep GRZ_MapRotation.pbo installed on both sides; clothing/outfit hooks remain in the native spawn chain.
Artwork hides while ESC/respawn/confirmation menus are open so controls remain accessible.
Artwork clears on respawn and mission exit. Unconsciousness is unchanged.
Unsigned development addon. Sign with your own key if required by your server.
Package and native script signatures checked; not compiled or tested in DayZ.
'''
(root / 'DEATH-SCREEN.md').write_text(notes)
with ZipFile(root / 'downloads/GRZ_DeathScreen.zip', 'w', ZIP_DEFLATED) as z:
    z.writestr('GRZ_DeathScreen.pbo', pbo)
    z.writestr('INSTALL.txt', notes)
(root / 'downloads/GRZ-DEATH-SCREEN-MANIFEST.json').write_text(json.dumps(dict(original_size=image.size, source_sha256=hashlib.sha256(original).hexdigest(), pbo_sha256=hashlib.sha256(pbo).hexdigest()),indent=2)+'\n')
print('Built death-screen PBO:',len(pbo),'bytes')
