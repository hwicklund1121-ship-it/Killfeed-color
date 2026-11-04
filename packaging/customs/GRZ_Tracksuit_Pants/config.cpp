class CfgPatches
{
    class GRZ_Customs_Tracksuit
    {
        units[] = {"GRZ_Tracksuit_Pants"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Characters_Pants"};
    };
};
class CfgVehicles
{
    class TrackSuitPants_Blue;
    class GRZ_Tracksuit_Pants: TrackSuitPants_Blue
    {
        scope = 2;
        displayName = "GRZ_Tracksuit_Pants";
        descriptionShort = "Made By Blake";
        hiddenSelectionsTextures[] = {"GRZ_Customs\Tracksuit\data\tracksuit_co.paa","GRZ_Customs\Tracksuit\data\tracksuit_co.paa","GRZ_Customs\Tracksuit\data\tracksuit_co.paa"};
        hiddenSelectionsMaterials[] = {"GRZ_Customs\Tracksuit\data\tracksuit.rvmat","GRZ_Customs\Tracksuit\data\tracksuit.rvmat","GRZ_Customs\Tracksuit\data\tracksuit.rvmat"};
        
    };
};
