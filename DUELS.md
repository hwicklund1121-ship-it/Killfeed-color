GRZ Loadout 0.3.1 — Duels and Server Leaderboard

INSTALL
Stop both DayZ and the server. Replace BOTH GRZ_Loadout.pbo and
GRZ_MapRotation.pbo in @GRZ_Local/Addons on BOTH game and server.
Restart both. Keep your existing GRZ_Loadout, GRZ_MapRotation profile data,
custom weapon packs, corpse cleanup, death screen and DZE files.
The paired map-rotation update depends on this new loadout version.
No extra mod-folder launch parameter is needed.

CONTROLS
Comma opens the existing loadout menu; click DUELS.
O opens SERVER LEADERBOARD. Both bindings are rebindable in Controls > GRZ Loadout.
Escape or X closes either menu.

DUELS
The menu lists every online player with their rank/Elo and availability,
plus the enabled named duel maps and Best of 1 / 3 / 5 buttons.
Select an opponent and map, select match length, then SEND CHALLENGE.
The opponent has 60 seconds to ACCEPT or DECLINE. The sender can CANCEL.
Incoming challenges open the duel menu when no other menu/dialog is open.
One pending challenge per player and one simultaneous match per duel map.
Each player uses the firearm IN THEIR HANDS when the challenge is accepted,
including its scope, wrap, suppressor and nested attachments. The challenger
must still hold the same weapon entity they used when sending the challenge.
This is independent of the saved normal-respawn gun selection.
Single-muzzle firearms with resolvable ammo-pile configuration are supported.
Each round regrants that weapon/attachments, loads it, and supplies the normal
10 ammo stacks and six spare magazines for detachable-magazine guns.
The existing transactional grant replaces carried guns/ammo while preserving
other cargo/clothing. Duel kit grants do not overwrite normal saved loadouts.
Loadout/clothing saves are locked while a player is in a duel.

First to 1, 2 or 3 round wins decides BO1, BO3 or BO5. Both players are put on
their respective side before each round. A visible 5,4,3,2,1 countdown precedes
GO; damage is server-blocked and local firing input disabled until GO.
Health, shock, blood and damage zones are reset between rounds.
Player deaths allow the 3-second death-camera/fade transition, then use the native respawn flow automatically, then re-equip the
current duel kit and return to the same side. The match has a 90-second
respawn/gear-preparation grace period. Normal arena rotation/boundary checks
skip duel participants, and its voting popup is suppressed during a duel.
A score HUD shows the current opponent, map and score.
Duels cannot damage other matches or outside players, and outside players,
AI and environment damage cannot harm duel participants. The matching held
firearm is required for valid duel damage. Leaving the configured duel radius
blocks damage and returns that player to their side after five seconds.

After the match, living players return immediately to the CURRENT active
rotation arena. Dead players auto-respawn and return there. If rotation is
unavailable, return falls back to the player's pre-duel position.
Forfeit and sustained disconnect award the opponent the match. Temporary
network-entity replacement during a round respawn is not an instant forfeit.
Round duration defaults to 180 seconds. A simultaneous death or timeout is a
draw round; three drawn rounds cancel the match without Elo changes. A
one-sided failure to respawn/prepare within the grace period is a forfeit.
Server restarts do not resume unfinished matches or award their Elo.

ADD YOUR DUEL MAPS WHEN YOU BUILD THEM
The leaderboard/ranks work immediately, but the shipped duel arena example
is DISABLED with zero coordinates. It will never teleport players there.
On first start the server creates profiles/GRZ_Duels/config.json.
For each built arena, configure a UNIQUE Id, display Name, Enabled=true,
Center, Radius, SpawnA, SpawnB, FacingA and FacingB, then restart the server.
Positions are JSON coordinate arrays [X, Y, Z]; use two verified clear standing positions
on opposite sides. Center/radius describes the horizontal arena boundary.
Both spawns must be within the radius, at least five metres apart and not
at zero/water coordinates. All arena objects must be loaded by your DZE/mod.
The named map list is read from this config; the mod does not build arenas.
An illustrative configuration is included as duel-config.example.json.
Do not enable it until you enter and test the actual arena coordinates.
When your arena is built, its DZE or side coordinates can be used to prepare
this config. Do not place duel arenas inside the normal rotation play areas.

