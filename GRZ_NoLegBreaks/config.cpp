class CfgPatches
{
    class GRZ_NoLegBreaks
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_NoLegBreaks
    {
        dir = "GRZ_NoLegBreaks";
        name = "GRZ No Leg Breaks and Foot Cuts";
        author = "GRZ";
        version = "0.1.0";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_NoLegBreaks/scripts/4_World"};
            };
        };
    };
};
