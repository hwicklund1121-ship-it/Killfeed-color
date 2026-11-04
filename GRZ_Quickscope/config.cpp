class CfgPatches
{
    class GRZ_Quickscope
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "GRZ_Hitscan", "GRZ_NoSway"};
    };
};
class CfgMods
{
    class GRZ_Quickscope
    {
        dir = "GRZ_Quickscope";
        name = "GRZ Quickscope Test";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Quickscope/scripts/4_World"};
            };
        };
    };
};
