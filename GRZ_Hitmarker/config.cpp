class CfgPatches
{
    class GRZ_Hitmarker
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class GRZ_Hitmarker
    {
        dir = "GRZ_Hitmarker";
        name = "GRZ Hitmarker";
        author = "GRZ";
        version = "0.1.9";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};
        class defs
        {
            class gameScriptModule { value = ""; files[] = {"GRZ_Hitmarker/scripts/3_Game"}; };
            class worldScriptModule { value = ""; files[] = {"GRZ_Hitmarker/scripts/4_World"}; };
            class missionScriptModule { value = ""; files[] = {"GRZ_Hitmarker/scripts/5_Mission"}; };
        };
    };
};
class CfgSoundShaders
{
    class GRZ_HitmarkerBody019_Shader
    {
        samples[] = {{"GRZ_Hitmarker\sounds\body_hit018", 1}};
        volume = 1;
        range = 100;
        rangeCurve[] = {{0, 1}, {100, 1}};
    };
    class GRZ_HitmarkerHead019_Shader
    {
        samples[] = {{"GRZ_Hitmarker\sounds\headshot018", 1}};
        volume = 1;
        range = 100;
        rangeCurve[] = {{0, 1}, {100, 1}};
    };
};
class CfgSoundSets
{
    class GRZ_HitmarkerBody019_SoundSet
    {
        soundShaders[] = {"GRZ_HitmarkerBody019_Shader"};
        volumeFactor = 1;
        frequencyFactor = 1;
        volumeRandomizer = 0;
        soundShadersLimit = 1;
        frequencyRandomizer = 0;
        sound3DProcessingType = "character3DProcessingType";
        volumeCurve = "characterAttenuationCurve";
        spatial = 1;
        doppler = 0;
        loop = 0;
    };
    class GRZ_HitmarkerHead019_SoundSet
    {
        soundShaders[] = {"GRZ_HitmarkerHead019_Shader"};
        volumeFactor = 1;
        frequencyFactor = 1;
        volumeRandomizer = 0;
        soundShadersLimit = 1;
        frequencyRandomizer = 0;
        sound3DProcessingType = "character3DProcessingType";
        volumeCurve = "characterAttenuationCurve";
        spatial = 1;
        doppler = 0;
        loop = 0;
    };
};
