class CfgPatches { class GRZ_MapRotation { units[] = {}; weapons[] = {}; requiredVersion = 0.1; requiredAddons[] = {"DZ_Data","DZ_Scripts","GRZ_Loadout"}; }; };
class CfgMods { class GRZ_MapRotation { name = "GRZ Map Rotation"; version = "0.1.2"; author = "Blake"; type = "mod"; inputs = "GRZ_MapRotation/inputs.xml"; dependencies[] = {"Game","World","Mission"}; class defs { class gameScriptModule {value = ""; files[] = {"GRZ_MapRotation/scripts/3_Game"};}; class worldScriptModule {value = ""; files[] = {"GRZ_MapRotation/scripts/4_World"};}; class missionScriptModule {value = ""; files[] = {"GRZ_MapRotation/scripts/5_Mission"};};};};};
class CfgSoundShaders {
class GRZ_Rotation_vote_Shader { samples[] = {{"GRZ_MapRotation\sounds\vote",1}}; volume = 0.7; range = 10; rangeCurve[] = {{0,1},{10,1}}; };
class GRZ_Rotation_warning_Shader { samples[] = {{"GRZ_MapRotation\sounds\warning",1}}; volume = 0.7; range = 10; rangeCurve[] = {{0,1},{10,1}}; };
class GRZ_Rotation_change_Shader { samples[] = {{"GRZ_MapRotation\sounds\change",1}}; volume = 0.7; range = 10; rangeCurve[] = {{0,1},{10,1}}; };
};
class CfgSoundSets {
class GRZ_Rotation_Vote_SoundSet { soundShaders[] = {"GRZ_Rotation_vote_Shader"}; volumeFactor = 1; frequencyRandomizer = 0; spatial = 0; doppler = 0; loop = 0; };
class GRZ_Rotation_Warning_SoundSet { soundShaders[] = {"GRZ_Rotation_warning_Shader"}; volumeFactor = 1; frequencyRandomizer = 0; spatial = 0; doppler = 0; loop = 0; };
class GRZ_Rotation_Change_SoundSet { soundShaders[] = {"GRZ_Rotation_change_Shader"}; volumeFactor = 1; frequencyRandomizer = 0; spatial = 0; doppler = 0; loop = 0; };
};
