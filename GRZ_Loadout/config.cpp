class CfgPatches
{
 class GRZ_Loadout
 {
  units[] = {};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"GRZ_Audit","DZ_Data", "DZ_Scripts"};
 };
};
class CfgMods
{
 class GRZ_Loadout
 {
  dir = "GRZ_Loadout";
  name = "GRZ Loadout";
  author = "Blake";
  version = "0.4.6";
  type = "mod";
  inputs = "GRZ_Loadout/inputs.xml";
  dependencies[] = {"Game", "World", "Mission"};
  class defs
  {
   class gameScriptModule { value = ""; files[] = {"GRZ_Loadout/scripts/3_Game"}; };
   class worldScriptModule { value = ""; files[] = {"GRZ_Loadout/scripts/4_World"}; };
   class missionScriptModule { value = ""; files[] = {"GRZ_Loadout/scripts/5_Mission"}; };
  };
 };
};
