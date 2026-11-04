"""Build the requested standalone no-fracture/no-walking-foot-cuts addon."""
import hashlib
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

from build_pbo import build
from repair_hexgrid import read_pbo

ROOT = Path(__file__).resolve().parents[1]
data = build(ROOT / 'GRZ_NoLegBreaks')
prefix, files = read_pbo(data)
assert prefix['prefix'] == 'GRZ_NoLegBreaks'
assert hashlib.sha1(data[:-21]).digest() == data[-20:]
for path, content in files.items():
    assert content == (ROOT / 'GRZ_NoLegBreaks' / path).read_bytes()
source = files['scripts/4_World/GRZNoLegBreaks.c'].decode()
assert 'super.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS)' in source
assert 'override void ProcessFeetDamageServer(int pUserInt)' in source
assert 'override void OnReconnect(PlayerBase player)' in source
assert not any(token in source for token in ['EEKilled(', 'EEHitBy(', 'EEDamageCalculated(', 'SetHealth(', 'AddHealth('])
output = ROOT / 'downloads'
(output / 'GRZ_NoLegBreaks.pbo').write_bytes(data)
with ZipFile(output / 'GRZ-NoLegBreaks.zip', 'w', ZIP_DEFLATED) as archive:
    archive.writestr('@GRZ_Local/Addons/GRZ_NoLegBreaks.pbo', data)
    archive.write(ROOT / 'NO-LEG-BREAKS.md', 'INSTALL.md')
with ZipFile(output / 'GRZ-NoLegBreaks.zip') as archive:
    assert archive.testzip() is None
    assert archive.read('@GRZ_Local/Addons/GRZ_NoLegBreaks.pbo') == data
print('Standalone PBO and ZIP verified; no combat-health/death callbacks overridden. DayZ runtime testing still needed.')
