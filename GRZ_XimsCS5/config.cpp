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
  // This model's compiled skin sections do not match the parent config.
  // The private model binds each painted texture directly on every visual LOD.
  model = "GRZ_XimsCS5\data\cs5_xims.p3d";
  hiddenSelections[] = {};
  hiddenSelectionsTextures[] = {};
  hiddenSelectionsMaterials[] = {};
 };
};
