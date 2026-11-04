"""Repair the supplied HEXGRID PBO mount and root config.

Input is a user-supplied addon, not executable instructions. Models, textures and
scripts remain byte-identical. No game runtime or Enforce compiler is used.
"""
import argparse
import hashlib
import io
import json
import re
import struct
import tempfile
from pathlib import Path, PurePosixPath
from zipfile import ZipFile, ZIP_DEFLATED

from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]


def read_pbo(data):
    stream = io.BytesIO(data)

    def z():
        value = bytearray()
        while True:
            byte = stream.read(1)
            if not byte:
                raise ValueError('Truncated PBO string')
            if byte == b'\0':
                return value.decode('utf-8')
            value.extend(byte)

    extensions, entries = {}, []
    while True:
        name = z()
        method, original, reserved, timestamp, size = struct.unpack('<5I', stream.read(20))
        if not name and method == 0x56657273:
            while True:
                key = z()
                if not key:
                    break
                extensions[key] = z()
            continue
        if not name:
            break
        if method != 0:
            raise ValueError('Compressed PBO entries are not supported: ' + name)
        name = name.replace('\\', '/')
        path = PurePosixPath(name)
        if path.is_absolute() or '..' in path.parts or ':' in name:
            raise ValueError('Unsafe PBO entry: ' + name)
        entries.append((name, size))
    files = {}
    for name, size in entries:
        if name.casefold() in {p.casefold() for p in files}:
            raise ValueError('Duplicate PBO entry: ' + name)
        files[name] = stream.read(size)
        if len(files[name]) != size:
            raise ValueError('Truncated PBO payload: ' + name)
    return extensions, files


def section(text, name):
    start = re.search(r'\bclass\s+' + re.escape(name) + r'\s*\{', text, re.I)
    if not start:
        raise ValueError('Missing config section: ' + name)
    opening = text.index('{', start.start())
    tokens = re.finditer(r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*[\s\S]*?\*/|[{}]', text[opening:])
    depth = 0
    for token in tokens:
        if token[0] == '{':
            depth += 1
        elif token[0] == '}':
            depth -= 1
            if depth == 0:
                closing = opening + token.end()
                end = closing + (1 if text[closing:closing + 1] == ';' else 0)
                return start.start(), opening, closing - 1, end
    raise ValueError('Unbalanced config section: ' + name)


def resource_refs(data):
    strings = re.findall(rb'[\x20-\x7e]{5,}', data)
    return [s.decode('ascii') for s in strings if re.search(rb'\.(?:p3d|paa|rvmat)$', s, re.I)]


def verify_resources(prefix, files):
    mounted = {(prefix + '/' + name).replace('\\', '/').casefold() for name in files}
    checked = set()
    for name, data in files.items():
        if name.endswith('.p3d'):
            assert data[:4] == b'ODOL', name
            refs = resource_refs(data)
        elif name.endswith(('.cpp', '.rvmat')):
            refs = re.findall(r'"([^"\r\n]+\.(?:p3d|paa|rvmat))"', data.decode('utf-8-sig'), re.I)
        else:
            continue
        for ref in refs:
            normalized = ref.replace('\\', '/').lstrip('/').casefold()
            if normalized.startswith('pvppack/'):
                assert normalized in mounted, f'{name}: missing internal resource {ref}'
                checked.add(normalized)
    return len(checked)


def fix_hexgrid(files):
    result = dict(files)
    result.pop('texHeaders.bin', None)
    root = result['config.cpp'].decode('utf-8-sig')
    nested = result.pop('HEXGRID/config.cpp').decode('utf-8-sig')
    _, _, root_close, _ = section(root, 'CfgPatches')
    _, nested_open, nested_close, _ = section(nested, 'CfgPatches')
    root = root[:root_close] + nested[nested_open + 1:nested_close] + root[root_close:]
    start, _, _, end = section(nested, 'CfgVehicles')
    root += '\n// GRZ repair: item definitions are in the root config.\n' + nested[start:end] + '\n'
    result['config.cpp'] = root.encode()
    return 'PVPPack\\HEXGRID', result


def package(input_path, name, repair, output):
    original_data = input_path.read_bytes()
    original_extensions, original_files = read_pbo(original_data)
    prefix, files = repair(original_files)
    checked = verify_resources(prefix, files)
    # Verify original textures/audio are unchanged and ODOL geometry was not resized.
    for path, data in files.items():
        if path.endswith(('.paa', '.ogg', '.c')):
            assert data == original_files[path], path
        elif path.endswith('.p3d'):
            assert len(data) == len(original_files[path]) and data[:8] == original_files[path][:8], path
            assert data == original_files[path], path
    with tempfile.TemporaryDirectory(prefix='grz-model-repair-') as directory:
        source = Path(directory)
        (source / '$PBOPREFIX$').write_text(prefix + '\n')
        for path, data in files.items():
            target = source / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        pbo = build(source)
    assert hashlib.sha1(pbo[:-21]).digest() == pbo[-20:]
    extension, packed = read_pbo(pbo)
    assert extension['prefix'] == prefix and packed == files
    (output / (name + '.pbo')).write_bytes(pbo)
    return {'input_sha256': hashlib.sha256(original_data).hexdigest(),
            'original_prefix': original_extensions.get('prefix'), 'repaired_prefix': prefix,
            'output_sha256': hashlib.sha256(pbo).hexdigest(), 'bytes': len(pbo),
            'verified_internal_resources': checked,
            'assets': {path: hashlib.sha256(data).hexdigest() for path, data in files.items()
                       if path.endswith(('.p3d', '.paa', '.ogg'))}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--hexgrid', required=True, type=Path)
    args = parser.parse_args()
    output = ROOT / 'downloads'
    report = {'HEXGRID': package(args.hexgrid, 'HEXGRID', fix_hexgrid, output)}
    (ROOT / 'HEXGRID-REPAIR-MANIFEST.json').write_text(json.dumps(report, indent=2) + '\n')
    target = output / 'GRZ-HEXGRID-model-fix.zip'
    with ZipFile(target, 'w', ZIP_DEFLATED) as archive:
        for name in report:
            archive.write(output / (name + '.pbo'), '@GRZ_Local/Addons/' + name + '.pbo')
        archive.write(ROOT / 'HEXGRID-REPAIR.md', 'INSTALL.md')
    with ZipFile(target) as archive:
        assert archive.testzip() is None
        for name in report:
            assert archive.read('@GRZ_Local/Addons/' + name + '.pbo') == (output / (name + '.pbo')).read_bytes()
    print(json.dumps({name: {k: entry[k] for k in ['repaired_prefix', 'bytes', 'verified_internal_resources']}
                      for name, entry in report.items()}, indent=2))
    print('Textures/audio preserved, ODOL sizes/headers retained, internal model/material paths resolve, ZIP/PBO payloads match.')
    print('Ground, hands, inventory and worn rendering/compilation still require DayZ testing.')


if __name__ == '__main__':
    main()
