class CfgPatches
{
    class GRZ_CleanArena
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Scripts","GRZ_DeathScreen"};
    };
};
class CfgMods
{
    class GRZ_CleanArena
    {
        dir = "GRZ_CleanArena";
        name = "GRZ Arena Cleanup";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_CleanArena/scripts/4_World"};
            };
        };
    };
};
