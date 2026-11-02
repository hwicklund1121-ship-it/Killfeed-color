GRZ LOCAL SERVER - DRAG AND DROP PACK

1. Install DayZ Server through Steam. Open its installed folder.
2. Extract ALL contents of this ZIP into that folder, beside DayZServer_x64.exe.
   This includes @GRZ_Local, serverDZ.cfg and StartServer.bat.
   If you already have a serverDZ.cfg or StartServer.bat, keep a copy before replacing them.
3. Double-click StartServer.bat. The server loads all eight bundled PBOs.
4. In the DayZ client launcher, add the @GRZ_Local folder as a Local mod and enable it.
   Do not also enable separate copies of the eight bundled mods.
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

GRZ_Autorun.pbo is replaced with the user's v0.11 LB Screens TEST update.
GRZ_CustomHud.pbo is updated to v0.1.14: arrows below health/blood and a cut
indicator/count beside blood. GRZ_Hitmarker.pbo adds hitmarker settings under Options > Game. It currently
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
