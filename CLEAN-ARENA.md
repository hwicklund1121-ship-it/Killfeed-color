GRZ Arena Cleanup

Stop DayZ and the server. Copy BOTH included PBOs into @GRZ_Local/Addons
in the DayZ and DayZServer folders, replacing GRZ_DeathScreen.pbo.
Keep GRZ_MapRotation installed for active-arena respawns.
Restart both with @GRZ_Local enabled.

Normal inventory drop, drag to ground and throw actions are blocked.
Inventory transfers and server loadout/clothing transactions remain available.
Dead PlayerBase characters (including survivor bots) and their contents are
removed 3 seconds after death, allowing the body to fall before cleanup.
A detached held weapon is removed unless another living player acquired it.
Existing ground loot, zombies, animals and placed arena objects are unchanged.
The updated death screen keeps RESPAWN available after corpse removal.

Check on your test server: inventory drop/drag/throw; killfeed and hit sounds;
body and held weapon cleanup; RESPAWN into the active arena; loadout and
clothing saves with cargo. Package/API checks only: not compiled or tested
in DayZ. Unsigned PBOs; sign with your server key if signatures are enforced.
