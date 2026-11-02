"""Build a separate CF + VPP + GRZ server pack from the user's supplied PBOs.

Requires the original VPPAdminTools.pbo in packaging/admin-addons/@VPPAdminTools/Addons.
No replacement Workshop files are fetched. All addon bytes are preserved.
"""
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'packaging/admin-addons'
EXPECTED = {
    '@CF/Addons/gui.pbo', '@CF/Addons/legacy.pbo', '@CF/Addons/scripts.pbo',
    '@VPPAdminTools/Addons/assets.pbo', '@VPPAdminTools/Addons/VPPStaticItems.pbo',
    '@VPPAdminTools/Addons/VPPAdminTools.pbo',
}


def main():
    available = {p.relative_to(SOURCE).as_posix() for p in SOURCE.rglob('*.pbo')}
    missing = EXPECTED-available
    if missing:
        raise SystemExit('Cannot build complete admin pack; missing: '+', '.join(sorted(missing)))
    assert available == EXPECTED
    bootstrap = (b'powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Setup-VPPAdmin.ps1"\r\n'
                 b'if errorlevel 1 (\r\n pause\r\n exit /b 1\r\n)\r\n')
    allowed = EXPECTED | {'@CF/mod.cpp', '@VPPAdminTools/mod.cpp', 'ADMIN-SETUP.txt', 'Setup-VPPAdmin.ps1'}
    target = ROOT/'downloads/GRZ-local-server-with-admin.zip'
    with ZipFile(ROOT/'downloads/GRZ-local-server-ready.zip') as base, ZipFile(target,'w',ZIP_DEFLATED) as output:
        for name in base.namelist():
            if name == 'StartServer.bat':
                payload=base.read(name).replace(b'"-mod=@GRZ_Local"', b'"-mod=@CF;@VPPAdminTools;@GRZ_Local"')
                payload=payload.replace(b'DayZServer_x64.exe -config',bootstrap+b'DayZServer_x64.exe -config',1)
                output.writestr(name,payload)
            else:
                output.writestr(name,base.read(name))
        manifest={}
        for path in sorted(SOURCE.rglob('*')):
            if not path.is_file():
                continue
            name=path.relative_to(SOURCE).as_posix()
            if name not in allowed:
                continue
            payload=path.read_bytes()
            output.writestr(name,payload)
            if path.suffix.lower()=='.pbo':
                manifest[name]={'bytes':len(payload),'sha256':hashlib.sha256(payload).hexdigest()}
        output.writestr('ADMIN-PBO-manifest.json',json.dumps(manifest,indent=2)+'\n')
    with ZipFile(target) as check:
        assert check.testzip() is None
        assert b'"-mod=@CF;@VPPAdminTools;@GRZ_Local"' in check.read('StartServer.bat')
        assert b'Setup-VPPAdmin.ps1' in check.read('StartServer.bat')
        assert not any(name.endswith('admin-owner.local.txt') for name in check.namelist())
        for name in EXPECTED:
            assert check.read(name)==(SOURCE/name).read_bytes()
    print(f'Verified admin pack: {target.stat().st_size} bytes; all six admin/framework PBOs preserved.')


if __name__=='__main__':
    main()
