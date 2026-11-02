"""Build technical alpha-progress masks without changing approved HUD artwork.

The source PAA pixels are never modified. Black linework gets mask alpha zero,
so it survives every fill setting. White interior gets a bottom-to-top gradient.
"""
import io
import struct
from pathlib import Path
from PIL import Image
from build_status_textures import encode_paa

ROOT = Path(__file__).resolve().parents[1]
TEXTURES = ROOT/'GRZ_CustomHud/gui/textures'


def decode_top_mip(data):
    assert struct.unpack_from('<H', data)[0] == 0xff05
    offset = struct.unpack_from('<I', data, 14)[0]
    w,h = struct.unpack_from('<HH',data,offset)
    size = int.from_bytes(data[offset+4:offset+7],'little')
    header = io.BytesIO()
    Image.new('RGBA',(w,h)).save(header,format='DDS',pixel_format='DXT5')
    return Image.open(io.BytesIO(header.getvalue()[:128]+data[offset+7:offset+7+size])).convert('RGBA')


def make_mask(source):
    pixels = list(source.get_flattened_data())
    width,height = source.size
    white_rows = [i//width for i,(r,g,b,a) in enumerate(pixels) if a>100 and min(r,g,b)>180]
    if not white_rows:
        raise ValueError('No white icon interior found')
    top,bottom = min(white_rows),max(white_rows)
    output = []
    for i,(r,g,b,a) in enumerate(pixels):
        alpha = 0
        if a>0 and (r+g+b)/3>110:
            y = min(1,max(0,(i//width-top)/max(1,bottom-top)))
            alpha = round(8+239*(1-y))
        output.append((255,255,255,alpha))
    mask = Image.new('RGBA',source.size)
    mask.putdata(output)
    return mask


def main():
    for name in ['health','blood']:
        path = TEXTURES/f'grz_{name}_ca.paa'
        source = decode_top_mip(path.read_bytes())
        mask = make_mask(source)
        target = TEXTURES/f'grz_{name}_fillmask.paa'
        target.write_bytes(encode_paa(mask))
        print(f'Built {target.name}; original artwork preserved.')


if __name__ == '__main__':
    main()
