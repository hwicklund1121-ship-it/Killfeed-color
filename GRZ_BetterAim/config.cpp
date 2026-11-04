class CfgPatches
{
    class GRZ_BetterAim
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_BetterAim
    {
        dir = "GRZ_BetterAim";
        name = "GRZ Better Aim";
        author = "GRZ";
        version = "0.1.1";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_BetterAim/scripts/4_World"};
            };
        };
    };
};
