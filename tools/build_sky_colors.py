"""Pack GRZ Sky colors from the checked-in native-format lighting presets."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build
ROOT = Path(__file__).resolve().parents[1]

def main():
    output = ROOT/'downloads'
    data = build(ROOT/'GRZ_SkyColors')
    (output/'GRZ_SkyColors.pbo').write_bytes(data)
    with ZipFile(output/'GRZ_SkyColors-mod.zip','w',ZIP_DEFLATED) as z:
        z.writestr('@GRZ_SkyColors/Addons/GRZ_SkyColors.pbo',data)
        z.write(ROOT/'packaging/GRZ_SkyColors/mod.cpp','@GRZ_SkyColors/mod.cpp')
        z.write(ROOT/'SKY-COLORS.md','@GRZ_SkyColors/INSTALL.md')
    print(f'Built GRZ Sky colors PBO: {len(data)} bytes.')

if __name__ == '__main__':
    main()
