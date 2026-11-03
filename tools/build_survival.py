"""Build GRZ Survival and verify its PBO/ZIP payloads."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build

ROOT=Path(__file__).resolve().parents[1]


def main():
    source=ROOT/'GRZ_Survival'
    output=ROOT/'downloads';output.mkdir(exist_ok=True)
    data=build(source);pbo=output/'GRZ_Survival.pbo';pbo.write_bytes(data)
    archive_path=output/'GRZ_Survival-mod.zip'
    with ZipFile(archive_path,'w',ZIP_DEFLATED) as archive:
        archive.writestr('@GRZ_Survival/Addons/GRZ_Survival.pbo',data)
        archive.write(ROOT/'packaging/GRZ_Survival/mod.cpp','@GRZ_Survival/mod.cpp')
        archive.write(ROOT/'SURVIVAL.md','@GRZ_Survival/INSTALL.md')
    with ZipFile(archive_path) as archive:
        assert archive.testzip() is None
        assert archive.read('@GRZ_Survival/Addons/GRZ_Survival.pbo')==data
    text=(source/'scripts/4_World/GRZ_Survival/GRZSurvival.c').read_text()
    assert text.count('{')==text.count('}')
    assert 'SetAllowDamage' not in text and 'SetHealth' not in text
    print(f'Built/verified GRZ Survival: {len(data)} bytes. DayZ runtime test still required.')


if __name__=='__main__':
    main()
