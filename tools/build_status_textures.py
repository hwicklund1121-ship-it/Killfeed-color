"""Encode approved artwork as alpha-preserving DXT5 PAA UI textures.
Requires Pillow >=12.3. Crops extract the two assets from the approved sheet;
resampling creates the square texture and mip chain required by the game.
"""
import io
import struct
from pathlib import Path
from PIL import Image, ImageOps

ROOT = Path(__file__).resolve().parents[1]

def encode_paa(image):
    output = io.BytesIO()
    output.write(struct.pack('<H', 0xFF05))
    output.write(b'GGATSFFO' + struct.pack('<I', 64))
    offset_position = output.tell()
    output.write(bytes(64) + b'\0\0')
    offsets = []
    while image.width >= 4 and image.height >= 4:
        offsets.append(output.tell())
        dds = io.BytesIO()
        image.save(dds, format='DDS', pixel_format='DXT5')
        compressed = dds.getvalue()[128:]
        assert len(compressed) == image.width * image.height
        output.write(struct.pack('<HH', *image.size))
        output.write(len(compressed).to_bytes(3, 'little'))
        output.write(compressed)
        image = image.resize((image.width // 2, image.height // 2), Image.Resampling.LANCZOS)
    offsets.append(output.tell())
    output.write(bytes(6))
    output.seek(offset_position)
    output.write(struct.pack('<16I', *(offsets + [0] * (16 - len(offsets)))))
    return output.getvalue()

if __name__ == '__main__':
    sheet = Image.open(ROOT / 'art/status-sheet.png').convert('RGBA')
    for name, box in [('health', (380, 20, 1060, 710)), ('blood', (1290, 0, 1810, 725))]:
        # Preserve the approved artwork; only extract, fit and encode it.
        source = Image.open(ROOT / 'art/health-refined.png').convert('RGBA') if name == 'health' else sheet.crop(box)
        asset = ImageOps.contain(source, (464, 464), Image.Resampling.LANCZOS)
        texture = Image.new('RGBA', (512, 512))
        texture.alpha_composite(asset, ((512 - asset.width) // 2, (512 - asset.height) // 2))
        texture.save(ROOT / f'art/{name}-preview.png')
        target = ROOT / f'GRZ_CustomHud/gui/textures/grz_{name}_ca.paa'
        target.write_bytes(encode_paa(texture))
        print(target.name)

    # Convert the approved transparent weapon outline; no painted panel fill.
    outline = Image.open(ROOT / 'art/weapon-outline.png').convert('RGBA')
    outline = outline.resize((1024, 256), Image.Resampling.LANCZOS)
    assert outline.getpixel((512, 128))[3] == 0
    (ROOT / 'GRZ_CustomHud/gui/textures/grz_weapon_outline_ca.paa').write_bytes(encode_paa(outline))
    print('grz_weapon_outline_ca.paa')
