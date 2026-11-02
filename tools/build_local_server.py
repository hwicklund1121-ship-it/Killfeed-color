"""Package the local server setup, preserving the supplied PBO bytes."""
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'packaging/local-server'


def main():
    manifest = json.loads((SOURCE / 'PBO-manifest.json').read_text())
    pbos = list((SOURCE / '@GRZ_Local/Addons').glob('*.pbo'))
    assert len(pbos) == len(manifest) == 7
    for pbo in pbos:
        data = pbo.read_bytes()
        assert len(data) == manifest[pbo.name]['bytes'], pbo.name
        assert hashlib.sha256(data).hexdigest() == manifest[pbo.name]['sha256'], pbo.name
    for filename, include_mods in [('GRZ-local-server-ready.zip', True), ('GRZ-local-server-config.zip', False)]:
        target = ROOT / 'downloads' / filename
        with ZipFile(target, 'w', ZIP_DEFLATED) as archive:
            for path in sorted(SOURCE.rglob('*')):
                if not path.is_file() or (not include_mods and '@GRZ_Local' in path.parts):
                    continue
                payload = path.read_bytes()
                if path.suffix.lower() == '.bat':
                    payload = path.read_text().replace('\n', '\r\n').encode()
                archive.writestr(path.relative_to(SOURCE).as_posix(), payload)
        with ZipFile(target) as archive:
            assert archive.testzip() is None
            assert b'"-mod=@GRZ_Local"' in archive.read('StartServer.bat')
            if include_mods:
                for pbo in pbos:
                    assert archive.read('@GRZ_Local/Addons/' + pbo.name) == pbo.read_bytes()
        print(f'Verified {filename}: {target.stat().st_size} bytes.')


if __name__ == '__main__':
    main()
