GRZ Map Rotation 0.1.0

INSTALL
Stop the server and close DayZ. Extract the whole ZIP and run Install-GRZMapRotation.cmd.
It installs GRZ_MapRotation.pbo into @GRZ_Local/Addons on BOTH game and server.
It installs one copy of the supplied GRZ_BBC_DM_2.dze in the server mission EditorFiles folder.
Recognized duplicates and previous files are backed up outside EditorFiles; unrelated DZE files are retained.
Keep Dabs Framework and DayZ Editor Loader loaded on the server, and @GRZ_Local loaded on both sides.
Keep every object mod used by the supplied DZE loaded on both sides, including the GRZ containers.
The installer preserves your BAT, server options, Customs pack, profiles and saves.

ARENAS
Electro Hill: rock/building group centred approximately X8997 Z2562.
NWAF: container/castle-wall group centred approximately X4462 Z10268.
NEAF: rock/barrier group centred approximately X12205 Z12531.
The two other build areas in the DZE are not voting options.
The whole DZE remains loaded by Editor Loader; rotation changes the active play area and spawn/return rules.
Blueprints show placement-derived schematic outlines/object markers, not exact mesh footprints.
Arena boundaries follow the outer convex envelope of each selected build group.

ROUND TIMING
30-minute rounds. At 27:30: voting-soon notification and sound.
28:00: voting opens; black/white GunRunnerZ-branded popup appears for up to 30 seconds.
29:30: voting locks, winning arena is announced with a sound.
30:00: players move to the selected arena with their existing gear; map-change sound/notification plays.
Type !vote in chat, or press F7, to reopen the popup while voting is open.
Each opening lasts at most 30 seconds. Re-voting replaces your previous vote; it never adds extra votes.
The current arena is excluded. Ties choose randomly among tied winners; no votes advances to the next arena.
Votes from late arrivals are accepted before the lock. Votes after the lock or from stale rounds are rejected.

SPAWNS AND BOUNDARIES
Respawns and reconnects enter the active arena. Existing saved outfits/loadout scripts are not removed.
Living players crossing the configured boundary get a five-second countdown and return with gear preserved.
Returning inside cancels the countdown. Detection runs every 250ms.
Players in vehicles are asked by the native exit command to get out before the move completes.
Spawn candidates are generated from open spaces in the DZE placement diagram.
The server checks terrain/water, surface slope and standing-room collision after Editor Loader finishes.
If any arena has fewer than two valid spawn positions, rotation disables itself rather than guessing.
These checks do not prove gameplay suitability: test the actual spawn locations in DayZ before using publicly.
Spawn selection favours distance from other living players. Dynamic objects placed after startup may affect cached positions.

CONFIGURATION
First startup creates $profile:GRZ_MapRotation/config.json from the bundled defaults.
The current arena ID is saved in active-arena.json; server restart begins a fresh 30-minute round there.
Arenas are configurable from two up to six. Add an ID, name, matching client blueprint PAA, boundary polygon and spawn candidates.
You must distribute new blueprint assets to clients when adding arenas.
See the RPT for [GRZ Rotation] runtime-checked spawn counts or disabled/config errors.
Custom mission scripts overriding CreateCharacter or OnClientReadyEvent without calling super can bypass hooks and require integration.

VALIDATION
DZE v8 parsed against Dabs Framework's public field definitions; all placed/hidden records preserved.
Private author/credit metadata removed from the packaged DZE; object data remains unchanged.
Static PBO, config, geometry, timeline and installer checks only. Enforce compilation, physics, teleport sync, audio and multiplayer voting require DayZ 1.30 testing.
Unsigned development PBO; sign using your server key if signature verification is enabled.

Respawn update: PlayerBase.OnConnect now schedules arena placement for both fresh characters and existing characters. This covers missions that replace CreateCharacter without calling super. Duplicate ready callbacks are ignored. Leaving the boundary shows a five-second teleport countdown, which clears when the player returns inside.
