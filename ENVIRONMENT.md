# GRZ Environment

Server-wide fixed noon (July 1, 2026, 12:00), with native time multiplier zero. The included server config also uses zero time acceleration. A 30-second guard reasserts the lock after conflicting mission/admin changes.

Keeps 25% cloud cover. Disables changing weather, rain, snowfall, fog/volumetric fog, wind and thunderstorms on the current ChernarusPlus setup. Clouds remain rendered; this does not replace the sky or supply custom sky colors. Player-specific colors and live preview are supplied separately by GRZ Sky colors.

Dynamic PO-X artillery areas are suppressed at their CE/deferred initialization and zone setup entry points, before the native artillery RPC and gas/explosion timers. This also discards loaded dynamic areas. Permanent static gas zones and player-thrown gas grenades remain.

Included in @GRZ_Local. Stop client/server, replace @GRZ_Local in both installations, then restart. Your existing StartServer.bat still works. The mod applies the time override even if you retain your existing serverDZ.cfg. No storage wipe or replacement mission folder is required.

Build: `python3 tools/build_environment.py`. PBO/archive checks and native script API/signature checks are performed here. Actual DayZ compilation, join/restart/time/weather synchronization, prolonged play and dynamic gas suppression still require in-game testing. Other mods can override the same hooks or weather; this is intended for the current ChernarusPlus server.

Visibility v0.1.2: client object rendering stays at 1,400 metres; terrain view extends to 3,200 metres to move the far-distance haze beyond that object range during the mission, with regular checks so an Options change does not lower it during play. Preferred graphics settings are not written to disk. Client weather fog and dynamic volumetric fog densities are held at zero alongside the existing server controls; clouds remain.

The included serverDZ.cfg sets `networkRangeFar = 1400;`. If keeping your own config, add or update that line and restart the server. Updating only the PBO cannot increase server entity replication range. Terrain/buildings, engine LOD/culling, character size and hardware still affect whether a player is actually visible; this is not a guarantee of visibility through obstructions or beyond the engine's rendering limits. Validate with two clients at measured distances before relying on it.

Empty-world startup setup: the new StartServer.bat runs Setup-GRZEmptyWorld.ps1 before starting DayZ Server. It patches installed mission types (nominal/min zero), disables all natural events (infected/animals/loot/world events) and sets existing ZombieMaxCount/AnimalMaxCount globals to zero. Source XML backups are created first under GRZ-EmptyWorld-Backups. Player persistence, equipment and economy player-save settings are untouched. Manual VPP survivor spawning remains available. Existing saved floor loot is not deleted. The script is idempotent and aborts if expected economy definitions are missing or the server is already running. Mod-specific scripted spawners outside central economy require their own controls.
