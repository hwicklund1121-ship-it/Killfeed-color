GRZ + XIMS Reskin Update — available packs combined

Extract this small ZIP to its own folder. Close DayZ and stop the server. In PowerShell run:
powershell -ExecutionPolicy Bypass -File .\Install-All-Reskins.ps1 -SteamId "YOUR_17_DIGIT_STEAM_ID"

Downloads three SHA256-verified archives from a pinned repository commit, merges their 17 PBOs and runs the existing transactional installer. Internet access is required. Alternatively place the three named ZIPs in one folder and add -PackageDirectory "C:\path\to\zips" for offline installation. ServerPath/GamePath/ProfilePath/MissionName accept the same overrides as Install-Update.ps1. Windows Steam default paths are used otherwise.

Includes the prior arena/menu/gameplay update, CSR50, XIMS CS5, animated wraps/Hexgrids, six GRZ vanilla clothing pieces, XIMS Sneakers and XIMS Jean Shorts. Names/descriptions for the new XIMS clothes: XIMS Sneakers / XIMS Jean Shorts, Made By Blake. Clothing is discovered by the existing loadout after restarting client and server. Original weapon/HEXGRID and other existing dependencies remain required. Existing Customs PBOs are retained; this does not rebuild or replace your old consolidated GRZ_Customs addon.

Preserves profiles/VIP access/prefixes/ratings/saved outfits using the existing backup/rollback installer. Also includes the previously authorized POND DZE/Long Range duel update and appends the supplied Steam ID to the admin map-skip list. Installs all PBOs to @GRZ_Local/Addons on BOTH sides. Do not run the old and new clothing PBOs from different active folders.

Pending and intentionally NOT installed:
- XIMS Layered Shirt: model references Windstride Clothing. Layered_Shirt_Base is confirmed in public class lists, but the original Windstride config.cpp is needed to verify the addon dependency and texture bindings. Colour/normal textures are converted and staged; the female MLOD alone is not a complete ground/male/female addon.
- XIMS Canvas Bag: the uploaded ZIP exceeded the 32 MiB transfer limit. Resend colour/normal textures separately in ZIPs under 32 MiB each.
- GRZ Jeans and GRZ Bandana currently use their supplied colours plus vanilla normals. Their custom normal ZIPs were still oversized; 2048px copies or split uploads can finish those later.

New XIMS sneakers use native material detail because the supplied exported normal is flat. Jean shorts use the supplied custom baked normal and all-black colour. The GRZ cowboy hat is black and GRZ bandana white, exactly as supplied. No new clothing armour, durability, slots or storage changes.

Static/asset checks and local PowerShell fixture validation only. Live Windows downloads, DayZ compilation and model appearance require an in-game check.
