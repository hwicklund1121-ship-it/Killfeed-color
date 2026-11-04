GRZ Native Damage Compatibility v0.1.4

This repair DISABLES the experimental scripted hitscan replacement after the report of blood effects with no damage or hitmarkers. The former native bullet damage rejection and scripted ray damage are removed. Native DayZ projectile damage, bullet drop, travel time, penetration and hit callbacks run normally. True hitscan/no-drop is not active in this build.

Replace GRZ_Hitscan.pbo in BOTH client and server @GRZ_Local/Addons with the supplied file. Its name is retained so copying the update directly overwrites the old damage-blocking implementation. An inert Weapon_Base.GRZAdjustHitscanDirection method is retained only to avoid an undefined override if an old AimControl file was accidentally left behind; it is never called on firing or damage. No firing or damage callbacks are installed by this compatibility build.

DELETE GRZ_Quickscope.pbo and GRZ_AimControl.pbo from BOTH installed Addons folders. They are superseded by GRZ_BetterAim.pbo, which only sets native camera-shooting parameters. Scope-entry timing and camera motion return to native behavior. The separate GRZ_LowRecoil addon supplies the requested 10% ADS recoil modifier without touching damage. GRZ_NoSway remains as the separate original breathing/wandering override. Anti-hardscope keeps its 0.75-second delay and centered red GRZ logo.

The update includes Remove-GRZScopeOverrides.cmd/.ps1 for removing exactly those two old PBO filenames from default Steam installs while DayZ and the server are closed. For other Steam library paths, remove the old files manually or pass the install roots to the PowerShell script with -InstallPaths. It does not delete custom weapons, the generated inventory override, profiles or mission/player saves.

After restarting both, shoot a damageable survivor BOT with ordinary body and head shots, then quickscope. Check actual health loss, the hitmarker and killfeed. Existing GRZ_Hitmarker reports confirmed native damage; no fake markers are emitted. The rollback removes this addon's damage veto but cannot prove other mods or target settings allow damage. Send current server/client logs if native damage still fails.

Static checks verify that the replacement PBO has no EEOnDamageCalculated, EEHitBy, EEFired, ProcessDirectDamage or RejectNative callbacks/code, that its prefix/checksum/payload match source, and that the updated packs contain it. In-game compilation, health loss and hit reporting remain untested here because no DayZ runtime is available.
