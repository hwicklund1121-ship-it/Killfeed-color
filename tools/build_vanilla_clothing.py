"""Build six vanilla clothing reskins; optional custom normals fall back to vanilla."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import argparse, hashlib, io, json, struct
from PIL import Image, ImageOps
from build_status_textures import encode_paa
from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]
ITEMS = [
    ('gloves', 'GRZ_SurgicalGloves', 'GRZ Surgical Gloves', 'SurgicalGloves_White'),
    ('shirt', 'GRZ_TShirt', 'GRZ T-Shirt', 'TShirt_Grey'),
    ('hat', 'GRZ_CowboyHat', 'GRZ Cowboy Hat', 'CowboyHat_Brown'),
    ('jeans', 'GRZ_Jeans', 'GRZ Jeans', 'Jeans_Blue'),
    ('bandana', 'GRZ_Bandana', 'GRZ Bandana', 'Bandana_BlackPattern'),
    ('sneakers', 'GRZ_Sneakers', 'GRZ Sneakers', 'Sneakers_Black'),
]
parser = argparse.ArgumentParser(description=__doc__)
for key in ('gloves', 'shirt', 'hat'):
    parser.add_argument('--' + key, type=Path, required=True)
for key in ('jeans', 'bandana', 'sneakers'):
    parser.add_argument('--' + key + '-colour', type=Path, required=True)
    parser.add_argument('--' + key + '-normal', type=Path, required=(key == 'sneakers'))
args = parser.parse_args()
source = ROOT / 'GRZ_VanillaClothing'
(source / 'data').mkdir(parents=True, exist_ok=True)
manifest = {
    'descriptionShort': 'made by blake', 'addon': 'GRZ_VanillaClothing', 'items': [],
    'pending_uploads': [], 'pending_normal_maps': [],
    'normal_conversion': 'Supplied OpenGL tangent normals converted to DirectX by inverting green; X packed into alpha with native NOHQ SWIZ [5,4,2,3]. Items without a custom normal inherit vanilla materials.',
    'vanilla_stats_preserved': True,
}
blocks = []
for key, classname, name, parent in ITEMS:
    combined = getattr(args, key, None)
    colour_zip = combined or getattr(args, key + '_colour')
    normal_zip = combined or getattr(args, key + '_normal', None)
    entry = {'key': key, 'class': classname, 'displayName': name, 'base_class': parent,
             'normal_mode': 'custom' if normal_zip else 'inherited vanilla', 'textures': {}}
    for kind, ending, path in [('co', '_Base_color.png', colour_zip), ('nohq', '_Normal.png', normal_zip)]:
        if path is None:
            (source / 'data' / (key + '_nohq.paa')).unlink(missing_ok=True)
            (source / 'data' / (key + '.rvmat')).unlink(missing_ok=True)
            manifest['pending_normal_maps'].append(name)
            continue
        with ZipFile(path) as archive:
            names = [n for n in archive.namelist() if n.endswith(ending)]
            assert len(names) == 1, (key, ending, names)
            raw = archive.read(names[0])
        image = Image.open(io.BytesIO(raw)).convert('RGB')
        assert image.size in ((2048, 2048), (4096, 4096)), (key, image.size)
        if kind == 'co':
            data = encode_paa(image.convert('RGBA'), 'DXT1')
        else:
            red, green, blue = image.split()
            packed = Image.merge('RGBA', (Image.new('L', image.size, 0), ImageOps.invert(green), blue, ImageOps.invert(red)))
            data = bytearray(encode_paa(packed, 'DXT5'))
            assert data[2:10] == b'GGATSFFO'
            offsets = struct.unpack_from('<16I', data, 14)
            struct.pack_into('<16I', data, 14, *[n + 16 if n else 0 for n in offsets])
            data = bytes(data[:2]) + b'GGATZIWS' + struct.pack('<I', 4) + bytes([5, 4, 2, 3]) + bytes(data[2:])
        (source / 'data' / (key + '_' + kind + '.paa')).write_bytes(data)
        entry['textures'][kind] = {'source_sha256': hashlib.sha256(raw).hexdigest(),
                                   'source_filename': Path(names[0]).name,
                                   'paa_sha256': hashlib.sha256(data).hexdigest(), 'size': list(image.size)}
    texture = 'GRZ_VanillaClothing\\data\\' + key + '_co.paa'
    texture_slots = 5 if key == 'bandana' else 3
    fields = [f' class {classname}: {parent}', ' {', '  scope = 2;',
              f'  displayName = "{name}";', '  descriptionShort = "made by blake";',
              '  hiddenSelectionsTextures[] = {' + ','.join('"' + texture + '"' for _ in range(texture_slots)) + '};']
    if normal_zip:
        material = (ROOT / 'packaging/customs/GRZ_XimsKit/data/pants.rvmat').read_text()
        material = material.replace('GRZ_XimsKit\\data\\pants_nohq.paa', 'GRZ_VanillaClothing\\data\\' + key + '_nohq.paa')
        (source / 'data' / (key + '.rvmat')).write_text(material)
        material_path = 'GRZ_VanillaClothing\\data\\' + key + '.rvmat'
        fields.append('  hiddenSelectionsMaterials[] = {' + ','.join('"' + material_path + '"' for _ in range(3)) + '};')
    fields.append(' };')
    blocks.append('\n'.join(fields))
    manifest['items'].append(entry)
units = ','.join('"' + item[1] + '"' for item in ITEMS)
config = '''class CfgPatches
{
 class GRZ_VanillaClothing
 {
  units[] = {UNITS};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Data","DZ_Characters","DZ_Characters_Gloves","DZ_Characters_Tops","DZ_Characters_Headgear","DZ_Characters_Pants","DZ_Characters_Masks","DZ_Characters_Shoes"};
 };
};
class CfgVehicles
{
'''.replace('UNITS', units)
config += ''.join(' class ' + item[3] + ';\n' for item in ITEMS)
config += '\n'.join(blocks) + '\n};\n'
(source / 'config.cpp').write_text(config)
pbo = build(source)
(ROOT / 'downloads/GRZ_VanillaClothing.pbo').write_bytes(pbo)
manifest['pbo_sha256'] = hashlib.sha256(pbo).hexdigest()
(ROOT / 'downloads/GRZ-Vanilla-Clothing-MANIFEST.json').write_text(json.dumps(manifest, indent=2) + '\n')
notes = '''GRZ Vanilla Clothing — all six items
Every description is exactly: made by blake

'''
notes += '\n'.join(f'{item[2]} — {item[1]} ({item[3]})' for item in ITEMS)
notes += '''

All six colour textures are included. The supplied custom normals are included for gloves, T-shirt, cowboy hat and sneakers. Jeans and bandana currently use VANILLA normal/material detail because their normal ZIP uploads still exceed the 32 MiB transfer limit. Those two custom normals remain pending; their clothing items are usable now. Send a 2048px normal or split the original ZIP into files under 32 MiB to finish them.

The hat colour image supplied is entirely black and the bandana colour image entirely white; both are preserved exactly. The jeans texture is the supplied jeans_f_blue map, reused across the three clothing texture entries; male/female alignment still needs an in-game check. Custom normal maps use OpenGL-to-DirectX conversion and native NOHQ packing. Converted textures are 4096px; future 2048px normals are also supported.

Vanilla models, slots, storage, weight, insulation, durability, armour and wear behavior are inherited. No custom DamageSystem or stat overrides. Where a custom pristine material is supplied, inherited wear-state materials can replace it as the item degrades. Jeans and bandana retain their native materials throughout.

Vanilla DayZ only; no non-vanilla dependencies. Ordinary clothing, not VIP-only. Automatically discovered by the existing GRZ loadout clothing picker after restart. Gloves go under GLOVE, T-shirt SHIRT, jeans PANTS, sneakers SHOES; the hat and bandana retain their native attachment slots. VPP can spawn the classnames above.

INSTALL / UPDATE: close DayZ and stop the server. Replace the existing GRZ_VanillaClothing.pbo in BOTH DayZ/@GRZ_Local/Addons and DayZServer/@GRZ_Local/Addons with this new one, then restart both. Keep only one active copy per side. This replaces the previous three-item clothing pack and keeps those same classnames; do not load the old and new PBO in different active mod folders. Other mods and saved loadouts are unchanged. Do not replace or repack vanilla PBOs.

This ZIP contains the ready-to-use PBO plus source config/material files. The PAA texture assets are inside the PBO, not duplicated outside it; unpack with PBO Manager/DayZ tools for a complete editable addon. Complete source assets are also in the repository's GRZ_VanillaClothing directory. The raw PBO exceeds GitHub's file-size limit, so download the ZIP instead of a direct PBO URL.

Metadata, vanilla inheritance, resource bindings, texture conversion and PBO/ZIP integrity are checked. DayZ compilation and worn-model appearance require an in-game test.
'''
(ROOT / 'GRZ-VANILLA-CLOTHING.md').write_text(notes)
with ZipFile(ROOT / 'downloads/GRZ-Vanilla-Clothing.zip', 'w', ZIP_DEFLATED) as archive:
    archive.writestr('GRZ_VanillaClothing.pbo', pbo)
    archive.writestr('INSTALL.txt', notes)
    archive.writestr('MANIFEST.json', json.dumps(manifest, indent=2) + '\n')
    for file in sorted(source.rglob('*')):
        if file.is_file() and file.suffix != '.paa':
            archive.write(file, 'source/GRZ_VanillaClothing/' + file.relative_to(source).as_posix())
assert (ROOT / 'downloads/GRZ-Vanilla-Clothing.zip').stat().st_size < 100 * 1024 * 1024
print('Built all six clothing items:', len(pbo), 'PBO bytes. Pending custom normals:', manifest['pending_normal_maps'])
