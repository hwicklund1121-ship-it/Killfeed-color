"""Generate gun-wrap colour variants from the supplied native UV texture.

Keep native model/material/attachment behaviour and the source cut-out alpha.
"""
import json
import struct
from pathlib import Path

from PIL import Image, ImageOps
from build_status_textures import encode_paa

ROOT = Path(__file__).resolve().parents[1]
SOLIDS = {
    'Black': '#181818', 'White': '#ffffff', 'Grey': '#909090', 'Charcoal': '#383838',
    'Silver': '#c5cbd0', 'Red': '#ff2020', 'DarkRed': '#8d101a', 'Crimson': '#dc143c',
    'Orange': '#ff8000', 'Amber': '#ffb000', 'Yellow': '#fff020', 'Gold': '#e8bd35',
    'Lime': '#b6ff20', 'Green': '#20cb45', 'DarkGreen': '#155734', 'Olive': '#8a9138',
    'Mint': '#9effce', 'Teal': '#00a899', 'Cyan': '#00f5ff', 'Turquoise': '#30d5c8',
    'SkyBlue': '#79caff', 'Blue': '#2560ff', 'DarkBlue': '#17257d', 'Navy': '#172448',
    'Purple': '#9e35ef', 'Violet': '#743dff', 'Lavender': '#cfafff', 'Magenta': '#ff00dc',
    'Pink': '#ff75c8', 'HotPink': '#ff1493', 'Rose': '#d88b9b', 'Brown': '#81502d',
    'Tan': '#ceb38a', 'Beige': '#e8d8b7', 'Cream': '#fff4ca', 'Maroon': '#681b36',
}
MIXES = {
    'Red_Black': ['Red', 'Black'], 'White_Black': ['White', 'Black'],
    'Blue_Black': ['Blue', 'Black'], 'Pink_Black': ['HotPink', 'Black'],
    'Purple_Black': ['Purple', 'Black'], 'Gold_Black': ['Gold', 'Black'],
    'Red_White': ['Red', 'White'], 'Blue_White': ['Blue', 'White'],
    'Pink_White': ['Pink', 'White'], 'Cyan_Purple': ['Cyan', 'Purple'],
    'Red_Blue': ['Red', 'Blue'],
    'Rainbow': ['Red', 'Orange', 'Yellow', 'Green', 'Cyan', 'Blue', 'Purple', 'Pink'],
}


def build_wraps(verify):
    source = Image.open(ROOT / 'art/customs/gunwraps/source.png').convert('RGBA')
    alpha = source.getchannel('A')
    # Lift native brown/green into a neutral, detailed fabric value before tinting.
    luminance = ImageOps.autocontrast(ImageOps.grayscale(source), cutoff=1)
    luminance = luminance.point(lambda value: 90 + round(value * 165 / 255))
    output = ROOT / 'packaging/customs/GRZ_Gunwraps'
    (output / 'data').mkdir(parents=True, exist_ok=True)
    (output / '$PBOPREFIX$').write_text('GRZ_Gunwraps\n')
    configs, manifest = [], []
    for name in list(SOLIDS) + list(MIXES):
        names = MIXES.get(name, [name])
        images = []
        for colour in names:
            image = ImageOps.colorize(luminance, '#000000', SOLIDS[colour]).convert('RGBA')
            image.putalpha(alpha)
            images.append(image)
        image = images[0].copy()
        if len(images) > 1:
            # Small alternating patches across both cloth and loose-strand UVs.
            pixels = image.load()
            layers = [layer.load() for layer in images]
            for y in range(image.height):
                for x in range(image.width):
                    layer = ((x // 96) + 2 * (y // 80) + (x // 41 + y // 53) % 3) % len(layers)
                    pixels[x, y] = layers[layer][x, y]
        assert image.getchannel('A').tobytes() == alpha.tobytes()
        data = encode_paa(image)
        verify(data, image)
        # Native colour map carries alpha-test flag 2. Insert the same tag and
        # move every nonzero mip offset forward by the new 16-byte tag.
        offsets = struct.unpack_from('<16I', data, 14)
        data = data[:14] + struct.pack('<16I', *(x + 16 if x else 0 for x in offsets)) + data[78:]
        data = data[:2] + b'GGATGALF' + struct.pack('<II', 4, 2) + data[2:]
        assert struct.unpack_from('<HH', data, 96) == source.size
        filename = name.lower() + '_co.paa'
        (output / 'data' / filename).write_bytes(data)
        classname = 'GRZ_Gunwraps_' + name
        configs.append('    class ' + classname + ': GhillieAtt_ColorBase\n    {\n'
                       '        scope = 2;\n        displayName = "' + classname + '";\n'
                       '        descriptionShort = "Made By Blake";\n'
                       '        hiddenSelectionsTextures[] = {"GRZ_Gunwraps\\data\\' + filename + '"};\n    };\n')
        manifest.append({'class': classname, 'display_name': classname, 'description': 'Made By Blake',
                         'colours': names, 'texture': 'data/' + filename})
    units = ','.join('"' + item['class'] + '"' for item in manifest)
    config = ('class CfgPatches\n{\n    class GRZ_Customs_Gunwraps\n    {\n'
              '        units[] = {' + units + '};\n        weapons[] = {};\n'
              '        requiredVersion = 0.1;\n'
              '        requiredAddons[] = {"DZ_Data","DZ_Weapons_Supports"};\n    };\n};\n'
              'class CfgVehicles\n{\n    class GhillieAtt_ColorBase;\n' + ''.join(configs) + '};\n')
    (output / 'config.cpp').write_text(config)
    (ROOT / 'GRZ-GUNWRAPS-ITEMS.json').write_text(json.dumps(manifest, indent=2) + '\n')
    return len(manifest)
