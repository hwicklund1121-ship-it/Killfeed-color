GRZ + XIMS Reskin Update — all available packs combined

Extract this small ZIP to its own folder. Close DayZ and stop the server. In PowerShell run:
powershell -ExecutionPolicy Bypass -File .\Install-All-Reskins.ps1 -SteamId "YOUR_17_DIGIT_STEAM_ID"

Downloads three SHA256-verified archives from a pinned repository commit, combines 18 PBOs and runs the existing backup/rollback installer. Internet access is required. For offline installation, place the three named ZIPs in one folder and add -PackageDirectory "C:\path\to\zips". ServerPath/GamePath/ProfilePath/MissionName accept the same overrides as Install-Update.ps1; Windows Steam default paths are used otherwise.

Includes the previous arena/menu/gameplay update, CSR50, XIMS CS5, animated wraps/Hexgrids, six GRZ vanilla clothing pieces, and ALL FOUR new clothes: XIMS Sneakers, XIMS Jean Shorts, XIMS Layered Shirt, XIMS Canvas Bag. All four XIMS descriptions are Made By Blake. The existing loadout discovers these after client/server restart.

IMPORTANT: original Windstride Clothing must be loaded on BOTH sides for XIMS Layered Shirt (verified dependency Layered_Shirt). The installer does not download Windstride. Other existing base weapon/HEXGRID dependencies remain required. Existing Customs PBOs are retained; this does not rebuild or replace the old consolidated GRZ_Customs addon.

Preserves VIP access, prefixes, ratings and saved outfits through the existing transactional installer. Also includes the previously authorized POND DZE/Long Range duel update and appends the supplied Steam ID to the admin map-skip list. Installs PBOs into @GRZ_Local/Addons on BOTH sides; do not load duplicate versions from separate active folders.

All clothing items are now included. Custom normal maps still pending due to oversized uploads: XIMS Canvas Bag, GRZ Jeans, GRZ Bandana. Those items are usable with their supplied colour textures and vanilla normals. Resend 2048px normals or split ZIP parts under 32 MiB to finish them. XIMS sneakers use native material detail because their supplied normal is flat. XIMS Jean Shorts and GRZ Cowboy Hat are black, and GRZ Bandana white, exactly as supplied. No new clothing stat, protection, durability, slot or storage changes.

Static/asset checks and local PowerShell fixture validation only; live Windows downloads, DayZ compilation and model appearance require an in-game check.

The combined installer checks for the original WindstrideClothing.pbo or Layered_Shirt.pbo under each side's @mod/Addons folders and the corresponding Steam Workshop content/221100 folder before installing the layered shirt. A .bisign alone is not enough. For external mod paths, pass -WindstrideServerPbo and -WindstrideClientPbo with the actual PBO paths, and keep that mod loaded. Use -SkipLayeredShirt if you only want the other updates. The installer does not change launcher/server mod arguments.
