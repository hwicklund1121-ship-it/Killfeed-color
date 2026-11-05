////////////////////////////////////////////////////////////////////
//DeRap: M100\M100\config.bin
//Produced from mikero's Dos Tools Dll version 10.21
//https://mikero.bytex.digital/Downloads
//'now' is Wed Nov 04 03:50:01 2026 : 'file' last modified on Sat Oct 24 21:26:41 2026
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgPatches
{
	class UAV_M100
	{
		units[] = {};
		weapons[] = {"UAV_Snipex_M100_Tan"};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data","DZ_Weapons_Firearms"};
	};
};
class Mode_Safe;
class Mode_SemiAuto;
class Mode_Single;
class Mode_Burst;
class Mode_FullAuto;
class OpticsInfoRifle;
class cfgWeapons
{
	class BoltActionRifle_InnerMagazine_Base;
	class UAV_Snipex_M100_Base: BoltActionRifle_InnerMagazine_Base
	{
		scope = 0;
		displayName = "rvk M100";
		descriptionShort = "designed by blake";
		model = "UAVWEAPONS\M100\M100\data\M100.p3d";
		attachments[] = {"weaponWrap","weaponOpticsHunting","weaponOptics"};
		canReceiveHuntingOptic = 1;
		itemSize[] = {10,2};
		weight = 16500;
		DisplayMagazine = 0;
		repairableWithKits[] = {1};
		repairCosts[] = {25.0};
		PPDOFProperties[] = {1,0.1,20.0,200.0,10.0,10.0};
		WeaponLength = 1.505;
		barrelArmor = 0.8;
		initSpeedMultiplier = 1.3;
		chamberSize = 5;
		chamberedRound = "";
		chamberableFrom[] = {"UAV_Ammo_VULCAN"};
		magazines[] = {};
		magazineSwitchTime = 0.38;
		ejectType = 0;
		recoilModifier[] = {1,2,1};
		swayModifier[] = {1,1,1};
		simpleHiddenSelections[] = {"hide_barrel"};
		hiddenSelections[] = {"camo","camo2","camo3","camo4","camo5","camo6"};
		class NoiseShoot
		{
			strength = 100;
			type = "shot";
		};
		modes[] = {"Single"};
		class Single: Mode_Single
		{
			soundSetShot[] = {"UAV_SnipexM100_Shot_SoundSet","UAV_SnipexM100_Shot_SoundSet","UAV_SnipexM100_Shot_SoundSet"};
			reloadTime = 1;
			dispersion = 0.00075;
			magazineSlot = "magazine";
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 5000;
					healthLevels[] = {{1.0,{"UAVWEAPONS\M100\M100\data\barrel.rvmat","UAVWEAPONS\M100\M100\data\bolt.rvmat","UAVWEAPONS\M100\M100\data\grip.rvmat","UAVWEAPONS\M100\M100\data\handgrip.rvmat","UAVWEAPONS\M100\M100\data\muzzle.rvmat","UAVWEAPONS\M100\M100\data\stock.rvmat"}},{0.7,{"UAVWEAPONS\M100\M100\data\barrel.rvmat","UAVWEAPONS\M100\M100\data\bolt.rvmat","UAVWEAPONS\M100\M100\data\grip.rvmat","UAVWEAPONS\M100\M100\data\handgrip.rvmat","UAVWEAPONS\M100\M100\data\muzzle.rvmat","UAVWEAPONS\M100\M100\data\stock.rvmat"}},{0.5,{""}},{0.3,{""}},{0.0,{""}}};
				};
			};
		};
		class OpticsInfo: OpticsInfoRifle
		{
			memoryPointCamera = "eye";
			modelOptics = "-";
			distanceZoomMin = 100;
			distanceZoomMax = 100;
			discreteDistance[] = {100};
			discreteDistanceInitIndex = 0;
		};
		class InventorySlotsOffsets
		{
			class Shoulder
			{
				position[] = {0.02,-0.02,-0.02};
				orientation[] = {0,-30,0};
			};
			class Melee
			{
				position[] = {0.02,0,0.01};
				orientation[] = {0,30,0};
			};
		};
		class Particles
		{
			class OnFire
			{
				class MuzzleFlash
				{
					overrideParticle = "weapon_shot_cz527_01";
					ignoreIfSuppressed = 1;
					illuminateWorld = 1;
					positionOffset[] = {-0.05,0,0};
				};
			};
			class OnOverheating
			{
				maxOverheatingValue = 1;
				shotsToStartOverheating = 1;
				overheatingDecayInterval = 0.5;
				class SmokingBarrel1
				{
					overrideParticle = "smoking_barrel_small";
					onlyWithinOverheatLimits[] = {0.1,1};
					positionOffset[] = {0.0,0,0};
					onlyWithinRainLimits[] = {0,0.2};
				};
			};
			class OnBulletCasingEject
			{
				class ChamberSmokeRaise
				{
					overrideParticle = "weapon_shot_chamber_smoke";
					overridePoint = "Nabojnicestart";
				};
			};
		};
	};
	class UAV_Snipex_M100_Tan: UAV_Snipex_M100_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"UAVWEAPONS\M100\M100\data\barrel_co.paa","UAVWEAPONS\M100\M100\data\bolt_co.paa","UAVWEAPONS\M100\M100\data\grip_co.paa","UAVWEAPONS\M100\M100\data\handgrip_co.paa","UAVWEAPONS\M100\M100\data\muzzle_co.paa","UAVWEAPONS\M100\M100\data\stock_co.paa"};
		hiddenSelectionsMaterials[] = {"UAVWEAPONS\M100\M100\data\barrel.rvmat","UAVWEAPONS\M100\M100\data\bolt.rvmat","UAVWEAPONS\M100\M100\data\grip.rvmat","UAVWEAPONS\M100\M100\data\handgrip.rvmat","UAVWEAPONS\M100\M100\data\muzzle.rvmat","UAVWEAPONS\M100\M100\data\stock.rvmat"};
	};
};
class CfgSoundShaders
{
	class base_closeShot_SoundShader;
	class base_midShot_SoundShader;
	class base_distShot_SoundShader;
	class base_tailDistant_SoundShader;
	class base_HomeSilenced_closeShot_SoundShader;
	class base_HomeSilenced_midShot_SoundShader;
	class base_HomeSilenced_distShot_SoundShader;
	class UAV_SnipexM100_closeShot_SoundShader: base_closeShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\M100\M100\data\M100_Shot",1}};
		volume = 0.9;
	};
	class UAV_SnipexM100_midShot_SoundShader: base_midShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\M100\M100\data\M100_Shot",1}};
		volume = 0.70794576;
	};
	class UAV_SnipexM100_distShot_SoundShader: base_distShot_SoundShader
	{
		samples[] = {{"UAVWEAPONS\M100\M100\data\M100_Shot",1}};
		volume = 0.70794576;
	};
	class UAV_SnipexM100_tailDistant_SoundShader: base_tailDistant_SoundShader
	{
		samples[] = {{"UAVWEAPONS\M100\M100\data\M100_Shot",1}};
		volume = 1.3782794;
	};
};
class CfgSoundSets
{
	class Rifle_Shot_Base_SoundSet;
	class Rifle_Tail_Base_SoundSet;
	class Rifle_silencerHomemade_Base_SoundSet;
	class UAV_SnipexM100_Shot_SoundSet: Rifle_Shot_Base_SoundSet
	{
		soundShaders[] = {"UAV_SnipexM100_closeShot_SoundShader","UAV_SnipexM100_midShot_SoundShader","UAV_SnipexM100_distShot_SoundShader"};
	};
	class UAV_SnipexM100_Tail_SoundSet: Rifle_Tail_Base_SoundSet
	{
		soundShaders[] = {"UAV_SnipexM100_tailDistant_SoundShader","Mosin_tailTrees_SoundShader","Mosin_tailForest_SoundShader","Mosin_tailMeadows_SoundShader","Mosin_tailHouses_SoundShader"};
	};
};
