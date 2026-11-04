class CfgPatches
{
    class GRZ_AntiHardscope
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_AntiHardscope
    {
        dir = "GRZ_AntiHardscope";
        name = "GRZ Anti Hardscope";
        author = "GRZ";
        version = "0.1.4";
        type = "mod";
        dependencies[] = {"Mission"};
        class defs
        {
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_AntiHardscope/scripts/5_Mission"};
            };
        };
    };
};
