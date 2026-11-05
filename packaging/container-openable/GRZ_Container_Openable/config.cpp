class CfgPatches
{
    class GRZ_Container_Openable
    {
        units[] = {"GRZ_Container_Openable"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Scripts","DZ_Structures","DZ_Structures_Industrial"};
    };
};
class CfgMods
{
    class GRZ_Container_Openable
    {
        dir = "GRZ_Container_Openable";
        name = "GRZ Openable Container";
        type = "mod";
        dependencies[] = {"World"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Container_Openable/scripts/4_World"};
            };
        };
    };
};
class CfgVehicles
{
    class HouseNoDestruct;
    class GRZ_Container_Openable: HouseNoDestruct
    {
        scope = 2;
        displayName = "GRZ_Container_Openable";
        descriptionShort = "Made By Blake";
        model = "GRZ_Container_Openable\model\container_openable.p3d";
        class Doors
        {
            class DoorsTwin1
            {
                displayName = "Container doors";
                component = "doorsTwin1";
                soundPos = "doorsTwin1_action";
                animPeriod = 1;
                initPhase = 0;
                initOpened = 0;
                soundOpen = "doorMetalTwinBigOpen";
                soundClose = "doorMetalTwinBigClose";
                soundLocked = "doorMetalTwinBigRattle";
                soundOpenABit = "doorMetalTwinBigOpenABit";
            };
        };
    };
};
