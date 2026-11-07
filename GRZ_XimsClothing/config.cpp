class CfgPatches
{
 class GRZ_XimsClothing
 {
  units[] = {"XIMS_Sneakers","XIMS_JeanShorts"};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Data","DZ_Characters","DZ_Characters_Pants","DZ_Characters_Shoes"};
 };
};
class CfgVehicles
{
 class Sneakers_Black;
 class ShortJeans_Blue;
 class XIMS_Sneakers: Sneakers_Black
 {
  scope = 2;
  displayName = "XIMS Sneakers";
  descriptionShort = "Made By Blake";
  hiddenSelectionsTextures[] = {"GRZ_XimsClothing\data\sneakers_co.paa","GRZ_XimsClothing\data\sneakers_co.paa","GRZ_XimsClothing\data\sneakers_co.paa"};
  // Supplied export normal is flat; inherit the native shoe material/detail.
 };
 class XIMS_JeanShorts: ShortJeans_Blue
 {
  scope = 2;
  displayName = "XIMS Jean Shorts";
  descriptionShort = "Made By Blake";
  hiddenSelectionsTextures[] = {"GRZ_XimsClothing\data\shorts_co.paa","GRZ_XimsClothing\data\shorts_co.paa","GRZ_XimsClothing\data\shorts_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_XimsClothing\data\shorts.rvmat","GRZ_XimsClothing\data\shorts.rvmat","GRZ_XimsClothing\data\shorts.rvmat"};
 };
};
