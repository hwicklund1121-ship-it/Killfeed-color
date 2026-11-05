class CfgPatches
{
 class GRZ_Customs_XimsKit
 {
  units[] = {"GRZ_XIMS_Tracksuit_Pants_BlackWhite","GRZ_XIMS_AirForces_White","GRZ_XIMS_Hexgrid_BlackWhite"};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Characters_Pants","PITSTOP_AirForce1","Hexgrid"};
 };
};
class CfgVehicles
{
 class TrackSuitPants_ColorBase;
 class PITSTOP_AirForce_Base;
 class HEXGRID_Base;
 class GRZ_XIMS_Tracksuit_Pants_BlackWhite: TrackSuitPants_ColorBase
 {
  scope = 2;
  displayName = "XIMS_Tracksuit pants";
  hiddenSelectionsTextures[] = {"GRZ_XimsKit\data\pants_co.paa","GRZ_XimsKit\data\pants_co.paa","GRZ_XimsKit\data\pants_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_XimsKit\data\pants.rvmat","GRZ_XimsKit\data\pants.rvmat","GRZ_XimsKit\data\pants.rvmat"};
 };
 class GRZ_XIMS_AirForces_White: PITSTOP_AirForce_Base
 {
  scope = 2;
  displayName = "XIMS AirForces";
  hiddenSelectionsTextures[] = {"GRZ_XimsKit\data\shoes_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_XimsKit\data\shoes.rvmat"};
 };
 class GRZ_XIMS_Hexgrid_BlackWhite: HEXGRID_Base
 {
  scope = 2;
  displayName = "XIMS Hexgrid";
  hiddenSelectionsTextures[] = {"GRZ_XimsKit\data\hexgrid_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_XimsKit\data\hexgrid.rvmat"};
 };
};
