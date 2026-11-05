class CfgPatches
{
    class SFG_Sounds
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"sfg50"};
    };
};
class CfgSoundShaders
{
    class sfg50_closeShot_SoundShader
    {
        samples[] = {{"SFG_Sounds\sounds\SFG_Close",1}};
    };
    class sfg50_midShot_SoundShader
    {
        samples[] = {{"SFG_Sounds\sounds\SFG_Mid",1}};
    };
    class sfg50_distShot_SoundShader
    {
        samples[] = {{"SFG_Sounds\sounds\SFG_Far",1}};
    };
    class sfgs50_closeShot_SoundShader
    {
        samples[] = {{"SFG_Sounds\sounds\SFG_Close",1}};
    };
    class sfgs50_midShot_SoundShader
    {
        samples[] = {{"SFG_Sounds\sounds\SFG_Mid",1}};
    };
    class sfgs50_distShot_SoundShader
    {
        samples[] = {{"SFG_Sounds\sounds\SFG_Far",1}};
    };
};
