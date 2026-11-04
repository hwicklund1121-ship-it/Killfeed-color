class CfgPatches
{
    class GRZ_AimControl
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "GRZ_Hitscan", "GRZ_NoSway"};
    };
};
class CfgMods
{
    class GRZ_AimControl
    {
        dir = "GRZ_AimControl";
        name = "GRZ Aim Control Test";
        author = "GRZ";
        version = "0.1.2";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_AimControl/scripts/4_World"};
            };
        };
    };
};
