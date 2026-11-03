class CfgPatches
{
    class GRZ_KillfeedVPPCompat
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        // Real patch name from the supplied VPPAdminTools.pbo. The handler
        // must wrap VPP's early-return slash command handler, not vice versa.
        requiredAddons[] = {"GRZ_Killfeed", "DZM_VPPAdminToolsScripts"};
    };
};
class CfgMods
{
    class GRZ_KillfeedVPPCompat
    {
        dir = "GRZ_KillfeedVPPCompat";
        name = "GRZ VIP VPP Compatibility";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"Mission"};
        class defs
        {
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_KillfeedVPPCompat/scripts/5_Mission"};
            };
        };
    };
};
