////////////////////////////////////////////////////////////////////
//DeRap: Ammo\50CAL\config.bin
//Produced from mikero's Dos Tools Dll version 10.21
//https://mikero.bytex.digital/Downloads
//'now' is Wed Nov 04 03:49:57 2026 : 'file' last modified on Sat Oct 24 21:26:43 2026
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgPatches
{
	class UAV
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data","DZ_Weapons_Ammunition","DZ_Weapons_Projectiles"};
		magazines[] = {"UAV_Ammo_50CAL"};
		ammo[] = {"UAV_Bullet_50CAL"};
	};
};
class cfgAmmoTypes
{
	class AType_UAV_Bullet_50CAL
	{
		name = "UAV_Bullet_50CAL";
	};
};
class cfgMagazines
{
	class Ammunition_Base;
	class UAV_Ammo_50CAL: Ammunition_Base
	{
		scope = 2;
		displayName = "50CAL Ammo";
		descriptionShort = "50CAL Ammo";
		model = "UAVWEAPONS\Ammo\50CAL\data\50CAL_LooseRounds.p3d";
		weight = 100;
		count = 20;
		itemSize[] = {1,3};
		ammo = "UAV_Bullet_50CAL";
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100000;
					healthLevels[] = {{1.0,{"UAVWEAPONS\Ammo\50CAL\data\50CAL_LooseRounds.rvmat"}},{0.7,{"UAVWEAPONS\Ammo\50CAL\data\50CAL_LooseRounds.rvmat"}},{0.5,{"UAVWEAPONS\Ammo\50CAL\data\50CAL_LooseRounds.rvmat"}},{0.3,{"UAVWEAPONS\Ammo\50CAL\data\50CAL_LooseRounds.rvmat"}},{0.0,{"UAVWEAPONS\Ammo\50CAL\data\50CAL_LooseRounds.rvmat"}}};
				};
			};
		};
	};
};
class CfgAmmo
{
	class Bullet_Base;
	class UAV_Bullet_50CAL: Bullet_Base
	{
		scope = 2;
		casing = "FxCartridge_762";
		round = "FxRound_308Win";
		spawnPileType = "UAV_Ammo_50CAL";
		model = "\dz\weapons\projectiles\tracer_green.p3d";
		hit = 30;
		indirectHit = 0;
		indirectHitRange = 0;
		visibleFire = 22;
		audibleFire = 22;
		visibleFireTime = 4;
		airLock = 1;
		typicalSpeed = 1400;
		initSpeed = 1400;
		airFriction = -5e-05;
		caliber = 250;
		deflecting = 5;
		damageBarrel = 800;
		damageBarrelDestroyed = 800;
		weight = 0.01;
		hitAnimation = 1;
		impactBehaviour = 1;
		class DamageApplied
		{
			type = "Projectile";
			dispersion = 0.0;
			bleedThreshold = 1.0;
			defaultDamageOverride[] = {{0.1,1.0}};
			class Health
			{
				damage = 1300;
			};
			class Blood
			{
				damage = 0;
			};
			class Shock
			{
				damage = 25;
			};
		};
		class NoiseHit
		{
			strength = 80;
			type = "shot";
		};
	};
};
