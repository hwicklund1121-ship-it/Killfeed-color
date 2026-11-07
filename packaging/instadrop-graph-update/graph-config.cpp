class CfgPatches
{
 class GRZ_InstaDropGraph
 {
  units[] = {};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Characters", "GRZ_InstaDrop"};
 };
};
class CfgVehicles
{
 class Man;
 class SurvivorBase: Man
 {
  class enfAnimSys
  {
   graphName = "GRZ_InstaDropGraph/player_main.agr";
  };
 };
};
