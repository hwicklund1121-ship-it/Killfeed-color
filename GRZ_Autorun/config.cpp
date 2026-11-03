class CfgPatches
{
    class GRZ_Autorun
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Autorun
    {
        dir = "GRZ_Autorun";
        name = "GUNRUNNERZ AUTORUN";
        author = "GRZ";
        version = "0.12";
        inputs = "GRZ_Autorun/inputs.xml";
        type = "mod";
        dependencies[] = {"World", "Mission"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Autorun/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_Autorun/scripts/5_Mission"};
            };
        };
    };
};
