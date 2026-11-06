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
class CfgSoundShaders
{
 class ar50_Shot_closeShot_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Close",1}};
 };
 class ar50_Shot_midShot_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Mid",1}};
 };
 class ar50_Shot_distShot_SoundShader
 {
  samples[] = {{"GRZ_AR50Sounds\sounds\AR50_Far",1}};
 };
};
