class CfgPatches
{
    class GRZ_Environment
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Environment
    {
        dir = "GRZ_Environment";
        name = "GRZ Environment";
        author = "GRZ";
        version = "0.1.2";
        type = "mod";
        dependencies[] = {"World", "Mission"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Environment/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_Environment/scripts/5_Mission"};
            };
        };
    };
};
