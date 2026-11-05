class CfgPatches
{
    class GRZ_DeathScreen
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_DeathScreen
    {
        dir = "GRZ_DeathScreen";
        name = "GRZ Death Screen";
        author = "Blake";
        version = "0.2.0";
        type = "mod";
        dependencies[] = {"World", "Mission"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_DeathScreen/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_DeathScreen/scripts/5_Mission"};
            };
        };
    };
};
