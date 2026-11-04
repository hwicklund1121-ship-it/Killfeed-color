"""Build GRZ Customs from the supplied HEXGRID models and approved PNG maps.

Requires Pillow with DDS DXT5 encoding. No engine compilation is performed.
"""
import argparse
import hashlib
import io
import json
import struct
import tempfile
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

from PIL import Image, ImageChops, ImageStat
from build_pbo import build
from build_status_textures import encode_paa
from repair_hexgrid import read_pbo, fix_hexgrid, verify_resources

ROOT = Path(__file__).resolve().parents[1]


def verify_paa(data, source):
    """Check every mip and decode the top level to catch swapped channels."""
    assert struct.unpack_from('<H', data)[0] == 0xFF05
    assert data[2:10] == b'GGATSFFO'
    offsets = struct.unpack_from('<16I', data, 14)
    at = 80  # format + OFFS header/payload + empty palette
    expected = source.size
    mips = 0
    while expected[0] >= 4 and expected[1] >= 4:
        assert offsets[mips] == at
        width, height = struct.unpack_from('<HH', data, at)
        size = int.from_bytes(data[at + 4:at + 7], 'little')
        assert (width, height) == expected and size == width * height
        if mips == 0:
            header = io.BytesIO()
            source.save(header, format='DDS', pixel_format='DXT5')
            decoded = Image.open(io.BytesIO(header.getvalue()[:128] + data[at + 7:at + 7 + size])).convert('RGBA')
            error = ImageStat.Stat(ImageChops.difference(decoded, source)).mean
            assert max(error) < 8, error
        at += 7 + size
        expected = (width // 2, height // 2)
        mips += 1
    assert data[at:] == bytes(6) and offsets[mips] == at
    return mips


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--hexgrid', type=Path, required=True)
    args = parser.parse_args()
    original = args.hexgrid.read_bytes()
    _, original_files = read_pbo(original)
    prefix, files = fix_hexgrid(original_files)
    maps = {}
    for source_name, target_name in [('colour', 'HEXGRID_co.paa'), ('normal', 'HEXGRID_nohq.paa')]:
        source_path = ROOT / 'art/customs/hexgrid' / (source_name + '.png')
        image = Image.open(source_path).convert('RGBA')
        assert image.size == (2048, 2048)
        # Store normal RGB directly, with opaque alpha and no SWIZ tag. Do not
        # carry the old texture's DXT5nm swizzle onto these RGB PNG channels.
        data = encode_paa(image)
        mips = verify_paa(data, image)
        files['HEXGRID/data/' + target_name] = data
        maps[target_name] = {'source_sha256': hashlib.sha256(source_path.read_bytes()).hexdigest(),
                             'paa_sha256': hashlib.sha256(data).hexdigest(),
                             'size': list(image.size), 'mips': mips, 'encoding': 'DXT5, direct RGBA, no swizzle'}
    for path, data in files.items():
        if path not in ('config.cpp', 'HEXGRID/data/HEXGRID_co.paa', 'HEXGRID/data/HEXGRID_nohq.paa'):
            assert data == original_files[path], path
    checked = verify_resources(prefix, files)
    with tempfile.TemporaryDirectory(prefix='grz-customs-') as folder:
        source = Path(folder)
        (source / '$PBOPREFIX$').write_text(prefix)
        for path, data in files.items():
            target = source / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        pbo = build(source)
    extensions, packed = read_pbo(pbo)
    assert packed == files and extensions['prefix'] == prefix
    assert hashlib.sha1(pbo[:-21]).digest() == pbo[-20:]
    output = ROOT / 'downloads'
    (output / 'HEXGRID.pbo').write_bytes(pbo)
    report = {'version': '0.1.0', 'included_reskins': ['Gunrunnerz HEXGRID'],
              'item_class': 'UAV_HEXGRID', 'original_sha256': hashlib.sha256(original).hexdigest(),
              'pbo_sha256': hashlib.sha256(pbo).hexdigest(), 'prefix': prefix,
              'verified_internal_resources': checked, 'maps': maps}
    (ROOT / 'GRZ-CUSTOMS-MANIFEST.json').write_text(json.dumps(report, indent=2) + '\n')
    with ZipFile(output / 'GRZ-Customs.zip', 'w', ZIP_DEFLATED) as archive:
        archive.writestr('@GRZ_Customs/Addons/HEXGRID.pbo', pbo)
        archive.writestr('@GRZ_Customs/mod.cpp', 'name = "GRZ Customs";\nauthor = "GRZ";\nversion = "0.1.0";\n')
        archive.write(ROOT / 'GRZ-CUSTOMS.md', 'INSTALL.md')
        archive.write(ROOT / 'GRZ-CUSTOMS-MANIFEST.json', '@GRZ_Customs/MANIFEST.json')
    # Keep the older direct model-fix download consistent with HEXGRID.pbo.
    with ZipFile(output / 'GRZ-HEXGRID-model-fix.zip', 'w', ZIP_DEFLATED) as archive:
        archive.writestr('@GRZ_Local/Addons/HEXGRID.pbo', pbo)
        archive.write(ROOT / 'HEXGRID-REPAIR.md', 'INSTALL.md')
    with ZipFile(output / 'GRZ-Customs.zip') as archive:
        assert archive.testzip() is None
        assert archive.read('@GRZ_Customs/Addons/HEXGRID.pbo') == pbo
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
