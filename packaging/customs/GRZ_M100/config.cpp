class CfgPatches
{
 class GRZ_Customs_M100
 {
  units[] = {};
  weapons[] = {"GRZ_RVK_M100"};
  requiredVersion = 0.1;
  requiredAddons[] = {"UAV_M100"};
 };
};
class CfgVehicles {};
class CfgWeapons
{
 class UAV_Snipex_M100_Tan;
 class GRZ_RVK_M100: UAV_Snipex_M100_Tan
 {
  scope = 2;
  displayName = "rvk M100";
  descriptionShort = "designed by blake";
  hiddenSelections[] = {"camo","camo2","camo3","camo4","camo5","camo6"};
  hiddenSelectionsTextures[] = {"GRZ_M100\data\barrel_co.paa","GRZ_M100\data\bolt_co.paa","GRZ_M100\data\grip_co.paa","GRZ_M100\data\handgrip_co.paa","GRZ_M100\data\muzzle_co.paa","GRZ_M100\data\stock_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_M100\data\barrel.rvmat","GRZ_M100\data\bolt.rvmat","GRZ_M100\data\grip.rvmat","GRZ_M100\data\handgrip.rvmat","GRZ_M100\data\muzzle.rvmat","GRZ_M100\data\stock.rvmat"};
  class DamageSystem
  {
   class GlobalHealth
   {
    class Health
    {
     hitpoints = 5000;
     healthLevels[] = {{1.0,{"GRZ_M100\data\barrel.rvmat","GRZ_M100\data\bolt.rvmat","GRZ_M100\data\grip.rvmat","GRZ_M100\data\handgrip.rvmat","GRZ_M100\data\muzzle.rvmat","GRZ_M100\data\stock.rvmat"}},{0.7,{"GRZ_M100\data\barrel.rvmat","GRZ_M100\data\bolt.rvmat","GRZ_M100\data\grip.rvmat","GRZ_M100\data\handgrip.rvmat","GRZ_M100\data\muzzle.rvmat","GRZ_M100\data\stock.rvmat"}},{0.5,{"GRZ_M100\data\barrel.rvmat","GRZ_M100\data\bolt.rvmat","GRZ_M100\data\grip.rvmat","GRZ_M100\data\handgrip.rvmat","GRZ_M100\data\muzzle.rvmat","GRZ_M100\data\stock.rvmat"}},{0.3,{"GRZ_M100\data\barrel.rvmat","GRZ_M100\data\bolt.rvmat","GRZ_M100\data\grip.rvmat","GRZ_M100\data\handgrip.rvmat","GRZ_M100\data\muzzle.rvmat","GRZ_M100\data\stock.rvmat"}},{0.0,{"GRZ_M100\data\barrel.rvmat","GRZ_M100\data\bolt.rvmat","GRZ_M100\data\grip.rvmat","GRZ_M100\data\handgrip.rvmat","GRZ_M100\data\muzzle.rvmat","GRZ_M100\data\stock.rvmat"}}};
    };
   };
  };
 };
};
