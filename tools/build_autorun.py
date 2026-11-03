"""Build the supplied GRZ Autorun sources with the Alt-freelook update."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build

ROOT=Path(__file__).resolve().parents[1]


def main():
    output=ROOT/'downloads';output.mkdir(exist_ok=True)
    pbo=output/'GRZ_Autorun.pbo';pbo.write_bytes(build(ROOT/'GRZ_Autorun'))
    with ZipFile(output/'GRZ_Autorun-mod.zip','w',ZIP_DEFLATED) as z:
        z.write(pbo,'@GRZ_Autorun/Addons/GRZ_Autorun.pbo')
        z.write(ROOT/'packaging/GRZ_Autorun/mod.cpp','@GRZ_Autorun/mod.cpp')
        for name in ['README.md','REFERENCE-LICENSE.txt','REFERENCE-NOTICE.txt']:
            z.write(ROOT/'GRZ_Autorun'/name,'@GRZ_Autorun/'+name)
    print(f'Built autorun v0.12: {pbo.stat().st_size} bytes.')


if __name__=='__main__':
    main()
