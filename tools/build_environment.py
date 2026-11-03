"""Package GRZ's server environment and dynamic PO-X suppression addon."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build
ROOT = Path(__file__).resolve().parents[1]

def main():
    data = build(ROOT/'GRZ_Environment')
    output = ROOT/'downloads'
    (output/'GRZ_Environment.pbo').write_bytes(data)
    target = output/'GRZ_Environment-mod.zip'
    with ZipFile(target, 'w', ZIP_DEFLATED) as z:
        z.writestr('@GRZ_Environment/Addons/GRZ_Environment.pbo', data)
        z.write(ROOT/'packaging/GRZ_Environment/mod.cpp','@GRZ_Environment/mod.cpp')
        z.write(ROOT/'ENVIRONMENT.md','@GRZ_Environment/INSTALL.md')
    with ZipFile(target) as z:
        assert z.testzip() is None
        assert z.read('@GRZ_Environment/Addons/GRZ_Environment.pbo') == data
    print('Built/verified environment PBO and ZIP; DayZ runtime testing required.')

if __name__ == '__main__':
    main()
