"""Pack the standalone ADS recoil modifier without firing or damage hooks."""
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
    source = ROOT / 'GRZ_LowRecoil'
    script = (source / 'scripts/4_World/grzlowrecoil.c').read_text()
    if args.reference_scripts:
        native = (args.reference_scripts / '4_world/classes/recoilbase/recoilbase.c').read_text()
        assert signature(script, 'GetRecoilModifier') == signature(native, 'GetRecoilModifier')
        assert 'GetRecoilModifier(Weapon_Base weapon)' in script
        assert 'm_RecoilModifier = GetRecoilModifier( GetWeapon() );' in native
        assert 'm_CamOffsetDistance *= m_RecoilModifier[2];' in native
        print('Native recoil signature, parameter and per-shot camera modifier use checked.')
    assert script.count('{') == script.count('}')
    data = build(source)
    assert data[-21] == 0 and hashlib.sha1(data[:-21]).digest() == data[-20:]
    filename = 'GRZ_LowRecoil.pbo'
    (ROOT / 'downloads' / filename).write_bytes(data)
    base = ROOT / 'packaging/local-server'
    (base / '@GRZ_Local/Addons' / filename).write_bytes(data)
    path = base / 'PBO-manifest.json'
    manifest = json.loads(path.read_text())
    manifest[filename] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    path.write_text(json.dumps(manifest, indent=2) + '\n')
    (base / 'GRZ-LowRecoil.md').write_bytes((ROOT / 'LOW-RECOIL.md').read_bytes())
    print(f'Built {filename}: {len(data)} bytes. Enforce compilation/gameplay untested.')


if __name__ == '__main__':
    main()
