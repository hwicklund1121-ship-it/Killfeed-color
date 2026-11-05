"""Build the standalone GRZ Hitmarker PBO and installable ZIP."""
import argparse
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', required=True, type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    source = root/'GRZ_Hitmarker'
    output = args.output_dir.resolve()
    if output == source or source in output.parents:
        parser.error('Output must be outside the addon source.')
    output.mkdir(parents=True, exist_ok=True)
    pbo = output/'GRZ_Hitmarker.pbo'
    pbo.write_bytes(build(source))
    with ZipFile(output/'GRZ_Hitmarker-mod.zip','w',ZIP_DEFLATED) as z:
        z.write(pbo,'@GRZ_Hitmarker/Addons/GRZ_Hitmarker.pbo')
        z.write(root/'packaging/GRZ_Hitmarker/mod.cpp','@GRZ_Hitmarker/mod.cpp')
        z.write(root/'HITMARKER.md','@GRZ_Hitmarker/INSTALL.md')
    with ZipFile(output/'GRZ-Hitmarker-Audio-Fix.zip','w',ZIP_DEFLATED) as z:
        z.write(pbo,'GRZ_Hitmarker.pbo')
        z.writestr('INSTALL.txt', '''GRZ Hitmarker Audio 0.1.9

Stop DayZ and the server. Replace GRZ_Hitmarker.pbo in @GRZ_Local/Addons
on BOTH the game and server. Restart both. Keep only one active copy.

This version explicitly configures the native 3D processor and attenuation
curve, matching EffectSound.Play3D. The local emitter follows the camera.
It uses native backslash sample paths and the supplied Rust headshot clip.
Sound and volume remain under Options > Game > GRZ HITMARKER.

The previous 0.1.8 client log confirmed hits at Sound ON / volume 100 but
neither sample started. Version 0.1.9 logs the resolved processor/curve,
sample availability and decoded duration on a timeout.

API, audio-format and package checks passed. No in-game audibility test
has been performed. Unsigned development PBO.
''')
    print(f'Built {pbo.name}: {pbo.stat().st_size} bytes.')


if __name__ == '__main__':
    main()
