class CfgPatches
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
 class GRZ_AR50_V2_Close_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Close",1}};
  volume = 0.25;
  range = 75;
  rangeCurve = "closeShotCurve";
 };
 class ar50_Shot_closeShot_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Close",1}};
 };
 class GRZ_AR50_V2_Mid_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Mid",1}};
  volume = 0.2;
  range = 600;
  rangeCurve = "midShotCurve";
 };
 class ar50_Shot_midShot_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Mid",1}};
 };
 class GRZ_AR50_V2_Far_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Far",1}};
  volume = 0.18;
  range = 1200;
  rangeCurve = "farShotCurve";
 };
 class ar50_Shot_distShot_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Far",1}};
 };
};
