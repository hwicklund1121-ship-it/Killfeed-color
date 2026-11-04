class CfgPatches
{
    class GRZ_Hitscan
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Hitscan
    {
        dir = "GRZ_Hitscan";
        name = "GRZ Hitscan Test";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"Game", "World"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"GRZ_Hitscan/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Hitscan/scripts/4_World"};
            };
        };
    };
};
