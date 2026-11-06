class CfgPatches
{
 class GRZ_SFGPermanent
 {
  units[] = {};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"sfg50", "UAV_SFG_Suppressor"};
 };
};
class CfgMods
{
 class GRZ_SFGPermanent
 {
  name = "GRZ SFG Permanent Suppressor";
  author = "Blake";
  type = "mod";
  dependencies[] = {"World"};
  class defs
  {
   class worldScriptModule
   {
    value = "";
    files[] = {"GRZSFGAUTO/SFG50/SFG/scripts/4_World"};
   };
  };
 };
};
class Mode_SemiAuto;
class CfgWeapons
{
 class BoltActionRifle_InnerMagazine_Base;
 class sfg50_Base: BoltActionRifle_InnerMagazine_Base
 {
  model = "GRZSFGAUTO\SFG50\SFG\data\SFG50.p3d";
  attachments[] = {"weaponWrap", "weaponOpticsHunting", "UAV_SFG_Suppressor"};
  GRZIntegratedSuppressor = 1;
  class Single: Mode_SemiAuto
  {
   soundSetShot[] = {"sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet"};
   soundSetShotExt[] = {{"sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet", "sfgs50_Shot_SoundSet"}};
   reloadTime = 1.0;
   recoil = "recoil_AKM";
   recoilProne = "recoil_AKM_prone";
  };
 };
};
class CfgVehicles
{
	class ItemSuppressor;
	class SFG_Suppressor_Base: ItemSuppressor
	{
		scope = 1;
		model = "GRZSFGAUTO\SFG50\SFG\proxy\sfg_muzzle.p3d";
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
					healthLevels[] = {{1,{"GRZSFGAUTO\SFG50\SFG\data\suppressor.rvmat"}},{0.7,{"GRZSFGAUTO\SFG50\SFG\data\suppressor.rvmat"}},{0.5,{""}},{0.3,{""}},{0,{""}}};
				};
			};
		};
	};
	class UAV_SFG_Sup: SFG_Suppressor_Base
	{
		scope = 1;
		displayName = "SFG Suppressor";
		descriptionShort = "SFG Suppressor";
		hiddenSelectionsTextures[] = {"GRZSFGAUTO\SFG50\SFG\data\suppressor.paa"};
		hiddenSelectionsMaterials[] = {"GRZSFGAUTO\SFG50\SFG\data\suppressor.rvmat"};
	};
};
class CfgNonAIVehicles
{
	class ProxyAttachment;
	class Proxysfg_muzzle: ProxyAttachment
	{
		scope = 2;
		simulation = "alwaysshow";
		inventorySlot = "";
		model = "GRZSFGAUTO\SFG50\SFG\proxy\sfg_muzzle.p3d";
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
