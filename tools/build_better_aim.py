"""Pack the small native player-type aiming setup; no camera or damage overrides."""
import argparse
import hashlib
import json
from pathlib import Path

from build_hitscan import signature
from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-scripts', type=Path)
    args = parser.parse_args()
    source = ROOT / 'GRZ_BetterAim'
    script = (source / 'scripts/4_World/grzbetteraimsetup.c').read_text()
    if args.reference_scripts:
        native = (args.reference_scripts / '4_world/entities/manbase/dayzplayer/dayzplayercfgbase.c').read_text()
        assert signature(script, 'DayZPlayerTypeCreate') == signature(native, 'DayZPlayerTypeCreate')
        assert 'void SetCameraShootParams(float pRayDistance, float pRayRadius, float pMaxAngleCos)' in (args.reference_scripts / '3_game/dayzplayer.c').read_text()
        print('Native player-type creation and SetCameraShootParams signatures checked.')
    assert script.count('{') == script.count('}')
    data = build(source)
    assert data[-21] == 0 and hashlib.sha1(data[:-21]).digest() == data[-20:]
    filename = 'GRZ_BetterAim.pbo'
    (ROOT / 'downloads' / filename).write_bytes(data)
    base = ROOT / 'packaging/local-server'
    (base / '@GRZ_Local/Addons' / filename).write_bytes(data)
    path = base / 'PBO-manifest.json'
    manifest = json.loads(path.read_text())
    manifest[filename] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    path.write_text(json.dumps(manifest, indent=2) + '\n')
    (base / 'GRZ-BetterAim.md').write_bytes((ROOT / 'BETTER-AIM.md').read_bytes())
    print(f'Built {filename}: {len(data)} bytes. Enforce compilation/gameplay untested.')


if __name__ == '__main__':
    main()
