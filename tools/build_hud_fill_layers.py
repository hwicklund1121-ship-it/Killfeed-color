"""Separate approved HUD artwork into outline/fill layers for native clipping.

Original PAA files are not changed. Black-over-white compositing reconstructs
source coverage; removing the white layer leaves the original black contours.
"""
import io
import struct
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
TEXTURES = ROOT/'GRZ_CustomHud/gui/textures'


def decode_top_mip(data):
    assert struct.unpack_from('<H',data)[0] == 0xff05
    offset = struct.unpack_from('<I',data,14)[0]
    w,h = struct.unpack_from('<HH',data,offset)
    size = int.from_bytes(data[offset+4:offset+7],'little')
    header=io.BytesIO()
    Image.new('RGBA',(w,h)).save(header,format='DDS',pixel_format='DXT5')
    return Image.open(io.BytesIO(header.getvalue()[:128]+data[offset+7:offset+7+size])).convert('RGBA')


def separate_layers(source):
    outline,fill=[],[]
    for r,g,b,a in source.get_flattened_data():
        coverage = a/255.0
        white = (r+g+b)/(3.0*255.0)
        black_alpha = coverage*(1-white)
        denominator = 1-black_alpha
        white_alpha = coverage*white/denominator if denominator>0 else 0
        outline.append((0,0,0,round(black_alpha*255)))
        fill.append((255,255,255,round(white_alpha*255)))
    black = Image.new('RGBA',source.size); black.putdata(outline)
    white = Image.new('RGBA',source.size); white.putdata(fill)
    return black,white


def encode_flat_alpha_paa(image, white=False):
    # Constant-color layers only need BC3's alpha channel. Encode its endpoints
    # directly so black/white matte edges keep their transparency accurately.
    output=io.BytesIO()
    output.write(struct.pack('<H',0xff05))
    output.write(b'GGATSFFO'+struct.pack('<I',64))
    table=output.tell();output.write(bytes(64)+b'\0\0')
    offsets=[]
    color=struct.pack('<HHI',65535 if white else 0,65535 if white else 0,0)
    while image.width>=4 and image.height>=4:
        offsets.append(output.tell())
        width,height=image.size
        alphas=list(image.getchannel('A').get_flattened_data())
        payload=bytearray()
        for by in range(0,height,4):
            for bx in range(0,width,4):
                block=[alphas[(by+y)*width+bx+x] for y in range(4) for x in range(4)]
                a0,a1=max(block),min(block)
                if a0==a1:
                    bits=0
                else:
                    palette=[a0,a1]+[((7-i)*a0+i*a1)//7 for i in range(1,7)]
                    bits=0
                    for i,alpha in enumerate(block):
                        index=min(range(8),key=lambda j:abs(palette[j]-alpha))
                        bits|=index<<(3*i)
                payload.extend(bytes([a0,a1])+bits.to_bytes(6,'little')+color)
        assert len(payload)==width*height
        output.write(struct.pack('<HH',width,height))
        output.write(len(payload).to_bytes(3,'little'))
        output.write(payload)
        image=image.resize((width//2,height//2),Image.Resampling.LANCZOS)
    offsets.append(output.tell());output.write(bytes(6))
    output.seek(table);output.write(struct.pack('<16I',*(offsets+[0]*(16-len(offsets)))))
    return output.getvalue()


def main():
    for name in ['health','blood']:
        source=decode_top_mip((TEXTURES/f'grz_{name}_ca.paa').read_bytes())
        outline,fill=separate_layers(source)
        for suffix,layer in [('outline',outline),('fill',fill)]:
            (TEXTURES/f'grz_{name}_{suffix}_ca.paa').write_bytes(encode_flat_alpha_paa(layer,white=suffix=='fill'))
        rows=[i//source.width for i,(r,g,b,a) in enumerate(source.get_flattened_data()) if a>128 and min(r,g,b)>180]
        print(f'{name}: white top={min(rows)/source.height:.8f} bottom={(max(rows)+1)/source.height:.8f}; outline/fill encoded.')


if __name__=='__main__':
    main()
