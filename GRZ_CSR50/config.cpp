class CfgPatches
{
 class GRZ_CSR50
 {
  units[] = {};
  weapons[] = {"GRZ_CSR50"};
  requiredVersion = 0.1;
  requiredAddons[] = {"CSR50_Base"};
 };
};
class CfgWeapons
{
 class UAV_CSR50;
 class GRZ_CSR50: UAV_CSR50
 {
  scope = 2;
  displayName = "GRZ CSR50";
  descriptionShort = "Made By Blake";
  GRZVipOnly = 1;
  // Direct bindings keep the complete paint on every native visual LOD.
  model = "GRZ_CSR50\data\csr50_grz.p3d";
  hiddenSelections[] = {};
  hiddenSelectionsTextures[] = {};
  hiddenSelectionsMaterials[] = {};
  // Preserve the parent's 10000 health points and keep the custom material
  // through all condition states instead of reverting to the original skin.
  class DamageSystem
  {
   class GlobalHealth
   {
    class Health
    {
     hitpoints = 10000;
     healthLevels[] =
     {
      {1.0,{"GRZ_CSR50\data\csr50_custom_part1.rvmat"}},
      {0.7,{"GRZ_CSR50\data\csr50_custom_part1.rvmat"}},
      {0.5,{"GRZ_CSR50\data\csr50_custom_part1.rvmat"}},
      {0.3,{"GRZ_CSR50\data\csr50_custom_part1.rvmat"}},
      {0.0,{"GRZ_CSR50\data\csr50_custom_part1.rvmat"}}
     };
    };
   };
  };
 };
};
