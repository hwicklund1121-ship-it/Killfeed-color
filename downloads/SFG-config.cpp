// SFG standalone config: matching gun/suppressor paths and one root config.
////////////////////////////////////////////////////////////////////
//DeRap: config.bin
//Produced from mikero's Dos Tools Dll version 9.98
//https://mikero.bytex.digital/Downloads
//'now' is Sat Oct 24 17:15:53 2026 : 'file' last modified on Sat Mar 22 19:57:14 2025
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgPatches
{
	class sfg50
	{
		requiredAddons[] = {"DZ_Data","DZ_Characters_Vests","DZ_Scripts","DZ_Characters"};
		units[] = {};
		weapons[] = {"UAV_SFG50"};
	};

	class UAV_SFG_Suppressor
	{
		units[] = {"UAV_SFG_Sup"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgMods
{
	class sfg50
	{
		dir = "uav";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "uav";
		credits = "uav7";
		author = "uav7";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"World"};
		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"UAVWEAPONS/SFG50/SFG/scripts/4_World"};
			};
		};
	};
};
class Mode_Safe;
class Mode_SemiAuto;
class Mode_Burst;
class Mode_FullAuto;
class OpticsInfoRifle;
class CfgWeapons
{
	class BoltActionRifle_InnerMagazine_Base;
	class sfg50_Base: BoltActionRifle_InnerMagazine_Base
	{
		scope = 0;
		weight = 6000;
		model = "UAVWEAPONS\SFG50\SFG\data\SFG50.p3d";
		attachments[] = {"weaponWrap","weaponOpticsHunting","UAV_SFG_Suppressor"};
		hiddenSelections[] = {"camo"};
		ironsightsExcludingOptics[] = {"GrozaOptic","KobraOptic"};
		chamberableFrom[] = {"BC_Ammo_20mm"};
		simpleHiddenSelections[] = {"hide_barrel"};
		modes[] = {"Single"};
		itemSize[] = {10,2};
		absorbency = 0.1;
		repairableWithKits[] = {1};
		repairCosts[] = {25};
		PPDOFProperties[] = {1,0.5,50,180,4,10};
		WeaponLength = 2.18986;
		barrelArmor = 3.6;
		initSpeedMultiplier = 1;
		chamberedRound = "";
		chamberSize = 1;
		ejectType = 1;
		recoilModifier[] = {2,2,2};
		swayModifier[] = {1,1,1};
		dispersion = 0.0005;
		class Single: Mode_SemiAuto
		{
			soundSetShot[] = {"sfg50_Shot_SoundSet","sfg50_Shot_SoundSet","sfg50_Shot_SoundSet"};
			soundSetShotExt[] = {{"sfgs50_Shot_SoundSet","sfgs50_Shot_SoundSet","sfgs50_Shot_SoundSet"}};
			reloadTime = 1.0;
			recoil = "recoil_AKM";
			recoilProne = "recoil_AKM_prone";
		};
		class Particles
		{
			class OnFire
			{
				class MuzzleFlash
				{
					ignoreIfSuppressed = 1;
					overrideParticle = "weapon_shot_cz527_01";
				};
				class SmokeCloud
				{
					overrideParticle = "weapon_shot_winded_smoke";
				};
			};
			class OnOverheating
			{
				shotsToStartOverheating = 5;
				maxOverheatingValue = 20;
				overheatingDecayInterval = 1;
			};
			class OnBulletCasingEject
			{
				class ChamberSmokeRaise
				{
					overrideParticle = "weapon_shot_chamber_smoke_raise";
					overridePoint = "Nabojnicestart";
				};
			};
		};
		class OpticsInfo: OpticsInfoRifle
		{
			memoryPointCamera = "eye";
			modelOptics = "-";
			distanceZoomMin = 100;
			distanceZoomMax = 100;
			discreteDistance[] = {100,200,300,400,500,600,700,800,900,1000};
			discreteDistanceInitIndex = 0;
		};
		class AnimEvents
		{
			class SoundWeapon
			{
				class walkErc_L
				{
					soundSet = "Weapon_Movement_HRifle_walkErc_L_SoundSet";
					id = 1;
				};
				class walkErc_R
				{
					soundSet = "Weapon_Movement_HRifle_walkErc_R_SoundSet";
					id = 2;
				};
				class runErc_L
				{
					soundSet = "Weapon_Movement_HRifle_runErc_L_SoundSet";
					id = 3;
				};
				class runErc_R
				{
					soundSet = "Weapon_Movement_HRifle_runErc_R_SoundSet";
					id = 4;
				};
				class sprintErc_L
				{
					soundSet = "Weapon_Movement_HRifle_sprintErc_SoundSet";
					id = 5;
				};
				class sprintErc_R
				{
					soundSet = "Weapon_Movement_HRifle_sprintErc_SoundSet";
					id = 6;
				};
				class walkCro_L
				{
					soundSet = "Weapon_Movement_HRifle_walkCro_SoundSet";
					id = 21;
				};
				class walkCro_R
				{
					soundSet = "Weapon_Movement_HRifle_walkCro_SoundSet";
					id = 22;
				};
				class runCro_L
				{
					soundSet = "Weapon_Movement_HRifle_runCro_SoundSet";
					id = 23;
				};
				class runCro_R
				{
					soundSet = "Weapon_Movement_HRifle_runCro_SoundSet";
					id = 24;
				};
				class walkProne_L
				{
					soundSet = "Weapon_Movement_HRifle_walkCro_SoundSet";
					id = 31;
				};
				class walkProne_R
				{
					soundSet = "Weapon_Movement_HRifle_walkCro_SoundSet";
					id = 32;
				};
				class runProne_L
				{
					soundSet = "Weapon_Movement_HRifle_runCro_SoundSet";
					id = 33;
				};
				class runProne_R
				{
					soundSet = "Weapon_Movement_HRifle_runCro_SoundSet";
					id = 34;
				};
				class jumpErc_L
				{
					soundSet = "Weapon_Movement_HRifle_runErc_L_SoundSet";
					id = 35;
				};
				class jumpErc_R
				{
					soundSet = "Weapon_Movement_HRifle_runErc_L_SoundSet";
					id = 36;
				};
				class landFootErc_L
				{
					soundSet = "Weapon_Movement_HRifle_landFootErc_SoundSet";
					id = 37;
				};
				class landFootErc_R
				{
					soundSet = "Weapon_Movement_HRifle_landFootErc_SoundSet";
					id = 38;
				};
				class walkRasErc_L
				{
					soundSet = "Weapon_Movement_HRifle_walkRasErc_SoundSet";
					id = 51;
				};
				class walkRasErc_R
				{
					soundSet = "Weapon_Movement_HRifle_walkRasErc_SoundSet";
					id = 52;
				};
				class runRasErc_L
				{
					soundSet = "Weapon_Movement_HRifle_runRasErc_SoundSet";
					id = 53;
				};
				class runRasErc_R
				{
					soundSet = "Weapon_Movement_HRifle_runRasErc_SoundSet";
					id = 54;
				};
				class HandStep_L
				{
					soundSet = "Weapon_Movement_HRifle_walkErc_L_SoundSet";
					id = 61;
				};
				class HandStep_R
				{
					soundSet = "Weapon_Movement_HRifle_walkErc_L_SoundSet";
					id = 62;
				};
				class HandStep_Hard_L
				{
					soundSet = "Weapon_Movement_HRifle_runErc_L_SoundSet";
					id = 63;
				};
				class HandStep_Hard_R
				{
					soundSet = "Weapon_Movement_HRifle_runErc_L_SoundSet";
					id = 64;
				};
				class landFeetErc
				{
					soundSet = "Weapon_Movement_HRifle_landFeetErc_SoundSet";
					id = 100;
				};
				class Weapon_Movement_HRifle_Walk
				{
					soundSet = "Weapon_Movement_HRifle_walkErc_L_SoundSet";
					id = 101;
				};
				class Weapon_Movement_HRifle_Run
				{
					soundSet = "Weapon_Movement_HRifle_runErc_L_SoundSet";
					id = 102;
				};
				class Weapon_Movement_HRifle_Sprint
				{
					soundSet = "Weapon_Movement_HRifle_sprintErc_SoundSet";
					id = 103;
				};
				class Weapon_Movement_HRifle_Land
				{
					soundSet = "Weapon_Movement_HRifle_landFeetErc_SoundSet";
					id = 104;
				};
				class Char_Gestures_Hand_Grab_Rifle
				{
					soundSet = "Char_Gestures_Hand_Grab_FabricRifle_SoundSet";
					id = 892;
				};
			};
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 10000;
					healthLevels[] = {{1,{"UAVWEAPONS\SFG50\SFG\data\sfg50.rvmat"}},{0.7,{"UAVWEAPONS\SFG50\SFG\data\sfg50.rvmat"}},{0.5,{"UAVWEAPONS\SFG50\SFG\data\sfg50_damage.rvmat"}},{0.3,{"UAVWEAPONS\SFG50\SFG\data\sfg50_damage.rvmat"}},{0,{"UAVWEAPONS\SFG50\SFG\data\sfg50_destruct.rvmat"}}};
				};
			};
		};
	};
	class UAV_SFG50: sfg50_Base
	{
		scope = 2;
		displayName = "SFG-50";
		descriptionShort = "SFG-50 Chambered in 20MM";
		hiddenSelectionsTextures[] = {"UAVWEAPONS\SFG50\SFG\data\sfg50_co.paa"};
		hiddenSelectionsMaterials[] = {"UAVWEAPONS\SFG50\SFG\data\sfg50.rvmat"};
	};
};
class CfgSoundSets
{
	class Rifle_Shot_Base_SoundSet;
	class sfg50_Shot_SoundSet: Rifle_Shot_Base_SoundSet
	{
		soundShaders[] = {"sfg50_closeShot_SoundShader","sfg50_midShot_SoundShader","sfg50_distShot_SoundShader"};
	};
	class sfgs50_Shot_SoundSet: Rifle_Shot_Base_SoundSet
	{
		soundShaders[] = {"sfgs50_closeShot_SoundShader","sfgs50_midShot_SoundShader","sfgs50_distShot_SoundShader"};
	};
};
class CfgSoundShaders
{
	class sfg50_closeShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1}};
		volume = 0.25;
		range = 50;
		rangeCurve = "closeShotCurve";
	};
	class sfg50_midShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1}};
		volume = 0.2;
		range = 300;
		rangeCurve = "midShotCurve";
	};
	class sfg50_distShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfg50_Shot",1}};
		volume = 0.18;
		range = 600;
		rangeCurve = "farShotCurve";
	};
	class sfgs50_closeShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1}};
		volume = 0.5;
		range = 50;
		rangeCurve = "closeShotCurve";
	};
	class sfgs50_midShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1}};
		volume = 0.35;
		range = 200;
		rangeCurve = "midShotCurve";
	};
	class sfgs50_distShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1},{"UAVWEAPONS\SFG50\SFG\data\sfgs50_Shot",1}};
		volume = 0.2;
		range = 500;
		rangeCurve = "farShotCurve";
	};
};

