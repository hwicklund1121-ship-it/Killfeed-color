class CfgPatches
{
    class GRZ_CustomHud
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};

class CfgMods
{
    class GRZ_CustomHud
    {
        dir = "GRZ_CustomHud";
        name = "GRZ Custom Hud";
        author = "GRZ";
        version = "0.1.13";
        type = "mod";
        dependencies[] = {"Mission"};
        class defs
        {
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_CustomHud/scripts/5_Mission"};
            };
        };
    };
};
