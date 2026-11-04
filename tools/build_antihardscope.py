"""Build the scope-blackout PBO and a merge-only ZIP of the recent updates."""
import argparse
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-scripts', type=Path)
    args = parser.parse_args()
    source = ROOT / 'GRZ_AntiHardscope'
    script = (source / 'scripts/5_Mission/grzantihardscope.c').read_text()
    if args.reference_scripts:
        native_player = (args.reference_scripts / '4_world/entities/dayzplayerimplement.c').read_text()
        native_mission = (args.reference_scripts / '5_mission/mission/missiongameplay.c').read_text()
        assert 'bool IsInOptics()' in native_player
        for callback in ['void OnUpdate(float timeslice)', 'void OnMissionFinish()']:
            assert callback in native_mission and callback in script
        assert 'override bool IsUnconscious()' in (args.reference_scripts / '4_world/entities/manbase/playerbase.c').read_text()
        print('Scope state, unconsciousness and mission lifecycle APIs found in native script reference.')
    layout = (source / 'gui/layouts/blackout.layout').read_text()
    assert layout.count('{') == layout.count('}')
    assert 'PanelWidgetClass GRZScopeBlackout' in layout and 'ImageWidgetClass GRZScopeLogo' in layout
    root = layout.split('ImageWidgetClass', 1)[0]
    assert 'position 0 0' in root and 'size 1 1' in root and 'color 0 0 0 1' in root
    assert (source / 'gui/textures/grz_scope_logo_ca.paa').read_bytes()[:2] == b'\x05\xff'
    assert 'hexactsize 0' in layout and 'vexactsize 0' in layout
    assert 'visible 0' in layout and 'ignorepointer 1' in layout
    assert script.count('{') == script.count('}')
    output = ROOT / 'downloads'
    base = ROOT / 'packaging/local-server'
    filename = 'GRZ_AntiHardscope.pbo'
    data = build(source)
    assert data[-21] == 0 and hashlib.sha1(data[:-21]).digest() == data[-20:]
    (output / filename).write_bytes(data)
    (base / '@GRZ_Local/Addons' / filename).write_bytes(data)
    manifest_path = base / 'PBO-manifest.json'
    manifest = json.loads(manifest_path.read_text())
    manifest[filename] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    (base / 'GRZ-AntiHardscope.md').write_bytes((ROOT / 'ANTI-HARDSCOPE.md').read_bytes())
    payloads = {}
    for name in ['GRZ_AntiHardscope', 'GRZ_Hitscan', 'GRZ_NoSway', 'GRZ_Killfeed', 'GRZ_AimControl', 'GRZ_SkyColors']:
        payload = (output / (name + '.pbo')).read_bytes()
        # Fail if an older downloadable addon is left over after a source change.
        assert payload == build(ROOT / name), name
        payloads['@GRZ_Local/Addons/' + name + '.pbo'] = payload
    payloads['GRZ-AntiHardscope.md'] = (ROOT / 'ANTI-HARDSCOPE.md').read_bytes()
    payloads['GRZ-Hitscan-TEST.md'] = (ROOT / 'HITSCAN.md').read_bytes()
    payloads['GRZ-AimControl-TEST.md'] = (ROOT / 'AIM-CONTROL.md').read_bytes()
    payloads['GRZ-SkyColors.md'] = (ROOT / 'SKY-COLORS.md').read_bytes()
    payloads['READ-ME-FIRST.txt'] = (
        'Stop DayZ and the server. Merge @GRZ_Local into BOTH the DayZ game folder and installed server folder.\n'
        'DELETE the old GRZ_Quickscope.pbo from BOTH installed Addons folders; merging this ZIP does not remove old files.\n'
        'Replace the matching PBOs and add GRZ_AimControl.pbo. Keep every other addon, especially UAVWEAPONS.pbo and your locally generated GRZ_InventorySizes.pbo.\n'
        'Restart both. No extra launch argument is needed when @GRZ_Local is already loaded.\n\n'
        'Anti hardscope: after 0.75 seconds the gameplay view goes black with the red GRZ logo in the center until you exit optics.\n'
        'Sky Colors: GREY added after WHITE under Options > Game; clouds retained and saved selections preserved.\n'
        'Killfeed: compact translucent row; killer, weapon, victim, distance; no outlines. Existing animations/prefixes stay active.\n'
        'No sway: removes breathing/horizontal aiming wander while preserving recoil.\n'
        'Aim Control TEST: 10% ADS recoil and hipfire/ADS aim-point alignment for supported hitscan shots. Native scope-entry timing and camera movement are restored. Read GRZ-AimControl-TEST.md.\n'
        'Hitscan is an EXPERIMENTAL damage-path replacement. Read GRZ-Hitscan-TEST.md for supported guns, native visual effects and duplicate-hit tests.\n'
        'Your generated inventory override is NOT included: it must be built with your own installed/custom gun definitions.\n'
        'Package and static source checks passed. Enforce compilation and in-game tests remain outstanding.\n'
    ).encode()
    target = output / 'GRZ-latest-mod-updates.zip'
    with ZipFile(target, 'w', ZIP_DEFLATED) as archive:
        for name, payload in payloads.items():
            archive.writestr(name, payload)
    with ZipFile(target) as archive:
        assert archive.testzip() is None
        assert set(archive.namelist()) == set(payloads)
        for name, payload in payloads.items():
            assert archive.read(name) == payload
    print(f'Verified {target.name}: {target.stat().st_size} bytes; six matching PBOs and installation notes.')
    print('Static/package checks only; actual scope rendering and timing require DayZ testing.')


if __name__ == '__main__':
    main()
