class CfgPatches
{
    class GRZ_LoadingScreen
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_LoadingScreen
    {
        dir = "GRZ_LoadingScreen";
        name = "GRZ Loading Screen";
        author = "Blake";
        type = "mod";
        dependencies[] = {"Game"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"GRZ_LoadingScreen/scripts/3_Game"};
            };
        };
    };
};
