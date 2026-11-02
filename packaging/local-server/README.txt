GRZ LOCAL SERVER - DRAG AND DROP PACK

1. Install DayZ Server through Steam. Open its installed folder.
2. Extract ALL contents of this ZIP into that folder, beside DayZServer_x64.exe.
   This includes @GRZ_Local, serverDZ.cfg and StartServer.bat.
   If you already have a serverDZ.cfg or StartServer.bat, keep a copy before replacing them.
3. Double-click StartServer.bat. The server loads all seven supplied PBOs.
4. In the DayZ client launcher, add the @GRZ_Local folder as a Local mod and enable it.
   Do not also enable separate copies of the seven bundled mods.
5. Connect to 127.0.0.1:2302. Password: test. Map: Chernarus.
6. Type /grzvip for the VIP menu.

VIP SAVE ACCESS
After the first server launch, stop it. Edit profiles/GRZ_Killfeed/config.json:

    { "AllowedSteamIds": ["YOUR_STEAM_ID_64"] }

Restart the server. The menu can preview without access; saving live VIP settings
requires your SteamID64. Existing profile files are not included in this ZIP.

INCLUDED - EXACT UPLOADED FILES
GRZ_ActionStamina.pbo
GRZ_CustomHud.pbo
GRZ_Killfeed.pbo
GRZ_LUGU_RollStand.pbo
GRZ_Autorun.pbo
GRZ_Movement.pbo
LUGUINSTADROPFIX.pbo

The original PBO bytes and internal prefixes are preserved. PBO-manifest.json
contains sizes and SHA-256 hashes. This bundle uses the uploaded killfeed PBO;
it is not replaced with another build.

Requires the Steam DayZ Server installation, standard Chernarus mission and a
DayZ client with the bundled mod enabled. Game/server executables and map files
are not included. Signature verification is disabled for this local development
setup. Packaging checks passed; combined mod compatibility and server startup
have not been tested in the DayZ runtime.
