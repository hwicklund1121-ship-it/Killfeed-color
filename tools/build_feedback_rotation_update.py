"""Package hitmarker, spawn enforcement and delayed cleanup updates."""
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build
from repair_hexgrid import read_pbo
root = Path(__file__).resolve().parents[1]
out = root / 'downloads'
payload = build(root / 'GRZ_MapRotation')
(out / 'GRZ_MapRotation.pbo').write_bytes(payload)
p = root / 'GRZ-MAP-ROTATION-MANIFEST.json'
manifest = json.loads(p.read_text())
manifest['pbo_sha256'] = hashlib.sha256(payload).hexdigest()
p.write_text(json.dumps(manifest, indent=2)+'\n')
archive = out / 'GRZ-MapRotation.zip'
with ZipFile(archive) as z:
    entries = {name:z.read(name) for name in z.namelist()}
entries['GRZ_MapRotation.pbo'] = payload
entries['MANIFEST.json'] = p.read_bytes()
entries['INSTALL.txt'] = (root / 'MAP-ROTATION.md').read_bytes()
with ZipFile(archive, 'w', ZIP_DEFLATED) as z:
    for name,data in entries.items():
        z.writestr(name,data)
notes = '''Close DayZ and the server. Copy all FIVE PBOs to @GRZ_Local/Addons
in BOTH DayZ and DayZServer, replacing the existing versions. Restart both.
Do not replace or delete your profile config or DZE.

GRZ_MapRotation: fresh respawns now register for arena placement and the
five-second outside-boundary countdown. Existing ready callbacks are deduplicated.
GRZ_Loadout 0.2.5: Save a weapon once after installing. Fresh respawns restore
that gun/wrap, compatible scope, ten ammo stacks and six spare magazines for
magazine-fed guns. Clothing is restored first. Choice survives server restarts;
reconnecting does not duplicate equipment. Preview-only choices are not saved.
GRZ_CleanArena: bodies and gear are removed after THREE seconds; drops blocked.
GRZ_DeathScreen: RESPAWN remains available after corpse deletion.
GRZ_Hitmarker 0.1.7: listener-positioned playback, flat long-range shaders,
unique sound-set names and invalid-sound-set logging. Supplied Rust headshot
clip retained. Sound ON and volume above zero in Options > Game > GRZ HITMARKER.

Static/package checks passed. DayZ compilation and gameplay untested.
If sound is still silent, send the CLIENT script log and RPT after one body
hit and one headshot; look for [GRZ Hitmarker 0.1.7] messages.
'''
with ZipFile(out / 'GRZ-Arena-Feedback-Fix.zip', 'w', ZIP_DEFLATED) as z:
    for addon in ('GRZ_MapRotation','GRZ_Hitmarker','GRZ_CleanArena','GRZ_DeathScreen','GRZ_Loadout'):
        data = (out / (addon+'.pbo')).read_bytes()
        extensions,files=read_pbo(data)
        assert extensions['prefix']==addon
        for path in (root / addon).rglob('*.c'):
            assert files[path.relative_to(root / addon).as_posix()]==path.read_bytes()
        z.writestr(addon+'.pbo',data)
    z.writestr('INSTALL.txt',notes)
with ZipFile(out / 'GRZ-Arena-Feedback-Fix.zip') as z:
    assert z.testzip() is None
print('Verified all five PBOs match source; built GRZ-Arena-Feedback-Fix.zip.')
