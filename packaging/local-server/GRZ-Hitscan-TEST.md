GRZ Hitscan 0.1.1 — local-server test build

Merge the update ZIP into the installed DayZ Server folder and your DayZ game folder. It contains @GRZ_Local/Addons/GRZ_Hitscan.pbo, GRZ_NoSway.pbo and the updated GRZ_Killfeed.pbo. Load @GRZ_Local on both sides and fully restart both. Preserve UAVWEAPONS.pbo, GRZ_InventorySizes.pbo and all other addons. There is no extra startup argument when using @GRZ_Local.

For supported firearms the server handles each EEFired event with one straight muzzle ray, up to 1400 metres. Damage is instantaneous and has no bullet drop. The first fire-geometry surface stops the ray, including terrain; there is no penetration or ricochet. It uses the configured ammo damage at coefficient 1, the ray's component damage zone, and the firing weapon as the source. There is no travel-speed damage loss or native random bullet dispersion on this ray. Native ammo consumption, cycling and recoil continue. GRZ_NoSway removes breathing/wandering offsets separately.

Native bullets are still emitted for sound, tracers and impact effects. A server damage filter rejects their FIRE_ARM damage for supported source/ammo combinations, including rays that miss. Only the synchronous scripted damage call is permitted through that filter. This is hitscan damage, not a visual replacement of the native projectile. Original tracers/impact effects may still show a curved path or a later impact.

Supported: nonexplosive, single-projectile shotBullet ammo fired from single-muzzle weapons with valid root muzzlePos/muzzleEnd memory points. Modded guns meeting those conditions are detected at runtime; they do not need a class list. Multi-muzzle guns, multi-pellet shells, arrows, grenades and explosives retain native ballistics. Missing/invalid muzzle definitions retain native behavior too.

Compatibility: damage filters are installed on scripted PlayerBase, ZombieBase, AnimalBase, ItemBase, BuildingBase, CarScript and BoatScript classes. The engine Object class cannot be modded; v0.1.1 removes that invalid hook. The ray only applies damage to those filtered target families. Other object types keep native projectile damage. Vanilla Plastic_Explosive has its own wrapper because its damage callback bypasses super. Other mods overriding EEOnDamageCalculated without calling super, or overriding EEFired without calling super, can bypass filtering/firing. This test build has not been verified with the user's custom weapon PBO. Do not treat duplicate-damage prevention or hit registration as runtime-proven.

Check in DayZ before continuing normal play:

1. Start the server, join, and fire an ordinary M4A1 or similar rifle. The server script log should say `[GRZ Hitscan TEST 0.1.1] Server muzzle ray active` once per weapon instance. No such line means this hook/weapon did not activate. The wrapper fix removes the reported engine-class compile error; actual Enforce compilation still requires this restart.
2. Shoot an unarmoured survivor BOT at close range and at 300+ metres. Confirm immediate damage and correct head/body zones. Inspect health after a single shot and again after the original bullet would arrive: there must be no delayed second damage.
3. Repeat with armour and the custom UAV gun, checking hitmarkers, live BOT killfeed and killstreak attribution. Native engine damage transfer and proxy collision behavior need this test.
4. Put a BOT behind a wall and shoot toward it. It must remain unharmed, including after the native projectile's arrival time. Repeat a deliberate ray miss with a target behind/below the aim point.
5. Test melee, fall/survival behavior, unsupported pellets and explosive ammo to confirm their prior behavior. Your existing survival addon still governs fall damage.

If damage stops registering or doubles, stop the server and remove only GRZ_Hitscan.pbo from both installations, then restart both. No save or mission files are modified by this addon. Keep GRZ_NoSway and GRZ_Killfeed if desired.

Verification here checks packaging, source API signatures and damage-hook inheritance in the available DayZ script reference. There is no DayZ/Enforce compiler or running game in this workspace; the runtime checks above remain outstanding.

Version 0.1.1 fixes the reported Game-module error `Engine class Object cannot be modded` by moving damage callbacks onto the seven scripted target families in the World module. Filters run before inherited hitmarker callbacks so rejected native bullet hits do not become extra damage reports. Native target-specific damage rules still run for permitted scripted hits and for other damage types.
