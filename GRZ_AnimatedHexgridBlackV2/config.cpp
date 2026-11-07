class CfgPatches
{
 class GRZ_AnimatedHexgridBlackV2
 {
  units[] = {"GRZ_Hexgrid_Animated_Black_V2"};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"Hexgrid", "GRZ_Loadout"};
 };
};
class CfgMods
{
 class GRZ_AnimatedHexgridBlackV2
 {
  name = "GRZ Animated Hexgrid Black V2";
  author = "Blake";
  version = "0.1.3";
  type = "mod";
  dependencies[] = {"World"};
  class defs
  {
   class worldScriptModule
   {
    value = "";
    files[] = {"GRZ_AnimatedHexgridBlackV2/scripts/4_World"};
   };
  };
 };
};
class CfgVehicles
{
 class HEXGRID_Base;
 class GRZ_Hexgrid_Animated_Black_V2: HEXGRID_Base
 {
  scope = 2;
  GRZVipOnly = 1;
  displayName = "GRZ Animated Hexgrid Black V2";
  descriptionShort = "Made By Blake";
  hiddenSelectionsMaterials[] = {"GRZ_AnimatedHexgridBlackV2\data\flat_black.rvmat"};
  hiddenSelectionsTextures[] = {"GRZ_AnimatedHexgridBlackV2\data\frame_000.paa"};
  // All damage stages keep the flat material instead of restoring the original hex normal map.
  class DamageSystem
  {
   class GlobalHealth
   {
    class Health
    {
     hitpoints = 10000;
     transferToAttachmentsCoef = 0.5;
     healthLevels[] = {
      {1,{"GRZ_AnimatedHexgridBlackV2\data\flat_black.rvmat"}},
      {0.45,{"GRZ_AnimatedHexgridBlackV2\data\flat_black.rvmat"}},
      {0.35,{"GRZ_AnimatedHexgridBlackV2\data\flat_black.rvmat"}},
      {0.3,{"GRZ_AnimatedHexgridBlackV2\data\flat_black.rvmat"}},
      {0,{"GRZ_AnimatedHexgridBlackV2\data\flat_black.rvmat"}}
     };
    };
   };
  class GlobalArmor
			{
				class Projectile
				{
					class Health
					{
						damage = 0.045;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0.1;
					};
				};
				class Melee
				{
					class Health
					{
						damage = 0.05;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0.1;
					};
				};
				class Infected
				{
					class Health
					{
						damage = 0.05;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0.1;
					};
				};
				class FragGrenade
				{
					class Health
					{
						damage = 0.05;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0.1;
					};
				};
			};
};
 };
};
