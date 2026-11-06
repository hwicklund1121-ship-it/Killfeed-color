class CfgPatches
{
    class GRZ_Movement
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Movement
    {
        dir = "GRZ_Movement";
        name = "GRZ Movement";
        author = "GRZ";
        credits = "Ronjam - original movement; GRZ - integration. Supplied OPM reference also credits Operation Monarch / Echo.";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"GRZ_Movement/scripts/3_Game"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_Movement/scripts/5_Mission"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Movement/scripts/4_World"};
            };
        };
    };
};
