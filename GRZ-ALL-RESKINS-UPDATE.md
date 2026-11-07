GRZ + XIMS Reskin Update — all available packs combined

Extract this small ZIP to its own folder. Close DayZ and stop the server. In PowerShell run:
powershell -ExecutionPolicy Bypass -File .\Install-All-Reskins.ps1 -SteamId "YOUR_17_DIGIT_STEAM_ID"

Downloads three SHA256-verified archives from a pinned repository commit, combines 18 PBOs plus two locally generated overrides and runs the existing backup/rollback installer. Internet access is required. For offline installation, place the three named ZIPs in one folder and add -PackageDirectory "C:\path\to\zips". ServerPath/GamePath/ProfilePath/MissionName accept the same overrides as Install-Update.ps1; Windows Steam default paths are used otherwise.

Includes the previous arena/menu/gameplay update, CSR50, XIMS CS5, animated wraps/Hexgrids, six GRZ vanilla clothing pieces, and ALL FOUR new clothes: XIMS Sneakers, XIMS Jean Shorts, XIMS Layered Shirt, XIMS Canvas Bag. All four XIMS descriptions are Made By Blake. The loadout prioritizes the GRZ/XIMS kit in its first catalogue page and accepts scalar or array slot definitions with case normalization. A per-item server report is saved to profiles/GRZ_Loadout/clothing-registration.txt after discovery.

IMPORTANT: original Windstride Clothing must be loaded on BOTH sides for XIMS Layered Shirt (verified dependency Layered_Shirt). The installer does not download Windstride. Other existing base weapon/HEXGRID dependencies remain required. Existing Customs PBOs are retained; this does not rebuild or replace the old consolidated GRZ_Customs addon.

Preserves VIP access, prefixes, ratings and saved outfits through the existing transactional installer. Also includes the previously authorized POND DZE/Long Range duel update and appends the supplied Steam ID to the admin map-skip list. Installs PBOs into @GRZ_Local/Addons on BOTH sides; do not load duplicate versions from separate active folders.

All clothing items are now included. Custom normal maps still pending due to oversized uploads: XIMS Canvas Bag, GRZ Jeans, GRZ Bandana. Those items are usable with their supplied colour textures and vanilla normals. Resend 2048px normals or split ZIP parts under 32 MiB to finish them. XIMS sneakers use native material detail because their supplied normal is flat. XIMS Jean Shorts and GRZ Cowboy Hat are black, and GRZ Bandana white, exactly as supplied. Protection and durability remain inherited. Eligible tops, vests, pants and belts now receive 10x10 cargo through the generated override; other clothing and bags retain native cargo.

Static/asset checks and local PowerShell fixture validation only; live Windows downloads, DayZ compilation and model appearance require an in-game check.

The combined installer checks for the original WindstrideClothing.pbo or Layered_Shirt.pbo under each side's @mod/Addons folders and the corresponding Steam Workshop content/221100 folder before installing the layered shirt. A .bisign alone is not enough. For external mod paths, pass -WindstrideServerPbo and -WindstrideClientPbo with the actual PBO paths, and keep that mod loaded. Use -SkipLayeredShirt if you only want the other updates. The installer does not change launcher/server mod arguments.

This installer rebuilds GRZ_InventorySizes from native game configs, staged addons, existing @GRZ_Local addons and the detected server Windstride PBO (including nested Models configs). For other active clothing/weapon mods outside @GRZ_Local, pass -ExtraModPaths with their Addons folders. Do not include inactive conflicting mods. No game/server writes occur until both generated addons succeed.

Movement requests a 1.10 gait animation speed through the native animation API, on both client and server. GRZ_MovementSpeedData contains only animation filenames discovered in your installed native game PBO headers, never copied game animations. Restart BOTH sides. The script log reports accepted/total clips. Native API acceptance and a configured 1.10 value do not establish measured running speed; compare travel time over a fixed distance in game. Prone transitions, attacks, reloads and other actions are excluded. The requested DayZ 1.27 HK/prone drop is a separate pending change.
