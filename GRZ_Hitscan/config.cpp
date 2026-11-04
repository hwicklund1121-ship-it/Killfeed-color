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
        name = "GRZ Native Damage Compatibility";
        author = "GRZ";
        version = "0.1.4";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Hitscan/scripts/4_World"};
            };
        };
    };
};
