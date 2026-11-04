"""Pack the custom quickscope test addon and check exposed callback signatures."""
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
    source = ROOT / 'GRZ_Quickscope'
    script = (source / 'scripts/4_World/grzquickscope.c').read_text()
    if args.reference_scripts:
        for method, filename in [
            ('OnCameraChange', '4_world/entities/manbase/dayzplayer/dayzplayercameras.c'),
            ('HandleADS', '4_world/entities/dayzplayerimplement.c'),
            ('OnUpdate', '4_world/entities/manbase/dayzplayer/dayzplayercameraironsights.c'),
            ('StdFovUpdate', '4_world/entities/manbase/dayzplayer/dayzplayercamera_base.c'),
            ('GetRecoilModifier', '4_world/classes/recoilbase/recoilbase.c'),
        ]:
            assert signature(script, method) == signature((args.reference_scripts / filename).read_text(), method), method
        weapon = (args.reference_scripts / '4_world/entities/firearms/weapon_base.c').read_text()
        assert 'hcw.GetBaseAimingAngleLR() + player.GetOrientation()[0]' in weapon
        assert 'hcw.GetAimingHandsOffsetLR()' in weapon and 'hcw.GetAimingHandsOffsetUD()' in weapon
        print('Quickscope callback signatures and native-script aiming-angle construction checked.')
    parent = (ROOT / 'GRZ_Hitscan/scripts/4_World/grzhitscan.c').read_text()
    assert signature(script, 'GRZAdjustHitscanDirection') == signature(parent, 'GRZAdjustHitscanDirection')
    assert script.count('{') == script.count('}')
    data = build(source)
    assert data[-21] == 0 and hashlib.sha1(data[:-21]).digest() == data[-20:]
    filename = 'GRZ_Quickscope.pbo'
    (ROOT / 'downloads' / filename).write_bytes(data)
    base = ROOT / 'packaging/local-server'
    (base / '@GRZ_Local/Addons' / filename).write_bytes(data)
    manifest_path = base / 'PBO-manifest.json'
    manifest = json.loads(manifest_path.read_text())
    manifest[filename] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    (base / 'GRZ-Quickscope-TEST.md').write_bytes((ROOT / 'QUICKSCOPE.md').read_bytes())
    print(f'Built {filename}: {len(data)} bytes. Native Enforce compilation/gameplay untested.')


if __name__ == '__main__':
    main()
