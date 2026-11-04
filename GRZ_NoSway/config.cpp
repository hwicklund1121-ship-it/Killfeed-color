class CfgPatches
{
    class GRZ_NoSway
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_NoSway
    {
        dir = "GRZ_NoSway";
        name = "GRZ No Sway";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_NoSway/scripts/4_World"};
            };
        };
    };
};
