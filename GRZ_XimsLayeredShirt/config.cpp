class CfgPatches
{
 class GRZ_XimsLayeredShirt
 {
  units[] = {"XIMS_LayeredShirt"};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Data","Layered_Shirt"};
 };
};
class CfgVehicles
{
 class Layered_Shirt_Base;
 class XIMS_LayeredShirt: Layered_Shirt_Base
 {
  scope = 2;
  displayName = "XIMS Layered Shirt";
  descriptionShort = "Made By Blake";
  // Verified against the supplied Windstride Models/Layered_Shirt config.
  hiddenSelections[] = {"zbytek"};
  hiddenSelectionsTextures[] = {"GRZ_XimsLayeredShirt\data\shirt_co.paa","GRZ_XimsLayeredShirt\data\shirt_co.paa","GRZ_XimsLayeredShirt\data\shirt_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_XimsLayeredShirt\data\shirt.rvmat"};
 };
};
