GRZ local DayZ server setup

1. Extract serverDZ.cfg and StartServer.bat beside DayZServer_x64.exe.
2. Put @GRZ_Killfeed and @GRZ_CustomHud in that same server folder.
3. Run StartServer.bat and let the server initialize.
4. Stop the server. Edit profiles/GRZ_Killfeed/config.json and add your SteamID64:

   { "AllowedSteamIds": ["YOUR_STEAM_ID_64"] }

   The included GRZKillfeed-config-example.json shows the empty config format.
   Do not replace an existing access list if you want to keep its entries.
5. Restart the server. Enable the same GRZ mods in your DayZ client launcher.
6. Connect to 127.0.0.1:2302. Server password: test.
7. Type /grzvip in chat. Steam-ID access enables saving live VIP settings.

Uses Chernarus and the standard dayzOffline.chernarusplus mission.
This is a local development setup with signature verification disabled for the
unsigned development mods. No Windows/DayZ runtime test was performed here.
