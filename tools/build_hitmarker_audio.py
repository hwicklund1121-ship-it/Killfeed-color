"""Generate original hit sounds, or import the user's supplied Rust headshot clip.

Default headshot audio is an original temporary sound, NOT Rust audio.
Import: python3 tools/build_hitmarker_audio.py --headshot /path/clip.wav
ffmpeg is needed for import; audio is encoded as mono 48 kHz PCM WAV and
normalized to -1 dB peak with a limiter. No protected game files are fetched.
"""
import argparse
import math
import random
import shutil
import struct
import subprocess
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOUNDS = ROOT / 'GRZ_Hitmarker/sounds'
STATUS = ROOT / 'GRZ_Hitmarker/scripts/3_Game/GRZ_Hitmarker/GRZHitmarkerAudioStatus.c'


def synthesize(path, head=False):
    rng = random.Random(741 if head else 318)
    rate = 48000
    duration = 0.15 if head else 0.09
    signal = []
    for i in range(round(rate * duration)):
        t = i / rate
        if head:
            sample = math.sin(2*math.pi*2500*t) * math.exp(-t*48) + .4*math.sin(2*math.pi*5100*t)*math.exp(-t*70)
        else:
            sample = .5*math.sin(2*math.pi*1700*t)*math.exp(-t*95)
        sample += rng.uniform(-1, 1) * .6 * math.exp(-t*180)
        sample *= min(1, i/24) * min(1, (duration-t)*1000)
        signal.append(sample)
    peak = max(abs(value) for value in signal)
    pcm = b''.join(struct.pack('<h', round(value/peak*.89*32767)) for value in signal)
    with wave.open(str(path), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(rate)
        out.writeframes(pcm)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--headshot', type=Path)
    args = parser.parse_args()
    SOUNDS.mkdir(exist_ok=True)
    synthesize(SOUNDS / 'body_hit.wav')
    imported = args.headshot is not None
    if imported:
        if not args.headshot.is_file():
            parser.error('Audio input file does not exist.')
        if not shutil.which('ffmpeg'):
            parser.error('Install ffmpeg to import the supplied clip.')
        subprocess.run(['ffmpeg','-v','error','-y','-i',str(args.headshot),'-vn','-ac','1','-ar','48000','-af','loudnorm=I=-14:TP=-1:LRA=7,alimiter=limit=0.891:level=false','-c:a','pcm_s16le',str(SOUNDS/'headshot.wav')],check=True)
    else:
        synthesize(SOUNDS/'headshot.wav',True)
    flag = 'true' if imported else 'false'
    STATUS.write_text('class GRZHitmarkerAudioStatus\n{\n    static const bool RUST_HEADSHOT_INSTALLED = '+flag+';\n}\n')
    print('Headshot audio: '+('user-supplied clip' if imported else 'original temporary sound; Rust clip not installed'))


if __name__ == '__main__':
    main()
