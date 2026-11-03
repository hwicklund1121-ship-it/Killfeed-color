class CfgPatches
{
    class GRZ_SkyColors
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "GRZ_Hitmarker"};
    };
};
class CfgMods
{
    class GRZ_SkyColors
    {
        dir = "GRZ_SkyColors";
        name = "GRZ Sky colors";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"Game", "Mission"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"GRZ_SkyColors/scripts/3_Game"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_SkyColors/scripts/5_Mission"};
            };
        };
    };
};
