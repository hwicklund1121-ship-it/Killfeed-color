class CfgPatches
{
    class GRZ_NoGrass
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Worlds_Chernarusplus_World"};
    };
};
class CfgWorlds
{
    class CAWorld;
    class ChernarusPlus: CAWorld
    {
        // Disable rendered terrain clutter without deleting world objects.
        clutterDist = 0;
    };
};
