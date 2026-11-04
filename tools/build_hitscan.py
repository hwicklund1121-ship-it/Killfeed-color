"""Pack the hitscan test update; optional native signature checks are not compilation."""
import argparse
import hashlib
import json
import re
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]


def signature(text, method):
    match = re.search(r'\b(?:void|bool|float|vector)\s+' + method + r'\s*\(([^)]*)\)', text)
    assert match, method
    return [p.strip().split()[:-1] for p in match[1].split(',')]


def check_reference(reference):
    pairs = [
        ('GRZ_Hitscan/scripts/4_World/grzhitscantargets.c', '3_game/entities/object.c', 'EEOnDamageCalculated'),
        ('GRZ_Hitscan/scripts/4_World/grzhitscan.c', '4_world/entities/firearms/weapon_base.c', 'EEFired'),
        ('GRZ_NoSway/scripts/4_World/grznosway.c', '4_world/entities/dayzplayerimplementaiming.c', 'ApplyBreathingPattern'),
        ('GRZ_NoSway/scripts/4_World/grznosway.c', '4_world/entities/dayzplayerimplementaiming.c', 'ApplyHorizontalNoise'),
    ]
    for own, native, method in pairs:
        assert signature((ROOT / own).read_text(), method) == signature((reference / native).read_text(), method), method
    # Confirm that the native exception specifically wrapped in this addon bypasses super.
    plastic = (reference / '4_world/entities/explosivesbase/plastic_explosive.c').read_text()
    callback = plastic.split('override bool EEOnDamageCalculated', 1)[1].split('override void', 1)[0]
    assert 'return true;' in callback and 'super.EEOnDamageCalculated' not in callback
    print('Native event/sway signatures match; vanilla plastic-explosive bypass confirmed.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-scripts', type=Path)
    args = parser.parse_args()
    for script in (ROOT / 'GRZ_Hitscan/scripts').rglob('*.c'):
        assert not re.search(r'modded\s+class\s+Object\b', script.read_text()), 'Engine Object cannot be modded'
    if args.reference_scripts:
        check_reference(args.reference_scripts)
    output = ROOT / 'downloads'
    base = ROOT / 'packaging/local-server'
    manifest_path = base / 'PBO-manifest.json'
    manifest = json.loads(manifest_path.read_text())
    payloads = {}
    for name in ['GRZ_Hitscan', 'GRZ_NoSway', 'GRZ_Killfeed']:
        payload = build(ROOT / name)
        assert payload[-21] == 0 and hashlib.sha1(payload[:-21]).digest() == payload[-20:]
        assert b'prefix\0' + name.encode() + b'\0\0' in payload[:100]
        filename = name + '.pbo'
        (output / filename).write_bytes(payload)
        (base / '@GRZ_Local/Addons' / filename).write_bytes(payload)
        manifest[filename] = {'bytes': len(payload), 'sha256': hashlib.sha256(payload).hexdigest()}
        payloads['@GRZ_Local/Addons/' + filename] = payload
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    notes = (ROOT / 'HITSCAN.md').read_bytes()
    (base / 'GRZ-Hitscan-TEST.md').write_bytes(notes)
    payloads['GRZ-Hitscan-TEST.md'] = notes
    target = output / 'GRZ-hitscan-nosway-killfeed-TEST.zip'
    with ZipFile(target, 'w', ZIP_DEFLATED) as archive:
        for name, payload in payloads.items():
            archive.writestr(name, payload)
    with ZipFile(target) as archive:
        assert archive.testzip() is None
        assert set(archive.namelist()) == set(payloads)
        for name, payload in payloads.items():
            assert archive.read(name) == payload
    print(f'Verified {target.name}: {target.stat().st_size} bytes. DayZ compilation/gameplay NOT tested.')


if __name__ == '__main__':
    main()
