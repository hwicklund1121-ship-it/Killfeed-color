"""Encode approved artwork as alpha-preserving DXT5 PAA UI textures.
Requires Pillow >=12.3. Crops extract the two assets from the approved sheet;
resampling creates the square texture and mip chain required by the game.
"""
import io
import struct
from pathlib import Path
from PIL import Image, ImageOps

ROOT = Path(__file__).resolve().parents[1]

def encode_paa(image, pixel_format='DXT5', alpha_flags=False):
    assert pixel_format in ('DXT1', 'DXT5')
    output = io.BytesIO()
    output.write(struct.pack('<H', 0xFF01 if pixel_format == 'DXT1' else 0xFF05))
    output.write(b'GGATSFFO' + struct.pack('<I', 64))
    offset_position = output.tell()
    output.write(bytes(64))
    if alpha_flags:
        output.write(b'GGATGALF' + struct.pack('<II', 4, 2))
    output.write(b'\0\0')
    offsets = []
    while image.width >= 4 and image.height >= 4:
        offsets.append(output.tell())
        dds = io.BytesIO()
        image.save(dds, format='DDS', pixel_format=pixel_format)
        compressed = dds.getvalue()[128:]
        assert len(compressed) == image.width * image.height // (2 if pixel_format == 'DXT1' else 1)
        stored_width = image.width
        if len(compressed) > 0xFFFFFF:
            import lzokay
            raw = compressed
            compressed = lzokay.compress(raw)
            assert lzokay.decompress(compressed, len(raw)) == raw
            assert len(compressed) <= 0xFFFFFF
            stored_width |= 0x8000
        output.write(struct.pack('<HH', stored_width, image.height))
        output.write(len(compressed).to_bytes(3, 'little'))
        output.write(compressed)
        image = image.resize((image.width // 2, image.height // 2), Image.Resampling.LANCZOS)
    offsets.append(output.tell())
    output.write(bytes(6))
    output.seek(offset_position)
    output.write(struct.pack('<16I', *(offsets + [0] * (16 - len(offsets)))))
    return output.getvalue()

if __name__ == '__main__':
    for name, white_height in [('health', 309), ('blood', 245)]:
        source = Image.open(ROOT / f'art/{name}-refined.png').convert('RGBA')
        # Encode at the prior white-glyph size: only the outer outline grows.
        white = Image.new('L', source.size)
        white.putdata([255 if a > 128 and min(r, g, b) > 200 else 0 for r, g, b, a in source.get_flattened_data()])
        bounds = white.getbbox()
        if not bounds:
            raise ValueError(f'No white glyph found in {name}')
        factor = white_height / (bounds[3] - bounds[1])
        asset = source.resize((round(source.width * factor), round(source.height * factor)), Image.Resampling.LANCZOS)
        center_x = (bounds[0] + bounds[2]) * factor / 2
        center_y = (bounds[1] + bounds[3]) * factor / 2
        texture = Image.new('RGBA', (512, 512))
        texture.alpha_composite(asset, (round(256 - center_x), round(266 - center_y)))
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
