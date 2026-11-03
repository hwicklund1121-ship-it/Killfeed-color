"""Build GRZ Earmuffs and check the input definitions and archive payload."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import xml.etree.ElementTree as ET
from build_pbo import build
ROOT = Path(__file__).resolve().parents[1]

def main():
    source = ROOT/'GRZ_Earmuffs'
    ET.parse(source/'inputs.xml')
    data = build(source)
    output = ROOT/'downloads'
    (output/'GRZ_Earmuffs.pbo').write_bytes(data)
    target = output/'GRZ_Earmuffs-mod.zip'
    with ZipFile(target, 'w', ZIP_DEFLATED) as z:
        z.writestr('@GRZ_Earmuffs/Addons/GRZ_Earmuffs.pbo', data)
        z.write(ROOT/'packaging/GRZ_Earmuffs/mod.cpp','@GRZ_Earmuffs/mod.cpp')
        z.write(ROOT/'EARMUFFS.md','@GRZ_Earmuffs/INSTALL.md')
    with ZipFile(target) as z:
        assert z.testzip() is None
        assert z.read('@GRZ_Earmuffs/Addons/GRZ_Earmuffs.pbo') == data
    print('Built/verified earmuffs ZIP and input XML. DayZ runtime test required.')

if __name__ == '__main__':
    main()
