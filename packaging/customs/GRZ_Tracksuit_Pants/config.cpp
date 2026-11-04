class CfgPatches
{
    class GRZ_Customs_Tracksuit
    {
        units[] = {"GRZ_Tracksuit_Pants"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Characters","DZ_Characters_Pants"};
    };
};
class CfgVehicles
{
    class TrackSuitPants_ColorBase;
    class GRZ_Tracksuit_Pants: TrackSuitPants_ColorBase
    {
        scope = 2;
        displayName = "GRZ_Tracksuit_Pants";
        descriptionShort = "Made By Blake";
        hiddenSelectionsTextures[] = {"GRZ_Tracksuit_Pants\data\tracksuit_co.paa","GRZ_Tracksuit_Pants\data\tracksuit_co.paa","GRZ_Tracksuit_Pants\data\tracksuit_co.paa"};
        hiddenSelectionsMaterials[] = {"GRZ_Tracksuit_Pants\data\tracksuit.rvmat","GRZ_Tracksuit_Pants\data\tracksuit.rvmat","GRZ_Tracksuit_Pants\data\tracksuit.rvmat"};
        
    };
};