ELO AND RANKS
Every player starts at 800. Match results use K=32 Elo expected score:
gain = round(32 * (1 - 1/(1 + 10^((opponentElo-ownElo)/400)))).
The winner gains and loser loses that amount, with a minimum of 1, maximum
32 and a rating floor of zero. At 800: beating 800 gives +16; beating 1000
+24; beating 600 +8. Ranked changes happen once per completed/forfeited
MATCH, not per round. Clients cannot submit wins, scores or rating deltas.
Ratings use a verified temporary JSON and backup before replacing the server
file. A rating-storage failure disables further ranked updates and reports
the problem without overwriting unreadable records.

Bronze:       0–999    copper shield/chevron (800 starting rank)
Silver:    1000–1199   silver double chevron
Gold:      1200–1399   gold shield/gem
Platinum:  1400–1599   teal winged shield
Diamond:   1600–1799   ice-blue faceted gem
Master:    1800–1999   purple crown
Grandmaster:2000–2199  crimson crown/laurels
Legend:    2200+       orange radiant crown
Badges are drawn as native vectors and scale with the menu resolution.

LEADERBOARD
O shows all recorded players, including offline players, 20 per page.
Columns: player, kills, deaths, K/D and Elo, with rank colour/badge.
Sort by kills, K/D or Elo. Server-side sort caches refresh every five seconds.
K/D uses kills/max(deaths,1) and displays two decimals.
PlayerBase kills count, including VPP survivor bots; infected/animals do not.
Only authenticated player accounts get a persistent row. Self-kills do not
grant a kill. Player deaths from any cause count once; duel rounds also count
in server-wide K/D. Historical kills before this update are not imported.
Stats are buffered and flushed every ten seconds and on clean shutdown;
a crash can lose the latest buffered K/D events. Match Elo commits immediately.
Names, K/D, wins/losses and ratings live in profiles/GRZ_Duels/ratings.json.
Keep this folder across updates and restarts. Steam identifiers stay in server
storage; leaderboard RPC rows contain only display name, stats and Elo.

VALIDATION AND LIMITS
PBO/source, native API signatures, RPC ownership, widget references, key
bindings, Elo vectors and save/restore paths are checked offline. This is an
unsigned source-script build: no DayZ runtime compilation or multiplayer test
has been performed. Test two accounts, both respawns, BO1/3/5, accept/decline/
expiry, disconnect, simultaneous kill, inventory space, all weapon attachments,
map rotation during a duel, return to active arena and rating/stat persistence.
Actual duel rounds additionally require your configured, built arenas.

Loadout 0.3.2 adds BAG selection and native mouse/keyboard/gamepad menu focus to keep the gameplay camera still. Rotation 0.1.3 adds server-authorized !skip: the next arena starts after a 30-second top-screen warning. Configure AdminSteamIds in profiles/GRZ_MapRotation/config.json; the shipped allowlist is empty. Duel participants remain in their duel.

## Configured DZE arenas

`GRZ-Duels-Arenas-Update.zip` supplies the five enabled maps from GRZ_BBC_DM_3.dze: 100 Meters (Vybor field), Shipment (Cherno apartments), Long Range 1v1 (NWAF municipal clock-tower build), Cargo Containers (near VMC), and Castle 1v1 (between Cherno and Elektro). Its dedicated installer installs `duel-config.json` into the server's GRZ_Duels profile and replaces the known older DZE, with backups. Existing ratings and main rotation settings are preserved.

Both spawn sides face their opponent. GroundSpawns resolves Y from live Chernarus terrain each round; keep it false for future rooftop arenas with explicitly measured Y. The 100 Meters spawns are 100 metres apart horizontally. The included black-and-white planning preview marks sides A/B. Positions were derived from placed geometry; verify both sides in game, including modded object collision, before public matches.
