"""Replace firing audio inside the user's current complete SFG folder."""
import argparse
import hashlib
import json
import re
import tempfile
from pathlib import Path, PurePosixPath
from zipfile import ZipFile, ZIP_DEFLATED

from build_pbo import build
from repair_hexgrid import read_pbo, resource_refs, section

ROOT = Path(__file__).resolve().parents[1]
PREFIX = r'UAVWEAPONS\SFG50\SFG'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-zip', type=Path, required=True)
    args = parser.parse_args()
    original = {}
    with ZipFile(args.source_zip) as archive:
        for entry in archive.infolist():
            path = PurePosixPath(entry.filename)
            assert not path.is_absolute() and '..' not in path.parts
            if entry.is_dir():
                continue
            assert path.parts[:2] == ('SFG50', 'SFG')
            name = '/'.join(path.parts[2:])
            assert name and name.lower() not in {key.lower() for key in original}
            original[name] = archive.read(entry)
    files = dict(original)
    config = files['config.cpp'].decode('utf-8-sig')
    mappings = [('close', 'SFG_Close'), ('mid', 'SFG_Mid'), ('dist', 'SFG_Far')]
    old_samples = []
    for prefix in ['sfg50', 'sfgs50']:
        for distance, audio_name in mappings:
            shader = prefix + '_' + distance + 'Shot_SoundShader'
            start, _, _, end = section(config, shader)
            old = config[start:end]
            pattern = r'\bsamples\[\]\s*=\s*\{[^;]+;'
            old_samples.extend(re.findall(pattern, old))
            replacement = 'samples[] = {{"' + PREFIX + '\\data\\' + audio_name + '",1}};'
            updated, count = re.subn(pattern, lambda match: replacement, old)
            assert count == 1
            config = config[:start] + updated + config[end:]
    files['config.cpp'] = config.encode()
    # Exactly six sample arrays change; all other config bytes are retained.
    strip_samples = lambda text: re.sub(r'\bsamples\[\]\s*=\s*\{[^;]+;', 'SAMPLES;', text)
    assert strip_samples(config) == strip_samples(original['config.cpp'].decode('utf-8-sig'))
    removed = ['data/sfg50_Shot.ogg', 'data/sfgs50_Shot.ogg']
    for path in removed:
        assert path in files
        del files[path]
    for _, audio_name in mappings:
        files['data/' + audio_name + '.ogg'] = (ROOT / 'SFG_Sounds/sounds' / (audio_name + '.ogg')).read_bytes()
    for name, content in files.items():
        if name in original and name != 'config.cpp':
            assert content == original[name], name
    mounted = {(PREFIX + '/' + name).replace('\\', '/').lower() for name in files}
    for name, content in files.items():
        if name.endswith(('.cpp', '.rvmat')):
            refs = re.findall(r'"([^"\r\n]+\.(?:p3d|paa|rvmat))"', content.decode('utf-8-sig'), re.I)
        elif name.endswith('.p3d'):
            refs = resource_refs(content)
        else:
            continue
        for ref in refs:
            ref = ref.replace('\\', '/').lstrip('/').lower()
            if ref.startswith('uavweapons/sfg50/sfg/'):
                assert ref in mounted, (name, ref)
    for path in re.findall(r'samples\[\]\s*=\s*\{\{"([^"]+)"', config):
        assert (path + '.ogg').replace('\\', '/').lower() in mounted
    with tempfile.TemporaryDirectory(prefix='grz-sfg-audio-replacement-') as directory:
        source = Path(directory)
        (source / '$PBOPREFIX$').write_text(PREFIX)
        for name, content in files.items():
            target = source / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(content)
        pbo = build(source)
    extension, packed = read_pbo(pbo)
    assert extension['prefix'] == PREFIX and packed == files
    assert hashlib.sha1(pbo[:-21]).digest() == pbo[-20:]
    output = ROOT / 'downloads'
    report = {'input_zip_sha256': hashlib.sha256(args.source_zip.read_bytes()).hexdigest(),
              'validation_pbo_sha256': hashlib.sha256(pbo).hexdigest(),
              'mount': PREFIX, 'replaced_shader_arrays': len(old_samples),
              'removed_old_audio': removed, 'new_audio': ['data/' + name + '.ogg' for _, name in mappings],
              'preserved_original_files': sorted(name for name in files if name in original and name != 'config.cpp')}
    (ROOT / 'SFG-REPLACEMENT-MANIFEST.json').write_text(json.dumps(report, indent=2) + '\n')
    with ZipFile(output / 'SFG50-SFG-Sounds.zip', 'w', ZIP_DEFLATED) as archive:
        for name, content in files.items():
            archive.writestr('SFG50/SFG/' + name, content)
        archive.write(ROOT / 'SFG-REPLACEMENT.md', 'INSTALL.md')
        archive.write(ROOT / 'SFG-REPLACEMENT-MANIFEST.json', 'MANIFEST.json')
    with ZipFile(output / 'SFG50-SFG-Sounds.zip') as archive:
        assert archive.testzip() is None
        assert {name for name in archive.namelist() if name.endswith('.ogg')} == {'SFG50/SFG/data/' + name + '.ogg' for _, name in mappings}
        for name, content in files.items():
            assert archive.read('SFG50/SFG/' + name) == content
    print('Replaced all six firing sample arrays and removed both old shot clips. Other config bytes, models, textures and scripts preserved. ZIP/PBO paths and checksums passed; DayZ playback still needs testing.')


if __name__ == '__main__':
    main()
