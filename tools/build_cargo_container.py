"""Build a separately named static container from supplied model and texture ZIP.

Only equal-length resource strings in the existing ODOL model are remapped;
native geometry, LODs and collision data are retained.
"""
import argparse
import hashlib
import io
import json
import re
import tempfile
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

from PIL import Image
from build_pbo import build
from build_status_textures import encode_paa
from repair_hexgrid import read_pbo, resource_refs

ROOT = Path(__file__).resolve().parents[1]


def material(normal):
    text = '''ambient[] = {1,1,1,1};
diffuse[] = {1,1,1,1};
forcedDiffuse[] = {0,0,0,0};
emmisive[] = {0,0,0,1};
specular[] = {0.08,0.08,0.08,1};
specularPower = 20;
PixelShaderID = "Super";
VertexShaderID = "Super";
'''
    stages = [normal, '#(argb,8,8,3)color(0.5,0.5,0.5,1,DT)',
              '#(argb,8,8,3)color(0,0,0,0,MC)', '#(argb,8,8,3)color(1,1,1,1,AS)',
              '#(argb,8,8,3)color(0,0,0,1,SMDI)', '#(ai,64,64,1)fresnel(1.3,0.3)',
              r'dz\data\data\env_land_co.paa']
    for index, texture in enumerate(stages, 1):
        text += f'class Stage{index}\n{{\n texture = "{texture}";\n uvSource = "tex";\n}};\n'
    return text.encode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--textures', type=Path, required=True)
    parser.add_argument('--model', type=Path, required=True)
    parser.add_argument('--name', required=True)
    args = parser.parse_args()
    assert re.fullmatch(r'GRZ_\w+', args.name)
    files, textures = {}, {}
    with ZipFile(args.textures) as archive:
        for name in archive.namelist():
            if not name.lower().endswith('.png'):
                continue
            image = Image.open(io.BytesIO(archive.read(name))).convert('RGBA')
            extrema = image.getextrema()
            # Constant colour/normal maps have no spatial content: 4x4 keeps
            # their exact values while avoiding redundant 4096x4096 payloads.
            if all(low == high for low, high in extrema):
                image = Image.new('RGBA', (4, 4), image.getpixel((0, 0)))
            if '_normal.png' in name.lower():
                key = 'normal'
            elif 'containers_colors_' in name:
                key = 'exterior'
            elif 'metal_white_lightrust' in name:
                key = 'metal'
            else:
                key = 'interior'
            encoded = encode_paa(image, 'DXT5' if key == 'normal' else 'DXT1')
            if key in textures:
                assert textures[key] == encoded, 'Normal maps differ; assign distinct maps instead.'
            textures[key] = encoded
    assert set(textures) == {'exterior', 'metal', 'interior', 'normal'}
    files['data/flat_nohq.paa'] = textures['normal']
    original = args.model.read_bytes()
    assert original[:4] == b'ODOL'
    model = original
    references = sorted(set(resource_refs(original)))
    replacements = {}
    for old in references:
        lower = old.lower().replace('/', '\\')
        payload, label = None, None
        if lower.endswith('.paa'):
            if re.search(r'containers_colors_\d+_co\.paa$', lower):
                label, payload = 'exterior_co.paa', textures['exterior']
            elif lower.endswith('metal_white_lightrust_co.paa'):
                label, payload = 'metal_co.paa', textures['metal']
            elif lower.endswith('basecooler_old_paint_co.paa'):
                label, payload = 'interior_co.paa', textures['interior']
        elif lower.endswith('.rvmat') and '\\containers\\data\\' in lower:
            label = 'interior.rvmat' if 'interior' in lower else 'metal.rvmat' if 'metal_white' in lower else 'exterior.rvmat'
            payload = material(args.name + r'\data\flat_nohq.paa')
        if payload is None:
            continue
        base = args.name + '\\data\\' + label
        assert len(base) <= len(old), (old, base)
        stem, ext = base.rsplit('.', 1)
        new = stem + '_' * (len(old) - len(base)) + '.' + ext
        assert len(new.encode()) == len(old.encode())
        path = new[len(args.name) + 1:].replace('\\', '/')
        if path in files:
            assert files[path] == payload
        files[path] = payload
        model = model.replace(old.encode(), new.encode())
        replacements[old] = new
    assert any('containers_colors_' in name for name in replacements)
    assert len(model) == len(original) and model[:12] == original[:12]
    restore = model
    for old, new in replacements.items():
        restore = restore.replace(new.encode(), old.encode())
    assert restore == original, 'A non-resource model byte changed.'
    files['model/container.p3d'] = model
    files['config.cpp'] = ('''class CfgPatches
{
    class ITEM_NAME
    {
        units[] = {"ITEM_NAME"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Structures","DZ_Structures_Industrial"};
    };
};
class CfgVehicles
{
    class HouseNoDestruct;
    class ITEM_NAME: HouseNoDestruct
    {
        scope = 2;
        displayName = "ITEM_NAME";
        descriptionShort = "Made By Blake";
        model = "ITEM_NAME\\model\\container.p3d";
    };
};
'''.replace('ITEM_NAME', args.name)).encode()
    mounted = {(args.name + '/' + name).replace('\\', '/').lower() for name in files}
    for name, payload in files.items():
        refs = resource_refs(payload) if name.endswith('.p3d') else re.findall(r'"([^"\r\n]+\.(?:paa|rvmat|p3d))"', payload.decode(), re.I) if name.endswith(('.cpp', '.rvmat')) else []
        for ref in refs:
            if ref.lower().startswith(args.name.lower()):
                assert ref.replace('\\', '/').lower() in mounted, (name, ref)
    with tempfile.TemporaryDirectory(prefix='grz-container-build-') as directory:
        source = Path(directory)
        (source / '$PBOPREFIX$').write_text(args.name)
        for name, payload in files.items():
            target = source / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(payload)
        pbo = build(source)
    extension, unpacked = read_pbo(pbo)
    assert extension['prefix'] == args.name and unpacked == files
    assert hashlib.sha1(pbo[:-21]).digest() == pbo[-20:]
    output = ROOT / 'downloads'
    (output / (args.name + '.pbo')).write_bytes(pbo)
    report = {'class': args.name, 'display_name': args.name, 'description': 'Made By Blake',
              'model_input_sha256': hashlib.sha256(original).hexdigest(),
              'pbo_sha256': hashlib.sha256(pbo).hexdigest(), 'bytes': len(pbo),
              'model_resource_remaps': replacements, 'model_geometry_and_size_preserved': True,
              'object_type': 'Static HouseNoDestruct; no inventory cargo or new door interactions added.'}
    notes = (args.name + '\n\nPut ' + args.name + '.pbo into @GRZ_Local/Addons on BOTH client and server, then restart both.\n'
             'Spawn class/display name: ' + args.name + '. Description: Made By Blake.\n'
             'Separate addon: requires vanilla DayZ only. Textures/materials are packaged under its own mount.\n'
             'This is a static shipping-container object for an admin/object spawner, not a new inventory-storage crate.\n'
             'Supplied compiled model LODs/geometry/collision are preserved; only equal-length asset references change.\n'
             '4K artwork retained; spatially constant material maps are stored at 4x4 with the same values.\n'
             'PBO, paths, checksums, model preservation and ZIP checks passed; runtime loading/spawning/rendering are untested.\n')
    with ZipFile(output / (args.name + '.zip'), 'w', ZIP_DEFLATED) as archive:
        archive.writestr('@GRZ_Local/Addons/' + args.name + '.pbo', pbo)
        archive.writestr('INSTALL.txt', notes)
        archive.writestr('MANIFEST.json', json.dumps(report, indent=2) + '\n')
    with ZipFile(output / (args.name + '.zip')) as archive:
        assert archive.testzip() is None
        assert archive.read('@GRZ_Local/Addons/' + args.name + '.pbo') == pbo
    (ROOT / (args.name + '-MANIFEST.json')).write_text(json.dumps(report, indent=2) + '\n')
    print(args.name, len(pbo), 'bytes; model geometry/size and all packaged resource paths verified. DayZ runtime testing needed.')


if __name__ == '__main__':
    main()
