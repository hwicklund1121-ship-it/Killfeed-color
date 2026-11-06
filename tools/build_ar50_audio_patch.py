"""Package the generated AR50 layers as a sound-only override for the combined installer."""
from pathlib import Path
from zipfile import ZipFile
import hashlib,json
from build_pbo import build
r=Path(__file__).resolve().parents[1];source=r/'GRZ_AR50Sounds';(source/'sounds').mkdir(parents=True,exist_ok=True)
(source/'$PBOPREFIX$').write_text('GRZ_AR50Sounds\n')
config='''class CfgPatches
{
 class GRZ_AR50Sounds
 {
  requiredAddons[] = {"AR50A1"};
  requiredVersion = 0.1;
  units[] = {};
  weapons[] = {};
 };
};
class CfgSoundShaders
{
'''
with ZipFile(r/'downloads/AR50-Sounds.zip') as z:
 for layer,shader in [('Close','closeShot'),('Mid','midShot'),('Far','distShot')]:
  data=z.read(f'ar50a1/ar50/data/AR50_{layer}.ogg');(source/f'sounds/AR50_{layer}.ogg').write_bytes(data)
  config+=' class ar50_Shot_'+shader+'_SoundShader\n {\n  samples[] = {{"GRZ_AR50Sounds\\sounds\\AR50_'+layer+'",1}};\n };\n'
config+='};\n';(source/'config.cpp').write_text(config)
data=build(source);(r/'downloads/GRZ_AR50Sounds.pbo').write_bytes(data)
print('Built sound-only AR50 override:',len(data),'bytes; original shader curves, volumes and gun stats retained.')
