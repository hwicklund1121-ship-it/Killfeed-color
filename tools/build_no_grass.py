"""Pack the ChernarusPlus clutter-distance override; no Enforce scripts required."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build
ROOT = Path(__file__).resolve().parents[1]

def main():
    source = ROOT/'GRZ_NoGrass'
    data = build(source)
    output = ROOT/'downloads'
    (output/'GRZ_NoGrass.pbo').write_bytes(data)
    target = output/'GRZ_NoGrass-mod.zip'
    with ZipFile(target, 'w', ZIP_DEFLATED) as z:
        z.writestr('@GRZ_NoGrass/Addons/GRZ_NoGrass.pbo', data)
        z.write(ROOT/'packaging/GRZ_NoGrass/mod.cpp','@GRZ_NoGrass/mod.cpp')
        z.write(ROOT/'NO-GRASS.md','@GRZ_NoGrass/INSTALL.md')
    with ZipFile(target) as z:
        assert z.testzip() is None
        assert z.read('@GRZ_NoGrass/Addons/GRZ_NoGrass.pbo') == data
    print('Built and archive-verified GRZ No Grass (ChernarusPlus). In-game rendering test required.')

if __name__ == '__main__':
    main()
