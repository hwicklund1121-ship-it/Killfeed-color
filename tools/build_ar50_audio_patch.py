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
class Mode_SemiAuto;
class CfgWeapons
{
 class BoltActionRifle_InnerMagazine_Base;
 class AR50A1_Base: BoltActionRifle_InnerMagazine_Base
 {
  class SemiAuto: Mode_SemiAuto
  {
   soundSetShot[] = {"GRZ_AR50_V2_Shot_SoundSet", "GRZ_AR50_V2_Shot_SoundSet", "GRZ_AR50_V2_Shot_SoundSet"};
   reloadTime = 1.0;
   recoil = "recoil_AKM";
   recoilProne = "recoil_AKM_prone";
  };
 };
 class UAV_AR50A1: AR50A1_Base
 {
  class SemiAuto: Mode_SemiAuto
  {
   soundSetShot[] = {"GRZ_AR50_V2_Shot_SoundSet", "GRZ_AR50_V2_Shot_SoundSet", "GRZ_AR50_V2_Shot_SoundSet"};
   reloadTime = 1.0;
   recoil = "recoil_AKM";
   recoilProne = "recoil_AKM_prone";
  };
 };
};
class CfgSoundSets
{
 class Rifle_Shot_Base_SoundSet;
 class GRZ_AR50_V2_Shot_SoundSet: Rifle_Shot_Base_SoundSet
 {
  soundShaders[] = {"GRZ_AR50_V2_Close_SoundShader", "GRZ_AR50_V2_Mid_SoundShader", "GRZ_AR50_V2_Far_SoundShader"};
 };
};
class CfgSoundShaders
{
'''
with ZipFile(r/'downloads/AR50-Sounds.zip') as z:
 for layer,shader in [('Close','closeShot'),('Mid','midShot'),('Far','distShot')]:
  data=z.read(f'ar50a1/ar50/data/AR50_{layer}.ogg');(source/f'sounds/AR50_{layer}.ogg').write_bytes(data)
  volume, distance, curve={'Close':(0.25,75,'closeShotCurve'),'Mid':(0.2,600,'midShotCurve'),'Far':(0.18,1200,'farShotCurve')}[layer]
  config+=f' class GRZ_AR50_V2_{layer}_SoundShader\n {{\n  samples[] = {{{{"GRZ_AR50Sounds\\sounds\\AR50_{layer}",1}}}};\n  volume = {volume};\n  range = {distance};\n  rangeCurve = "{curve}";\n }};\n'
  # Preserve compatibility with other reskins still pointing at the original set.
  config+=' class ar50_Shot_'+shader+'_SoundShader\n {\n  samples[] = {{"GRZ_AR50Sounds\\sounds\\AR50_'+layer+'",1}};\n };\n'
config+='};\n';(source/'config.cpp').write_text(config)
data=build(source);(r/'downloads/GRZ_AR50Sounds.pbo').write_bytes(data)
print('Built sound-only AR50 override:',len(data),'bytes; original shader curves, volumes and gun stats retained.')
