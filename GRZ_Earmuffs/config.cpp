class CfgPatches
{
    class GRZ_Earmuffs
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Earmuffs
    {
        dir = "GRZ_Earmuffs";
        name = "GRZ Earmuffs";
        author = "GRZ";
        version = "0.1.2";
        type = "mod";
        inputs = "GRZ_Earmuffs/inputs.xml";
        dependencies[] = {"Mission"};
        class defs
        {
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_Earmuffs/scripts/5_Mission"};
            };
        };
    };
};
