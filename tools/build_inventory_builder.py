"""Package the Windows inventory-size PBO builder without runtime dependencies."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
ROOT=Path(__file__).resolve().parents[1]


def main():
    source=ROOT/'packaging/inventory-builder'
    target=ROOT/'downloads/GRZ-clothing-10x10-guns-10x2-builder.zip'
    with ZipFile(target,'w',ZIP_DEFLATED) as z:
        for path in sorted(source.iterdir()):
            if not path.is_file():continue
            payload=path.read_bytes()
            if path.suffix in ['.ps1','.cmd']:
                payload=payload.replace(b'\r\n',b'\n').replace(b'\n',b'\r\n')
            z.writestr(path.name,payload)
    with ZipFile(target) as z:
        assert z.testzip() is None
        assert set(z.namelist())=={'Build-Inventory-Mod.ps1','Build-Inventory-Mod.cmd','InventoryBuilder.cs','README.txt','INSTALL.txt'}
        assert z.read('InventoryBuilder.cs')==(source/'InventoryBuilder.cs').read_bytes()
    print(f'Built and ZIP-verified {target.name}: {target.stat().st_size} bytes.')


if __name__=='__main__':
    main()
