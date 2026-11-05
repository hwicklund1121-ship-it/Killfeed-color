CS5 .50 CAL correction

Replace the complete CS5 folder in your unpacked UAVWEAPONS pack with this CS5 folder.
Repack the weapon pack, update BOTH game and server copies, and restart both.
Spawn a fresh UAV_CS5_BLACK and fresh CS5_Magazine_10RND magazines.
Already loaded rounds in existing magazines/chambers are not converted by config changes.
The UAV ammo addon defining UAV_Ammo_50CAL and UAV_Bullet_50CAL must be loaded.

CS5 chambers UAV_Ammo_50CAL; CS5_Magazine_10RND accepts UAV_Ammo_50CAL and fires UAV_Bullet_50CAL.
The config now inherits BoltActionRifle_Base, matching the script's CZ527 detachable-magazine mechanism.
Magazine capacity remains 10. Models, sounds, scripts and other weapon stats unchanged.
The GRZ loadout automatically detects this scope=2 bolt-action as .50 cal after restart.
The current loadout gives a compatible scope, selected wrap, ten ammo stacks and six full spare magazines.
No new loadout PBO is needed for this gun fix.

Config/package checks passed; not compiled or tested in DayZ.
