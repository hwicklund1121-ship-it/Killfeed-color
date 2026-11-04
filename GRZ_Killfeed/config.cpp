class CfgPatches
{
    class GRZ_Killfeed
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Killfeed
    {
        dir = "GRZ_Killfeed";
        name = "GRZ Killfeed";
        author = "GRZ";
        version = "0.1.23";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"GRZ_Killfeed/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"GRZ_Killfeed/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"GRZ_Killfeed/scripts/5_Mission"};
            };
        };
    };
};

class CfgSoundShaders
{
    class GRZ_Killstreak_Notification_Shader
    {
        samples[] = {{"GRZ_Killfeed\sounds\streak_notification", 1}};
        volume = 1;
        range = 10;
        rangeCurve[] = {{0, 1}, {10, 1}};
    };
};
class CfgSoundSets
{
    class GRZ_Killstreak_Notification_SoundSet
    {
        soundShaders[] = {"GRZ_Killstreak_Notification_Shader"};
        volumeFactor = 1;
        frequencyRandomizer = 0;
        spatial = 0;
        doppler = 0;
        loop = 0;
    };
};
