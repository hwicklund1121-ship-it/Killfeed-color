"""Replace only GRZ_Container's visual model with the UV-correct FBX conversion."""
import argparse
import hashlib
import json
import re
import subprocess
import tempfile
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile

from PIL import Image
from build_pbo import build
from build_status_textures import encode_paa
from repair_hexgrid import read_pbo, resource_refs

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--fbx', required=True)
parser.add_argument('--physics', required=True)
parser.add_argument('--codec', required=True)
args = parser.parse_args()
output = ROOT / 'downloads'
old_pbo = output / 'GRZ_Container.pbo'
extension, old = read_pbo(old_pbo.read_bytes())
report_path = ROOT / 'GRZ_Container-MANIFEST.json'
previous = json.loads(report_path.read_text())
remaps = previous.get('model_resource_remaps', previous.get('legacy_model_resource_remaps', {}))
def original_asset(fragment):
    canonical = 'data/fbx_exterior_co.paa' if 'containers_colors_' in fragment else 'data/fbx_interior_co.paa'
    if canonical in old:
        return old[canonical]
    path = next(new for old_path, new in remaps.items() if fragment in old_path)
    return old[path.split('\\', 1)[1].replace('\\', '/')]

files = {'config.cpp': old['config.cpp'], 'data/flat_nohq.paa': old['data/flat_nohq.paa'],
         'data/fbx_exterior_co.paa': original_asset('containers_colors_'),
         'data/fbx_interior_co.paa': original_asset('basecooler_old_paint_co.paa'),
         'data/fbx_metal_co.paa': encode_paa(Image.new('RGBA', (4, 4), (255, 255, 255, 255)), 'DXT1')}
material = next(data for name, data in old.items() if name.endswith('.rvmat'))
for key in ['exterior', 'interior', 'metal']:
    files['data/fbx_' + key + '.rvmat'] = material
with tempfile.TemporaryDirectory(prefix='grz-single-fbx-fix-') as folder:
    directory = Path(folder)
    model = directory / 'container.p3d'
    result = subprocess.run(['blender', '--background', '--factory-startup', '--python',
                             str(ROOT / 'tools/export_single_container_fbx.py'), '--',
                             '--fbx', args.fbx, '--physics', args.physics, '--codec', args.codec,
                             '--output', str(model)], check=True, capture_output=True, text=True)
    if not model.exists():
        raise RuntimeError(result.stdout[-3000:] + result.stderr[-1000:])
    files['model/container.p3d'] = model.read_bytes()
    stats = json.loads(Path(str(model) + '.json').read_text())
    mounted = {'grz_container/' + name.lower() for name in files}
    for name, data in files.items():
        if name.endswith('.p3d'):
            refs = resource_refs(data)
        elif name.endswith(('.cpp', '.rvmat')):
            refs = re.findall(r'"([^"\r\n]+\.(?:paa|rvmat|p3d))"', data.decode(), re.I)
        else:
            continue
        for ref in refs:
            if ref.lower().startswith('grz_'):
                assert ref.replace('\\', '/').lower() in mounted, (name, ref)
    source = directory / 'addon'
    source.mkdir()
    (source / '$PBOPREFIX$').write_text('GRZ_Container')
    for name, data in files.items():
        target = source / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    pbo = build(source)
assert read_pbo(pbo)[1] == files and hashlib.sha1(pbo[:-21]).digest() == pbo[-20:]
old_pbo.write_bytes(pbo)
report = {'class': 'GRZ_Container', 'display_name': 'GRZ_Container', 'description': 'Made By Blake',
          'visual_model': stats, 'pbo_sha256': hashlib.sha256(pbo).hexdigest(),
          'source_fbx_sha256': hashlib.sha256(Path(args.fbx).read_bytes()).hexdigest(),
          'legacy_model_resource_remaps': remaps,
          'runtime_tested': False}
report_path.write_text(json.dumps(report, indent=2) + '\n')
notes = '''GRZ_Container - source FBX UV fix

Replace GRZ_Container.pbo in @GRZ_Local/Addons on BOTH client and server;
restart both and spawn a fresh GRZ_Container. Description: Made By Blake.
The double-container PBO is unchanged.

The visible mesh now comes from the original 1A.fbx and retains its UVs and
material assignments, matching the supplied containers_colors_03 artwork.
The prior 1mo collision/contact LODs are retained; check physical fit in-game.
This P3D is editable MLOD, not binarized ODOL. Binarize with DayZ Tools if the
runtime rejects the editable model; no DayZ engine is available here to test.
UV round-trip error below 0.000001, model size, material/texture paths, ZIP/PBO
contents and checksums were checked. Spawning/rendering remain unverified.

Conversion uses the unmodified external Arma 3 Object Builder P3D codec:
https://github.com/MrClock8163/Arma3ObjectBuilder (GPL-3.0; separate build tool).
'''
with ZipFile(output / 'GRZ_Container.zip', 'w', ZIP_DEFLATED) as archive:
    archive.writestr('@GRZ_Local/Addons/GRZ_Container.pbo', pbo)
    archive.writestr('INSTALL.txt', notes)
    archive.writestr('MANIFEST.json', json.dumps(report, indent=2) + '\n')
with ZipFile(output / 'GRZ_Container.zip') as archive:
    assert archive.testzip() is None
    assert archive.read('@GRZ_Local/Addons/GRZ_Container.pbo') == pbo
print(json.dumps(stats, indent=2))
print('Single-container PBO rebuilt with FBX visual UVs and existing physics LODs. Runtime untested.')
