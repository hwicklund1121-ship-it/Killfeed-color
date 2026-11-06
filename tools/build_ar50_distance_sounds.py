"""Replace AR50 firing sound and derive distance layers from the user's clip."""
import argparse
import hashlib
import json
import re
import subprocess
import tempfile
import array
from pathlib import Path, PurePosixPath
from zipfile import ZipFile, ZIP_DEFLATED
from repair_hexgrid import section
root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('source_zip',type=Path)
args=parser.parse_args()
recording=root/'art/audio/ar50/UAV_Ak50-original.ogg'
with ZipFile(args.source_zip) as z:
    files={}
    for entry in z.infolist():
        path=PurePosixPath(entry.filename)
        assert not path.is_absolute() and '..' not in path.parts
        if not entry.is_dir():files[entry.filename]=z.read(entry)
original=dict(files)
config_name='ar50a1/ar50/config.cpp'
config=files[config_name].decode('utf-8-sig')
filters={
    'Close':'volume=0.82,alimiter=limit=0.95:level=false:latency=true',
    'Mid':'highpass=f=55,lowpass=f=5200,aecho=0.88:1:23|51|89:0.12|0.08|0.04,alimiter=limit=0.95:level=false:latency=true',
    'Far':'highpass=f=80,lowpass=f=2300,aecho=0.82:1:37|79|127|181:0.20|0.14|0.09|0.05,lowpass=f=2600,alimiter=limit=0.95:level=false:latency=true'
}
shaders={'Close':'ar50_Shot_closeShot_SoundShader','Mid':'ar50_Shot_midShot_SoundShader','Far':'ar50_Shot_distShot_SoundShader'}
report={}
with tempfile.TemporaryDirectory(prefix='grz-ar50-audio-') as directory:
    for layer,filter_chain in filters.items():
        output=Path(directory)/f'AR50_{layer}.ogg'
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(recording),'-ac','1','-ar','44100','-af',filter_chain,'-c:a','libvorbis','-q:a','7',str(output)],check=True)
        # Vorbis reconstruction can overshoot even a limited input transient.
        # Measure decoded float peaks and re-encode with additional headroom.
        headroom_gain = 1.0
        for attempt in range(4):
            decoded = subprocess.check_output(['ffmpeg','-v','error','-i',str(output),'-f','f32le','-ac','1','-'])
            samples = array.array('f')
            samples.frombytes(decoded)
            peak = max(abs(value) for value in samples)
            if peak <= 0.95:
                break
            headroom_gain *= 0.90 / peak
            subprocess.run(['ffmpeg','-v','error','-y','-i',str(recording),'-ac','1','-ar','44100','-af',filter_chain+f',volume={headroom_gain:.9f}','-c:a','libvorbis','-q:a','7',str(output)],check=True)
        assert peak <= 0.95, (layer, peak)
        info=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-show_format','-of','json',str(output)]))
        stream=info['streams'][0]
        assert stream['channels']==1 and stream['sample_rate']=='44100' and stream['codec_name']=='vorbis'
        subprocess.run(['ffmpeg','-v','error','-i',str(output),'-f','null','-'],check=True)
        name=f'ar50a1/ar50/data/AR50_{layer}.ogg'
        files[name]=output.read_bytes()
        start,_,_,end=section(config,shaders[layer])
        old_section=config[start:end]
        sample_path='UAVWEAPONS\\ar50a1\\ar50\\data\\AR50_'+layer
        updated,count=re.subn(r'samples\[\]\s*=\s*\{.*?\};',lambda _: 'samples[] = {{"'+sample_path+'",1}};',old_section,flags=re.S)
        assert count==1
        config=config[:start]+updated+config[end:]
        report[layer]=dict(filter=filter_chain,headroom_gain=headroom_gain,decoded_peak=peak,channels=1,sample_rate=44100,duration_seconds=float(info['format']['duration']),sha256=hashlib.sha256(files[name]).hexdigest())
files[config_name]=config.encode()
old_audio='ar50a1/ar50/data/AR50_Shot.ogg'
files.pop(old_audio)
# The three samples arrays are the only config changes; original gains/ranges stay intact.
restored=config
for layer,shader in shaders.items():
    a,_,_,b=section(restored,shader)
    oa,_,_,ob=section(original[config_name].decode('utf-8-sig'),shader)
    original_section=original[config_name].decode('utf-8-sig')[oa:ob]
    restored=restored[:a]+original_section+restored[b:]
assert restored==original[config_name].decode('utf-8-sig')
for name,data in original.items():
    if name not in [config_name,old_audio]:assert files[name]==data,name
notes='''AR50 Sounds — Made By Blake

Replace the complete ar50a1 folder inside your unpacked UAVWEAPONS pack with this ar50a1 folder.
Repack the weapon pack and update BOTH client and server, then restart both.
This replaces the original shot clip in the weapon folder; no additional sound PBO is needed.

AR50_Close: supplied UAV_Ak50_Shot recording downmixed to positional mono with peak headroom.
AR50_Mid: softened high frequencies and short, quiet reflections for distance.
AR50_Far: more high-frequency loss, reduced sharpness and a diffuse longer reflection tail.
All three are mono 44.1kHz Ogg Vorbis. Mid/far are processed versions of the supplied clip, not separate field recordings.
The existing close/mid/far curves, shader volumes and ranges (75m/600m/1200m) are retained.
Weapon models, materials, scripts, ammo, damage, recoil and other stats remain unchanged.

All clips decoded successfully; config references, layer encoding and package contents verified.
Actual distance blending and loudness still need testing in DayZ.
'''
(root/'AR50-SOUNDS.md').write_text(notes)
manifest=dict(source_sha256=hashlib.sha256(recording.read_bytes()).hexdigest(),layers=report,config_changes='three samples arrays only',old_shot_removed=True)
(root/'AR50-SOUNDS-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
output=root/'downloads/AR50-Sounds.zip'
with ZipFile(output,'w',ZIP_DEFLATED) as z:
    for name,data in files.items():z.writestr(name,data)
    z.writestr('INSTALL.txt',notes)
    z.writestr('MANIFEST.json',json.dumps(manifest,indent=2)+'\n')
with ZipFile(output) as z:
    assert z.testzip() is None
    for name,data in files.items():assert z.read(name)==data
print('Built',output,output.stat().st_size,'bytes')
for layer,info in report.items():print(layer,info['duration_seconds'],'seconds, mono 44100 Hz')
