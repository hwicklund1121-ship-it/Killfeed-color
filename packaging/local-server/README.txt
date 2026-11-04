GRZ LOCAL SERVER - DRAG AND DROP PACK

1. Install DayZ Server through Steam. Open its installed folder.
2. Extract ALL contents of this ZIP into that folder, beside DayZServer_x64.exe.
   This includes @GRZ_Local, serverDZ.cfg and StartServer.bat.
   If you already have a serverDZ.cfg or StartServer.bat, keep a copy before replacing them.
3. Double-click StartServer.bat. The server loads all thirteen bundled PBOs.
4. In the DayZ client launcher, add the @GRZ_Local folder as a Local mod and enable it.
   Do not also enable separate copies of the thirteen bundled mods.
5. Connect to 127.0.0.1:2302. Password: test. Map: Chernarus.
6. Type /grzvip for the VIP menu.

VIP SAVE ACCESS
After the first server launch, stop it. Edit profiles/GRZ_Killfeed/config.json:

    { "AllowedSteamIds": ["YOUR_STEAM_ID_64"] }

Restart the server. The menu can preview without access; saving live VIP settings
requires your SteamID64. Existing profile files are not included in this ZIP.

INCLUDED MODS
GRZ_ActionStamina.pbo
GRZ_CustomHud.pbo
GRZ_Killfeed.pbo
GRZ_LUGU_RollStand.pbo
GRZ_Autorun.pbo
GRZ_Movement.pbo
LUGUINSTADROPFIX.pbo
GRZ_Hitmarker.pbo
GRZ_Survival.pbo
GRZ_NoGrass.pbo
GRZ_Earmuffs.pbo
GRZ_Environment.pbo
GRZ_SkyColors.pbo

GRZ_Autorun.pbo v0.12 adds Alt/remapped freelook while keeping the supplied
v0.11 LB-screen support and existing autorun binding.
GRZ_CustomHud.pbo v0.1.16 replaces the faulty fill-mask shader with clipped
white-fill layers and independent black outlines; arrows and cut count remain. GRZ_Hitmarker.pbo v0.1.4 adds settings under Options > Game and uses native EffectSound
non-spatial playback with OGG audio; the failed direct Play2D path is removed. It currently
uses an explicitly labeled temporary headshot sound, pending the Rust audio clip.
Other uploaded PBOs are preserved unchanged.
Internal PBO prefixes are preserved. PBO-manifest.json has sizes and SHA-256 hashes.
Update the pack on the server and client before joining; do not enable the old
autorun separately alongside this pack.

Requires the Steam DayZ Server installation, standard Chernarus mission and a
DayZ client with the bundled mod enabled. Game/server executables and map files
are not included. Signature verification is disabled for this local development
setup. Packaging checks passed; combined mod compatibility and server startup
have not been tested in the DayZ runtime.

GRZ Survival: no fall damage, full hunger/thirst and neutral temperature for all
players. Combat and other unrelated damage still apply. Hitmarker audio unchanged.

Replace @GRZ_Local on both client and server and restart DayZ fully.

GRZ No Grass disables terrain clutter on ChernarusPlus. Replace the client pack too.
The watermark has been removed from the custom HUD.

Earmuffs: N toggles; +/- changes game effects in 10% steps (10%-100%).
An ear symbol and percentage appear, then fade after each press.
Voice chat volume is unchanged. Rebind under Controls > GRZ Earmuffs.
Earmuffs stay active in inventory, chat and Esc; typing in menus does not trigger hotkeys.

Environment: fixed noon, static clouds, no rain/fog/snow/wind/storms.
Dynamic PO-X gas artillery is suppressed; static gas zones remain.
Staff-related VIP prefix words are blocked by server validation.

Visibility: client object distance is 1400m; terrain horizon distance is 3200m; server and client fog are disabled.
If keeping your own serverDZ.cfg, add/update networkRangeFar = 1400; and restart.
Changing only the PBO will not increase the server replication range.

GRZ Sky colors: Options > Game, below Hit Marker. Arrow choices preview
immediately; Apply saves your local choice and Cancel/Back restores it.
Cloud lighting is retained. Replace the pack on your DayZ client too.

NEW STARTUP SCRIPT REQUIRED FOR EMPTY WORLD:
Copy Setup-GRZEmptyWorld.ps1 and the updated StartServer.bat beside the server exe.
It backs up and zeros loot nominal/min values and disables natural world/NPC
events in the installed mission XML before startup. Player storage is preserved.
Existing saved loot is not deleted; it may remain until separate cleanup.

Killfeed: /killfeed test makes a local BOT preview. Actual spawned survivor kills
show BOT. GRZ Killstreaks announces at 10,20,30... kills; resets on death/logout.
Earmuffs: N starts at 20% every time, +/- adjusts; ear-with-waves icon fades.
Inventory: /grzvip inventory checks whether your generated override is loaded.

Killstreak notifications: top-center popup and sound at each 10-kill milestone.
/killfeed streaktest previews the popup and chime locally without adding kills.
Install matching updated client/server GRZ_Killfeed.pbo files.

When updating, merge the new GRZ files into your existing @GRZ_Local folder.
Preserve extra addons you installed, including UAVWEAPONS.pbo and your generated
GRZ_InventorySizes.pbo. They are not supplied by this server ZIP.
