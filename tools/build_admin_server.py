"""Build a separate CF + VPP + GRZ server pack from the user's supplied PBOs.

Requires the original VPPAdminTools.pbo in packaging/admin-addons/@VPPAdminTools/Addons.
No replacement Workshop files are fetched. All addon bytes are preserved.
"""
import hashlib
import json
import shutil
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'packaging/admin-addons'
EXPECTED = {
    '@CF/Addons/gui.pbo', '@CF/Addons/legacy.pbo', '@CF/Addons/scripts.pbo',
    '@VPPAdminTools/Addons/assets.pbo', '@VPPAdminTools/Addons/VPPStaticItems.pbo',
    '@VPPAdminTools/Addons/VPPAdminTools.pbo',
}


def main():
    main_pbo = SOURCE/'@VPPAdminTools/Addons/VPPAdminTools.pbo'
    supplied_zip = SOURCE/'VPPAdminTools-upload.zip'
    if not main_pbo.exists() and supplied_zip.exists():
        with ZipFile(supplied_zip) as supplied:
            entry = supplied.getinfo('New folder/VPPAdminTools.pbo')
            assert entry.file_size < 128*1024*1024
            main_pbo.parent.mkdir(parents=True,exist_ok=True)
            with supplied.open(entry) as incoming, main_pbo.open('wb') as output:
                shutil.copyfileobj(incoming,output)
    available = {p.relative_to(SOURCE).as_posix() for p in SOURCE.rglob('*.pbo')}
    missing = EXPECTED-available
    if missing:
        raise SystemExit('Cannot build complete admin pack; missing: '+', '.join(sorted(missing)))
    assert available == EXPECTED
    bootstrap = (b'powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Setup-VPPAdmin.ps1"\r\n'
                 b'if errorlevel 1 (\r\n pause\r\n exit /b 1\r\n)\r\n')
    allowed = EXPECTED | {'@CF/mod.cpp', '@VPPAdminTools/mod.cpp', 'ADMIN-SETUP.txt', 'Setup-VPPAdmin.ps1'}
    # Optional VPP-dependent patch stays out of the GRZ-only pack.
    compat_name = '@GRZ_Local/Addons/GRZ_KillfeedVPPCompat.pbo'
    compat_data = build(ROOT/'GRZ_KillfeedVPPCompat')
    (ROOT/'downloads/GRZ_KillfeedVPPCompat.pbo').write_bytes(compat_data)
    target = ROOT/'downloads/GRZ-local-server-with-admin.zip'
    with ZipFile(ROOT/'downloads/GRZ-local-server-ready.zip') as base, ZipFile(target,'w',ZIP_DEFLATED) as output:
        for name in base.namelist():
            if name == 'StartServer.bat':
                payload=base.read(name).replace(b'"-mod=@GRZ_Local"', b'"-mod=@CF;@VPPAdminTools;@GRZ_Local"')
                payload=payload.replace(b'DayZServer_x64.exe -config',bootstrap+b'DayZServer_x64.exe -config',1)
                output.writestr(name,payload)
            elif name == 'PBO-manifest.json':
                grz_manifest = json.loads(base.read(name))
                grz_manifest['GRZ_KillfeedVPPCompat.pbo'] = {'bytes':len(compat_data), 'sha256':hashlib.sha256(compat_data).hexdigest()}
                output.writestr(name,json.dumps(grz_manifest,indent=2)+'\n')
            elif name == 'README.txt':
                intro = ('CF + VPP ADMIN + GRZ COMPLETE LOCAL SERVER PACK\n\n'
                         'Extract all contents beside DayZServer_x64.exe, including the new StartServer.bat.\n'
                         'Enable @CF, @VPPAdminTools and @GRZ_Local in your client launcher.\n'
                         'StartServer.bat asks for your SteamID64 locally on first launch and preserves other admins.\n'
                         'Includes GRZ_KillfeedVPPCompat.pbo so /grzvip is handled before VPP slash commands.\n'
                         'See ADMIN-SETUP.txt for VPP password/login instructions.\n\n')
                output.writestr(name,intro+base.read(name).decode())
            else:
                output.writestr(name,base.read(name))
        output.writestr(compat_name,compat_data)
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
        assert check.read(compat_name)==compat_data
        assert json.loads(check.read('PBO-manifest.json'))['GRZ_KillfeedVPPCompat.pbo']['sha256']==hashlib.sha256(compat_data).hexdigest()
        for name in EXPECTED:
            assert check.read(name)==(SOURCE/name).read_bytes()
    print(f'Verified admin pack: {target.stat().st_size} bytes; VPP command compatibility included; all six admin/framework PBOs preserved.')


if __name__=='__main__':
    main()
