class CfgPatches
{
 class GRZ_XimsCS5
 {
  units[] = {};
  weapons[] = {"GRZ_XIMS_CS5"};
  requiredVersion = 0.1;
  requiredAddons[] = {"CS5_Base"};
 };
};
class CfgWeapons
{
 class UAV_CS5_BLACK;
 class GRZ_XIMS_CS5: UAV_CS5_BLACK
 {
  scope = 2;
  displayName = "XIMS CS5";
  descriptionShort = "Made By Blake";
  GRZVipOnly = 1;
  hiddenSelectionsTextures[] = {
   "GRZ_XimsCS5\data\cs5_receiver_co.paa",
   "GRZ_XimsCS5\data\cs5_stock_co.paa",
   "GRZ_XimsCS5\data\cs5_barrel_co.paa",
   "GRZ_XimsCS5\data\suppressor_co.paa"
  };
 };
};
