"""Package arena cleanup and its compatible death-screen update."""
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build
from repair_hexgrid import read_pbo

root = Path(__file__).resolve().parents[1]
notes = '''GRZ Arena Cleanup

Stop DayZ and the server. Copy BOTH included PBOs into @GRZ_Local/Addons
in the DayZ and DayZServer folders, replacing GRZ_DeathScreen.pbo.
Keep GRZ_MapRotation installed for active-arena respawns.
Restart both with @GRZ_Local enabled.

Normal inventory drop, drag to ground and throw actions are blocked.
Inventory transfers and server loadout/clothing transactions remain available.
Dead PlayerBase characters (including survivor bots) and their contents are
removed 250 milliseconds after death, after damage/killfeed callbacks finish.
A detached held weapon is removed unless another living player acquired it.
Existing ground loot, zombies, animals and placed arena objects are unchanged.
The updated death screen keeps RESPAWN available after corpse removal.

Check on your test server: inventory drop/drag/throw; killfeed and hit sounds;
body and held weapon cleanup; RESPAWN into the active arena; loadout and
clothing saves with cargo. Package/API checks only: not compiled or tested
in DayZ. Unsigned PBOs; sign with your server key if signatures are enforced.
'''
(root / 'CLEAN-ARENA.md').write_text(notes)
manifest = {}
with ZipFile(root / 'downloads/GRZ-CleanArena.zip', 'w', ZIP_DEFLATED) as z:
    for addon in ('GRZ_CleanArena', 'GRZ_DeathScreen'):
        payload = build(root / addon)
        extensions, files = read_pbo(payload)
        assert extensions['prefix'] == addon
        for path in (root / addon).rglob('*.c'):
            packed = path.relative_to(root / addon).as_posix()
            assert files[packed] == path.read_bytes()
        (root / 'downloads' / (addon + '.pbo')).write_bytes(payload)
        z.writestr(addon + '.pbo', payload)
        manifest[addon] = dict(bytes=len(payload), sha256=hashlib.sha256(payload).hexdigest())
    z.writestr('INSTALL.txt', notes)
(root / 'downloads/GRZ-CLEAN-ARENA-MANIFEST.json').write_text(json.dumps(manifest, indent=2)+'\n')
print(json.dumps(manifest, indent=2))
