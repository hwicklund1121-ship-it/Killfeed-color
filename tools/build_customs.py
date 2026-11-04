"""Build GRZ Customs from the supplied HEXGRID models and approved PNG maps.

Requires Pillow with DDS DXT5 encoding. No engine compilation is performed.
"""
import argparse
import hashlib
import io
import json
import re
import struct
import tempfile
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

from PIL import Image, ImageChops, ImageStat
from build_pbo import build
from build_status_textures import encode_paa
from repair_hexgrid import read_pbo, fix_hexgrid, verify_resources
from hexgrid_balance import balance_hexgrid, report as armour_report

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
    base_prefix, base_files = fix_hexgrid(original_files)
    base_files = balance_hexgrid(base_files)
    prefix = 'GRZ_Customs_Hexgrid'
    files = {}
    maps = {}
    for source_name, target_name in [('colour', 'HEXGRID_co.paa'), ('normal', 'HEXGRID_nohq.paa')]:
        source_path = ROOT / 'art/customs/hexgrid' / (source_name + '.png')
        image = Image.open(source_path).convert('RGBA')
        assert image.size == (2048, 2048)
        # Store normal RGB directly, with opaque alpha and no SWIZ tag. Do not
        # carry the old texture's DXT5nm swizzle onto these RGB PNG channels.
        data = encode_paa(image)
        mips = verify_paa(data, image)
        files['data/' + target_name] = data
        maps[target_name] = {'source_sha256': hashlib.sha256(source_path.read_bytes()).hexdigest(),
                             'paa_sha256': hashlib.sha256(data).hexdigest(),
                             'size': list(image.size), 'mips': mips, 'encoding': 'DXT5, direct RGBA, no swizzle'}
    material = original_files['HEXGRID/data/HEXGRID.rvmat'].decode('utf-8-sig')
    material = material.replace('PVPPack\\HEXGRID\\HEXGRID\\data\\HEXGRID_nohq.paa',
                                'GRZ_Customs_Hexgrid\\data\\HEXGRID_nohq.paa')
    assert 'GRZ_Customs_Hexgrid\\data\\HEXGRID_nohq.paa' in material
    files['data/HEXGRID.rvmat'] = material.encode()
    config = r'''
class CfgPatches
{
    class GRZ_Customs_Hexgrid
    {
        units[] = {"GRZ_GunRunnerZ_Hexgrid"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Characters","DZ_Characters_Vests","Hexgrid"};
    };
};
class CfgVehicles
{
    class HEXGRID_Base;
    class GRZ_GunRunnerZ_Hexgrid: HEXGRID_Base
    {
        scope = 2;
        displayName = "GunRunnerZ Hexgrid";
        descriptionShort = "Made By Blake";
        hiddenSelectionsTextures[] = {"GRZ_Customs_Hexgrid\data\HEXGRID_co.paa"};
        hiddenSelectionsMaterials[] = {"GRZ_Customs_Hexgrid\data\HEXGRID.rvmat"};
        
    };
};
'''
    files['config.cpp'] = config.encode()
    assert not re.search(r'class\s+(DamageSystem|GlobalHealth|Health)\s*:\s*\1\b', config)
    # Resolve references across both PBO mounts, including original material maps.
    mounted = {(base_prefix + '/' + name).replace('\\', '/').lower() for name in base_files}
    mounted |= {(prefix + '/' + name).replace('\\', '/').lower() for name in files}
    checked = verify_resources(base_prefix, base_files)
    for path, data in files.items():
        if path.endswith(('.cpp', '.rvmat')):
            for ref in re.findall(r'"([^"\r\n]+\.(?:paa|rvmat))"', data.decode(), re.I):
                if ref.lower().startswith(('grz_customs', 'pvppack')):
                    assert ref.replace('\\', '/').lower() in mounted, ref
                    checked += 1
    with tempfile.TemporaryDirectory(prefix='grz-customs-') as folder:
        source = Path(folder)
        (source / '$PBOPREFIX$').write_text(prefix)
        for path, data in files.items():
            target = source / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        pbo = build(source)
    with tempfile.TemporaryDirectory(prefix='grz-hexgrid-base-') as folder:
        source = Path(folder)
        (source / '$PBOPREFIX$').write_text(base_prefix)
        for path, data in base_files.items():
            target = source / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        base_pbo = build(source)
    for path, data in base_files.items():
        if path != 'config.cpp':
            assert data == original_files[path], path
    assert read_pbo(base_pbo)[1] == base_files
    extensions, packed = read_pbo(pbo)
    assert packed == files and extensions['prefix'] == prefix
    assert hashlib.sha1(pbo[:-21]).digest() == pbo[-20:]
    output = ROOT / 'downloads'
    (output / 'HEXGRID.pbo').write_bytes(base_pbo)
    (output / 'GRZ_Customs_Hexgrid.pbo').write_bytes(pbo)
    extras = {}
    extra_report = {}
    from build_gunwraps import build_wraps
    wrap_count = build_wraps(verify_paa)
    for name, target in [('colour', 'tracksuit_co.paa'), ('normal', 'tracksuit_nohq.paa')]:
        image = Image.open(ROOT / 'art/customs/tracksuit' / (name + '.png')).convert('RGBA')
        data = encode_paa(image)
        verify_paa(data, image)
        (ROOT / 'packaging/customs/GRZ_Tracksuit_Pants/data' / target).write_bytes(data)
    for source in sorted((ROOT / 'packaging/customs').iterdir()):
        if not source.is_dir():
            continue
        addon = build(source)
        extension, contents = read_pbo(addon)
        assert hashlib.sha1(addon[:-21]).digest() == addon[-20:]
        mounted = {(extension['prefix'] + '/' + name).lower() for name in contents}
        for path, data in contents.items():
            assert data == (source / path).read_bytes()
            if path.endswith(('.cpp', '.rvmat')):
                for ref in re.findall(r'"([^"\r\n]+\.(?:paa|rvmat))"', data.decode('utf-8-sig'), re.I):
                    if ref.replace('\\', '/').lower().startswith(extension['prefix'].replace('\\', '/').lower() + '/'):
                        assert ref.replace('\\', '/').lower() in mounted, ref
        config = contents['config.cpp'].decode()
        assert not re.search(r'class\s+(DamageSystem|GlobalHealth|Health)\s*:\s*\1\b', config)
        assert 'BUCA_Vest_HexGrid' not in config and 'blake_beaniehat' not in config
        assert not any('/hexgrid/' in path.lower() for path in contents)
        extras[source.name + '.pbo'] = addon
        extra_report[source.name] = {'sha256': hashlib.sha256(addon).hexdigest(),
                                    'prefix': extension['prefix'], 'files': len(contents)}
    from consolidate_customs import consolidate
    consolidated, consolidated_report = consolidate([pbo] + list(extras.values()))
    extras = {'GRZ_Customs.pbo': consolidated}
    extra_report = {'GRZ_Customs': consolidated_report}
    (output / 'GRZ_Customs.pbo').write_bytes(consolidated)
    report = {'version': '0.1.9', 'hexgrid_armour': armour_report(), 'gunwrap_variants': wrap_count, 'included_reskins': ['Gunrunnerz HEXGRID', 'Blakes Kit (without kit HEXGRID)', 'GRZ_Tracksuit_Pants', 'GRZ_Gunwraps'],
              'additional_addons': extra_report,
              'item_class': 'GRZ_GunRunnerZ_Hexgrid', 'base_item_class': 'HEXGRID_Base',
              'display_name': 'GunRunnerZ Hexgrid', 'description': 'Made By Blake',
              'base_pbo_sha256': hashlib.sha256(base_pbo).hexdigest(), 'original_sha256': hashlib.sha256(original).hexdigest(),
              'pbo_sha256': hashlib.sha256(consolidated).hexdigest(), 'prefix': 'GRZ_Customs',
              'verified_internal_resources': checked, 'maps': maps}
    (ROOT / 'GRZ-CUSTOMS-MANIFEST.json').write_text(json.dumps(report, indent=2) + '\n')
    with ZipFile(output / 'GRZ-Customs.zip', 'w', ZIP_DEFLATED) as archive:
        archive.writestr('@GRZ_Customs/Addons/HEXGRID.pbo', base_pbo)
        for name, addon in extras.items():
            archive.writestr('@GRZ_Customs/Addons/' + name, addon)
        archive.writestr('@GRZ_Customs/mod.cpp', 'name = "GRZ Customs";\nauthor = "GRZ";\nversion = "0.1.9";\n')
        archive.write(ROOT / 'GRZ-CUSTOMS.md', 'INSTALL.md')
        archive.write(ROOT / 'GRZ-CUSTOMS-MANIFEST.json', '@GRZ_Customs/MANIFEST.json')
        archive.write(ROOT / 'GRZ-GUNWRAPS-ITEMS.json', '@GRZ_Customs/GUNWRAPS-ITEMS.json')
        for name in ['Install-GRZCustoms.ps1', 'Install-GRZCustoms.cmd']:
            archive.write(ROOT / 'packaging/customs' / name, name)
    # Keep the older direct model-fix download consistent with HEXGRID.pbo.
    with ZipFile(output / 'GRZ-HEXGRID-model-fix.zip', 'w', ZIP_DEFLATED) as archive:
        archive.writestr('@GRZ_Local/Addons/HEXGRID.pbo', base_pbo)
        archive.write(ROOT / 'HEXGRID-REPAIR.md', 'INSTALL.md')
    with ZipFile(output / 'GRZ-Customs.zip') as archive:
        assert archive.testzip() is None
        assert archive.read('@GRZ_Customs/Addons/HEXGRID.pbo') == base_pbo
        for name, addon in extras.items():
            assert archive.read('@GRZ_Customs/Addons/' + name) == addon
    (output / ('GRZ-Customs-' + report['version'] + '.zip')).write_bytes((output / 'GRZ-Customs.zip').read_bytes())
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
