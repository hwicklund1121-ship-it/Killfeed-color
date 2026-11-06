class CfgPatches
{
 class GRZ_AnimatedGhillie
 {
  units[] = {"GRZ_Animated_Ghillie_Checker", "GRZ_Animated_Ghillie_Cross"};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Data", "DZ_Weapons_Supports", "GRZ_Loadout"};
 };
};
class CfgMods
{
 class GRZ_AnimatedGhillie
 {
  name = "GRZ Animated Checker Ghillie";
  author = "Blake";
  version = "0.2.0";
  type = "mod";
  dependencies[] = {"World"};
  class defs
  {
   class worldScriptModule
   {
    value = "";
    files[] = {"GRZ_AnimatedGhillie/scripts/4_World"};
   };
  };
 };
};
class CfgVehicles
{
 class GhillieAtt_ColorBase;
 class GRZ_Animated_Ghillie_Checker: GhillieAtt_ColorBase
 {
  scope = 2;
  displayName = "GRZ Animated Checker Wrap";
  descriptionShort = "Made by Blake. Black and white checks swap every 0.25 seconds.";
  GRZVipOnly = 1;
  hiddenSelectionsTextures[] = {"GRZ_AnimatedGhillie\data\checker_a_co.paa"};
 };
 class GRZ_Animated_Ghillie_Cross: GhillieAtt_ColorBase
 {
  scope = 2;
  displayName = "GRZ Animated Cross Wrap";
  descriptionShort = "Made by Blake. Upright and upside-down crosses alternate every 0.75 seconds.";
  GRZVipOnly = 1;
  hiddenSelectionsTextures[] = {"GRZ_AnimatedGhillie\data\cross_upright_co.paa"};
 };
};
