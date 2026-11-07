"""Build independent GRZ transition tuning from explicitly reviewed vanilla paths.

This does not discover graph semantics or reproduce DayZ 1.27 automatically.
Both direction lists must be supplied after reviewing the native filename index.
"""
from pathlib import Path,PurePosixPath
import argparse,json,math,tempfile,hashlib
from zipfile import ZipFile,ZIP_DEFLATED
from build_pbo import build
ROOT=Path(__file__).resolve().parents[1]

def reviewed_data(index,drop,recover,drop_speed=2.0,recovery_speed=1.6):
    if index.get('Schema')!=1 or not isinstance(index.get('Animations'),list):
        raise ValueError('Unsupported native animation index')
    available={row['Path'].replace('\\','/').lower() for row in index['Animations']}
    def direction(clips):
        result=[]
        if not clips:raise ValueError('Both direction lists must contain reviewed native clips')
        for value in clips:
            path=value.replace('\\','/').lower()
            if not path.startswith('dz/anims/anm/player/moves/') or PurePosixPath(path).suffix!='.anm' or '..' in PurePosixPath(path).parts or path not in available:
                raise ValueError('Clip absent from native index or invalid: '+value)
            if path in result:raise ValueError('Duplicate transition clip: '+value)
            result.append(path)
        return result
    down,up=direction(drop),direction(recover)
    if set(down)&set(up):raise ValueError('One clip cannot be assigned both directions')
    for speed in [drop_speed,recovery_speed]:
        if not math.isfinite(speed) or not 1.0<=speed<=4.0:raise ValueError('Prototype speed request must be in [1.0, 4.0]')
    return dict(DropSpeed=drop_speed,RecoverySpeed=recovery_speed,Drop=down,Recover=up)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--index',required=True,type=Path)
    p.add_argument('--drop',action='append')
    p.add_argument('--preset',type=Path)
    p.add_argument('--recover',action='append')
    p.add_argument('--drop-speed',type=float,default=2.0)
    p.add_argument('--recovery-speed',type=float,default=1.6)
    p.add_argument('--output',type=Path,default=ROOT/'downloads/GRZ-InstaDrop-Prototype.zip')
    a=p.parse_args()
    if a.preset:
        if a.drop or a.recover:p.error('Use either a preset or explicit direction lists')
        preset=json.loads(a.preset.read_text())
        a.drop,a.recover=preset['Drop'],preset['Recover']
        a.drop_speed,a.recovery_speed=preset['DropSpeed'],preset['RecoverySpeed']
    data=reviewed_data(json.loads(a.index.read_text(encoding='utf-8-sig')),a.drop,a.recover,a.drop_speed,a.recovery_speed)
    with tempfile.TemporaryDirectory() as t:
        d=Path(t);(d/'$PBOPREFIX$').write_text('GRZ_InstaDropData')
        (d/'config.cpp').write_text('class CfgPatches { class GRZ_InstaDropData { units[]={}; weapons[]={}; requiredVersion=0.1; requiredAddons[]={"GRZ_InstaDrop"}; }; };\n')
        (d/'transitions.json').write_text(json.dumps(data,indent=2)+'\n')
        files={'GRZ_InstaDrop.pbo':build(ROOT/'GRZ_InstaDrop'),'GRZ_InstaDropData.pbo':build(d)}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with ZipFile(a.output,'w',ZIP_DEFLATED) as z:
        for name,payload in files.items():z.writestr(name,payload)
        z.writestr('MANIFEST.json',json.dumps({n:hashlib.sha256(b).hexdigest() for n,b in files.items()},indent=2))
        z.write(ROOT/'GRZ_InstaDrop/README.md','README.md')
        z.write(ROOT/'packaging/instadrop-update/Install-InstaDrop.ps1','Install-InstaDrop.ps1')
        z.writestr('TRANSITIONS.json',json.dumps(data,indent=2)+'\n')
    print('Built prototype; native animation direction and actual in-game motion still require verification:',a.output)

if __name__=='__main__':main()
