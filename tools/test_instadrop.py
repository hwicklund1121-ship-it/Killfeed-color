"""Verify reviewed clip guards, actual package payloads and the PowerShell installer.
Does not compile Enforce or run DayZ. Pass the supplied native index to check paths.
"""
from pathlib import Path
from zipfile import ZipFile
import argparse,json,hashlib,os,subprocess,tempfile,sys
from build_instadrop import reviewed_data
from repair_hexgrid import read_pbo
ROOT=Path(__file__).resolve().parents[1]

def main():
 p=argparse.ArgumentParser();p.add_argument('--index',required=True,type=Path);p.add_argument('--pwsh',required=True);a=p.parse_args()
 index=json.loads(a.index.read_text(encoding='utf-8-sig'))
 preset=json.loads((ROOT/'packaging/instadrop-update/TRANSITIONS.json').read_text())
 data=reviewed_data(index,preset['Drop'],preset['Recover'],preset['DropSpeed'],preset['RecoverySpeed'])
 assert data['OverrideTransitionSpeeds'] is False and data['TuneDropInMovement'] is True and data['DropSpeed']==4.0
 assert len(data['Drop'])==6 and len(data['Recover'])==10
 assert all('/rifles/transitions/' in x and '_inj_' not in x for x in data['Drop']+data['Recover'])
 for down,up,speed in [([],data['Recover'],2),([data['Drop'][0]],[data['Drop'][0]],2),([data['Drop'][0]]*2,data['Recover'],2),(['dz/anims/anm/player/moves/missing.anm'],data['Recover'],2),(['dz/anims/anm/player/moves/../file.anm'],data['Recover'],2),(data['Drop'],data['Recover'],float('nan')),(data['Drop'],data['Recover'],4.1)]:
  try:reviewed_data(index,down,up,speed)
  except ValueError:pass
  else:raise AssertionError('Invalid list accepted')
 archive=ROOT/'downloads/GRZ-InstaDrop-Prototype.zip'
 with ZipFile(archive) as z:
  assert z.testzip() is None
  manifest=json.loads(z.read('MANIFEST.json'))
  for name,sha in manifest.items():assert hashlib.sha256(z.read(name)).hexdigest()==sha
  _,f=read_pbo(z.read('GRZ_InstaDropData.pbo'));assert json.loads(f['transitions.json'])==data
  _,f=read_pbo(z.read('GRZ_InstaDrop.pbo'));assert f['scripts/4_World/GRZInstaDrop.c']==(ROOT/'GRZ_InstaDrop/scripts/4_World/GRZInstaDrop.c').read_bytes()
  assert z.read('Install-InstaDrop.ps1')==(ROOT/'packaging/instadrop-update/Install-InstaDrop.ps1').read_bytes()
  with tempfile.TemporaryDirectory(prefix='grz-instadrop-') as t:
   d=Path(t);pkg=d/'package';z.extractall(pkg)
   for side,exe in [('server','DayZServer_x64.exe'),('game','DayZ_x64.exe')]:
    root=d/side;root.mkdir();(root/exe).touch();addons=root/'@GRZ_Local/Addons';addons.mkdir(parents=True)
    (addons/'GRZ_MovementSpeedData.pbo').write_bytes(b'KEEP GAIT DATA 1.50')
    (root/'saved-profile.json').write_text('KEEP PROFILE')
    for name in manifest:(addons/name).write_bytes(b'PREVIOUS '+name.encode())
   env=dict(os.environ,XDG_CACHE_HOME=str(d/'cache'),XDG_CONFIG_HOME=str(d/'config'),XDG_DATA_HOME=str(d/'data'))
   command=[a.pwsh,'-NoProfile','-File',str(pkg/'Install-InstaDrop.ps1'),'-ServerPath',str(d/'server'),'-GamePath',str(d/'game')]
   def run():return subprocess.run(command,env=env,text=True,capture_output=True)
   result=run();assert result.returncode==0,result.stdout+result.stderr
   for side in ['server','game']:
    addons=d/side/'@GRZ_Local/Addons'
    for name in manifest:assert (addons/name).read_bytes()==z.read(name)
    assert (addons/'GRZ_MovementSpeedData.pbo').read_bytes()==b'KEEP GAIT DATA 1.50'
    assert (d/side/'saved-profile.json').read_text()=='KEEP PROFILE'
   backups=list((d/'server').glob('GRZ-instadrop-backup-*'));assert len(backups)==1
   for side in ['server','client']:
    for name in manifest:assert (backups[0]/(side+'-'+name)).read_bytes()==b'PREVIOUS '+name.encode()
   # Bad package must abort before any existing destination changes.
   addon=pkg/'GRZ_InstaDrop.pbo';original=addon.read_bytes();addon.write_bytes(original+b'bad')
   result=run();assert result.returncode!=0 and 'hash mismatch' in result.stdout+result.stderr
   addon.write_bytes(original)
   # Visible conflicting addon must abort without deleting it or installing again.
   conflict=d/'game/@GRZ_Local/Addons/LUGUINSTADROPFIX.pbo';conflict.write_bytes(b'fixture')
   result=run();assert result.returncode!=0 and 'LUGU addon found' in result.stdout+result.stderr
   assert conflict.read_bytes()==b'fixture'
   for side in ['server','game']:
    for name in manifest:assert (d/side/'@GRZ_Local/Addons'/name).read_bytes()==z.read(name)
   assert len(list((d/'server').glob('GRZ-instadrop-backup-*')))==1
 print('16 paths verified against supplied index; guards, three PBO checksums, six installs, backups, untouched gait-data/profile sentinels, corruption and visible-conflict rejection passed. DayZ compilation/playback untested.')

if __name__=='__main__':main()
