class CfgPatches
{
    class GRZ_Survival
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Survival
    {
        dir = "GRZ_Survival";
        name = "GRZ Survival";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Survival/scripts/4_World"};
            };
        };
    };
};
