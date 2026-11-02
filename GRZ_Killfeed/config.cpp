class CfgPatches
{
    class GRZ_Killfeed
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Killfeed
    {
        dir = "GRZ_Killfeed";
        name = "GRZ Killfeed";
        author = "GRZ";
        version = "0.1.14";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"GRZ_Killfeed/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Killfeed/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_Killfeed/scripts/5_Mission"};
            };
        };
    };
};