class CfgVehicles
{
	class ItemSuppressor;
	class SFG_Suppressor_Base: ItemSuppressor
	{
		scope = 1;
		model = "UAVWEAPONS\SFG50\SFG\proxy\suppressor.p3d";
		rotationFlags = 17;
		reversed = 0;
		weight = 70;
		itemSize[] = {3,1};
		itemModelLength = 0.176;
		barrelArmor = 1500;
		lootCategory = "Attachments";
		hiddenSelections[] = {"camo"};
		inventorySlot[] = {"UAV_SFG_Suppressor"};
		simulation = "ItemSuppressor";
		noiseShootModifier = -0.9;
		recoilModifier[] = {0.8,0.8,0.8};
		swayModifier[] = {1,1,1};
		selectionFireAnim = "zasleh";
		soundIndex = 1;
		muzzlePos = "usti hlavne";
		class Particles
		{
			class OnFire
			{
				class MuzzleFlash
				{
					overrideParticle = "weapon_shot_fnx_02";
					onlyWithinHealthLabel[] = {0,3};
				};
				class Ruined
				{
					onlyWithinHealthLabel[] = {4,4};
					illuminateWorld = 1;
				};
			};
			class OnOverheating
			{
				shotsToStartOverheating = 2;
				maxOverheatingValue = 10;
				overheatingDecayInterval = 1;
				class SmokingBarrel
				{
					overrideParticle = "smoking_barrel_small";
					onlyWithinRainLimits[] = {0,0.2};
				};
				class SmokingBarrelHotSteamSmall
				{
					overrideParticle = "smoking_barrel_steam_small";
					positionOffset[] = {0,0.1,0};
					onlyWithinRainLimits[] = {0.2,1};
				};
			};
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 2500;
					healthLevels[] = {{1,{"UAVWEAPONS\SFG50\SFG\data\suppressor.rvmat"}},{0.7,{"UAVWEAPONS\SFG50\SFG\data\suppressor.rvmat"}},{0.5,{""}},{0.3,{""}},{0,{""}}};
				};
			};
		};
	};
	class UAV_SFG_Sup: SFG_Suppressor_Base
	{
		scope = 2;
		displayName = "SFG Suppressor";
		descriptionShort = "SFG Suppressor";
		hiddenSelectionsTextures[] = {"UAVWEAPONS\SFG50\SFG\data\suppressor.paa"};
		hiddenSelectionsMaterials[] = {"UAVWEAPONS\SFG50\SFG\data\suppressor.rvmat"};
	};
};

class CfgNonAIVehicles
{
	class ProxyAttachment;
	class Proxysuppressor: ProxyAttachment
	{
		scope = 2;
		inventorySlot = "UAV_SFG_Suppressor";
		model = "UAVWEAPONS\SFG50\SFG\proxy\suppressor.p3d";
	};
};

class CfgSlots
{
	class Slot_UAV_SFG_Suppressor
	{
		name = "UAV_SFG_Suppressor";
		displayName = "SFG Suppressor";
		ghostIcon = "supressor";
	};
};
