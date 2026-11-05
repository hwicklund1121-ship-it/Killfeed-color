"""Swap the supplied Vamp recording into AR50 or NOREEN50 folders for repacking."""
import argparse
import hashlib
import json
import re
import subprocess
import tempfile
from pathlib import Path, PurePosixPath
from zipfile import ZIP_DEFLATED, ZipFile

from repair_hexgrid import resource_refs

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-zip', type=Path, required=True)
    parser.add_argument('--weapon', choices=['AR50', 'NOREEN50'], default='AR50')
    args = parser.parse_args()
    if args.weapon == 'AR50':
        folder, config_path = 'ar50a1', 'ar50a1/ar50/config.cpp'
        old_audio = 'ar50a1/ar50/data/ar50_Shot.ogg'
        new_audio = 'ar50a1/ar50/data/AR50_Shot.ogg'
    else:
        folder, config_path = 'NOREEN50', 'NOREEN50/ULR/config.cpp'
        old_audio = 'NOREEN50/ULR/data/Noreen_Shot.ogg'
        new_audio = 'NOREEN50/ULR/data/NOREEN50_Shot.ogg'
    title = args.weapon
    files = {}
    with ZipFile(args.source_zip) as archive:
        for entry in archive.infolist():
            path = PurePosixPath(entry.filename)
            assert not path.is_absolute() and '..' not in path.parts
            if entry.is_dir():
                continue
            assert path.parts[0] == folder
            assert entry.filename.lower() not in {key.lower() for key in files}
            files[entry.filename] = archive.read(entry)
    original = dict(files)
    old_path = 'UAVWEAPONS\\' + old_audio[:-4].replace('/', '\\')
    new_path = 'UAVWEAPONS\\' + new_audio[:-4].replace('/', '\\')
    config = files[config_path].decode('utf-8-sig')
    assert config.count(old_path) == 9
    updated = config.replace(old_path, new_path)
    assert updated.replace(new_path, old_path) == config
    files[config_path] = updated.encode()
    del files[old_audio]
    source_audio = ROOT / 'art/audio/ar50/Vamp_AR50.ogg'
    with tempfile.TemporaryDirectory(prefix='ar50-sounds-') as directory:
        converted = Path(directory) / 'AR50_Shot.ogg'
        subprocess.run(['ffmpeg', '-v', 'error', '-i', str(source_audio), '-ac', '1',
                        '-ar', '44100', '-c:a', 'libvorbis', '-q:a', '7',
                        '-map_metadata', '-1', str(converted)], check=True)
        info = json.loads(subprocess.check_output(['ffprobe', '-v', 'error', '-show_streams',
                                                   '-show_format', '-of', 'json', str(converted)]))
        stream = info['streams'][0]
        assert stream['codec_name'] == 'vorbis' and stream['channels'] == 1
        assert stream['sample_rate'] == '44100' and abs(float(info['format']['duration']) - 2.857) < 0.01
        subprocess.run(['ffmpeg', '-v', 'error', '-i', str(converted), '-f', 'null', '-'], check=True)
        files[new_audio] = converted.read_bytes()
    for name, data in original.items():
        if name not in [config_path, old_audio]:
            assert files[name] == data, name
    mounted = {'uavweapons/' + name.lower() for name in files}
    for name, data in files.items():
        if name.endswith(('.cpp', '.rvmat')):
            refs = re.findall(r'"([^"\r\n]+\.(?:p3d|paa|rvmat))"', data.decode('utf-8-sig'), re.I)
        elif name.endswith('.p3d'):
            refs = resource_refs(data)
        else:
            continue
        for ref in refs:
            ref = ref.replace('\\', '/').lstrip('/').lower()
            if ref.startswith('uavweapons/' + folder.lower() + '/'):
                assert ref in mounted, (name, ref)
    report = {'source_zip_sha256': hashlib.sha256(args.source_zip.read_bytes()).hexdigest(),
              'source_recording_sha256': hashlib.sha256(source_audio.read_bytes()).hexdigest(),
              'runtime_audio': new_audio,
              'replacement_audio_sha256': hashlib.sha256(files[new_audio]).hexdigest(),
              'replaced_sample_references': 9, 'sample_rate': 44100, 'channels': 1,
              'unchanged_files': sorted(name for name in original if name not in [config_path, old_audio])}
    manifest = ROOT / (title + '-SOUNDS-MANIFEST.json')
    manifest.write_text(json.dumps(report, indent=2) + '\n')
    target = ROOT / 'downloads' / (title + '-Sounds.zip')
    with ZipFile(target, 'w', ZIP_DEFLATED) as archive:
        for name, data in files.items():
            archive.writestr(name, data)
        archive.write(ROOT / (title + '-SOUNDS.md'), 'INSTALL.md')
        archive.write(manifest, 'MANIFEST.json')
    with ZipFile(target) as archive:
        assert archive.testzip() is None
        for name, data in files.items():
            assert archive.read(name) == data
        assert [name for name in archive.namelist() if name.lower().endswith('.ogg')] == [new_audio]
    print(title + ' recording replaced for all three distance shaders; mono Vorbis decodes; all other config values and original models/materials/textures/scripts unchanged. ZIP and resource checks passed; playback needs DayZ testing.')


if __name__ == '__main__':
    main()
