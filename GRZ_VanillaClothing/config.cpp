class CfgPatches
{
 class GRZ_VanillaClothing
 {
  units[] = {"GRZ_SurgicalGloves","GRZ_TShirt","GRZ_CowboyHat"};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Data","DZ_Characters","DZ_Characters_Gloves","DZ_Characters_Tops","DZ_Characters_Headgear"};
 };
};
class CfgVehicles
{
 class SurgicalGloves_White;
 class TShirt_Grey;
 class CowboyHat_Brown;
 class GRZ_SurgicalGloves: SurgicalGloves_White
 {
  scope = 2;
  displayName = "GRZ Surgical Gloves";
  descriptionShort = "made by blake";
  hiddenSelectionsTextures[] = {"GRZ_VanillaClothing\data\gloves_co.paa","GRZ_VanillaClothing\data\gloves_co.paa","GRZ_VanillaClothing\data\gloves_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_VanillaClothing\data\gloves.rvmat","GRZ_VanillaClothing\data\gloves.rvmat","GRZ_VanillaClothing\data\gloves.rvmat"};
 };
 class GRZ_TShirt: TShirt_Grey
 {
  scope = 2;
  displayName = "GRZ T-Shirt";
  descriptionShort = "made by blake";
  hiddenSelectionsTextures[] = {"GRZ_VanillaClothing\data\shirt_co.paa","GRZ_VanillaClothing\data\shirt_co.paa","GRZ_VanillaClothing\data\shirt_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_VanillaClothing\data\shirt.rvmat","GRZ_VanillaClothing\data\shirt.rvmat","GRZ_VanillaClothing\data\shirt.rvmat"};
 };
 class GRZ_CowboyHat: CowboyHat_Brown
 {
  scope = 2;
  displayName = "GRZ Cowboy Hat";
  descriptionShort = "made by blake";
  hiddenSelectionsTextures[] = {"GRZ_VanillaClothing\data\hat_co.paa","GRZ_VanillaClothing\data\hat_co.paa","GRZ_VanillaClothing\data\hat_co.paa"};
  hiddenSelectionsMaterials[] = {"GRZ_VanillaClothing\data\hat.rvmat","GRZ_VanillaClothing\data\hat.rvmat","GRZ_VanillaClothing\data\hat.rvmat"};
 };
};
