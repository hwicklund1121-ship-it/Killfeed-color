"""Build an additive SFG firing-sound override from the three supplied clips."""
import hashlib
import json
import re
import subprocess
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile

from build_pbo import build
from repair_hexgrid import read_pbo

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'SFG_Sounds'
for name, runtime_name in [('ax50_sup', 'SFG_Close'), ('ax50_sup_mid', 'SFG_Mid'), ('ax50_sup_far', 'SFG_Far')]:
    target = source / 'sounds' / (runtime_name + '.ogg')
    subprocess.run(['ffmpeg', '-y', '-v', 'error', '-i',
                    str(ROOT / 'art/audio/sfg-ax50' / (name + '.ogg')),
                    '-ac', '1', '-ar', '44100', '-c:a', 'libvorbis', '-q:a', '7',
                    '-map_metadata', '-1', str(target)], check=True)
    info = json.loads(subprocess.check_output(['ffprobe', '-v', 'error', '-show_streams',
                                              '-show_format', '-of', 'json', str(target)]))
    stream = info['streams'][0]
    assert stream['codec_name'] == 'vorbis' and stream['channels'] == 1
    assert stream['sample_rate'] == '44100' and 0.8 < float(info['format']['duration']) < 1
    subprocess.run(['ffmpeg', '-v', 'error', '-i', str(target), '-f', 'null', '-'], check=True)
data = build(source)
prefix, files = read_pbo(data)
assert prefix['prefix'] == 'SFG_Sounds'
assert hashlib.sha1(data[:-21]).digest() == data[-20:]
config = files['config.cpp'].decode()
assert 'requiredAddons[] = {"sfg50"}' in config
assert config.count('class sfgs50_') == 3
assert config.count('class sfg50_') == 3
assert 'CfgWeapons' not in config and 'CfgAmmo' not in config
for path in re.findall(r'"(SFG_Sounds[^"\r\n]+)"', config):
    relative = path.replace('\\', '/').split('/', 1)[1] + '.ogg'
    assert relative in files, relative
for name, content in files.items():
    assert content == (source / name).read_bytes()
output = ROOT / 'downloads'
(output / 'SFG_Sounds.pbo').write_bytes(data)
with ZipFile(output / 'SFG-Sounds.zip', 'w', ZIP_DEFLATED) as archive:
    archive.writestr('@GRZ_Local/Addons/SFG_Sounds.pbo', data)
    archive.write(ROOT / 'SFG-AX50-AUDIO.md', 'INSTALL.md')
with ZipFile(output / 'SFG-Sounds.zip') as archive:
    assert archive.testzip() is None
    assert archive.read('@GRZ_Local/Addons/SFG_Sounds.pbo') == data
print('Verified three mono Vorbis clips, six normal/suppressed shader mappings, PBO checksum/contents and ZIP. DayZ playback remains untested.')
