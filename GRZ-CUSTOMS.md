GRZ Customs 0.2.0

Current pack layout: two PBOs only. HEXGRID.pbo contains the original repaired
HEXGRID models/base class and shared armour tuning. GRZ_Customs.pbo contains one
root CfgPatches/CfgVehicles/CfgWeapons config for 55 clothing/wrap reskins and rvk M100 and their assets,
mounted consistently under GRZ_Customs. Spawn class names are retained.

The installer backs up and removes the four obsolete split reskin PBOs before
copying the consolidated addon into both client and server mod folders. Run the
new installer rather than copying over files and leaving duplicate definitions.
Keep the helmet and SK dependencies loaded on both sides.

A small startup check writes GRZ_Customs_registration.txt into the server/client
profile directory and prints the number of registered custom item classes. A
successful config check reads Registered 56/56 custom items. This is a runtime
config diagnostic; it does not alter inventories, spawn items or grant admin.
If the script itself cannot load, no report is written and its compile error
will appear in the script log. Packaging tests cannot establish an in-game fix.


Fix: custom HEXGRID and tracksuit configs no longer declare any DamageSystem,
GlobalHealth or Health blocks. Their native damage settings are inherited.
Installer prints and checks the supplied pack version before copying files.

Gun wraps: 48 separate GRZ_Gunwraps_<Colour> items (36 solids and 12 mixes).
All display names match their spawn class and descriptions say Made By Blake.
Examples: GRZ_Gunwraps_Red, GRZ_Gunwraps_Black, GRZ_Gunwraps_White,
GRZ_Gunwraps_Red_Black, GRZ_Gunwraps_Cyan_Purple, GRZ_Gunwraps_Rainbow.
GUNWRAPS-ITEMS.json lists every variant. Original vanilla wraps remain available.

The wraps inherit GhillieAtt_ColorBase model, attachment slot, handling, stats and
native materials (including normal/specular detail). Only the colour texture
changes. The supplied native colour map's UV positions, cut-out alpha and
alpha-test flag are preserved. Mixed colours form alternating fabric patches.
They attach to weapons that already support vanilla gun ghillie wraps.
These items need the vanilla DZ_Weapons_Supports addon, not an extra Workshop mod.

Includes separate GRZ_Tracksuit_Pants item, display name GRZ_Tracksuit_Pants,
description Made By Blake. It inherits vanilla TrackSuitPants_ColorBase geometry,
clothing behaviour and stats, with supplied 2048x2048 colour/normal artwork.
No additional model dependency is required for these pants. Their damage settings and damage-level materials are inherited from the original pants.

Also includes Blake's configured kit, excluding its HEXGRID item and textures:
- blake_SurgicalGloves
- blake_jean
- blake_helment (original class spelling retained)
- blake_sneakers
- blake_Tshirt

Their supplied names, descriptions, base classes, materials and PAA assets are
preserved. The kit's units list now lists its actual five items. It requires
UAV_Helmet_BNTI and sk_taladro in addition to vanilla DayZ clothing addons.
Load the mods supplying these addons on both client and server before Customs.
Their external models are not included in the uploaded kit. Remove any other
active Blakes_Kit.pbo before using the copy supplied in this pack.

Config layout follows Blake's kit: declared ColourBase parent, separate scoped
item, displayName, descriptionShort and hidden-selection texture/material arrays.
Custom addon asset paths match their own PBO prefixes. No native class is
overwritten. HEXGRID inherits HEXGRID_Base; the original UAV_HEXGRID is retained.

This separate reskin pack currently contains the supplied Gunrunnerz HEXGRID:
black/white artwork and matching embossed normal map, both 2048x2048 PAA.
Additional reskins can be added to @GRZ_Customs/Addons as they are supplied.
Two separate spawnable items are available:
- UAV_HEXGRID: the original UAV vest with its original textures.
- GRZ_GunRunnerZ_Hexgrid: display name "GunRunnerZ Hexgrid", description
  "Made By Blake", with the supplied custom colour and normal maps.

HEXGRID.pbo supplies the original model and item, with its broken mount repaired.
The consolidated GRZ_Customs.pbo adds the new item through inheritance, with its own
texture/material paths. Original geometry, animation script and cargo are retained. Shared armour and
durability are balanced as described below. Damage settings and damage-level materials are inherited from the original vest.

Installation

Recommended: extract the entire ZIP, close DayZ and the server, and run
Install-GRZCustoms.cmd. This copies and verifies both PBOs in the default
Steam game/server folders, backs up duplicates from @GRZ_Local, and adds
@GRZ_Customs to the existing server BAT -mod argument. Existing BAT settings
are preserved. If installed under Program Files, run as administrator if Windows
reports access denied. For other install locations, use PowerShell:
./Install-GRZCustoms.ps1 -ServerPath 'D:\DayZServer' -GamePath 'D:\DayZ'

Then enable the DayZ game folder's @GRZ_Customs as a LOCAL MOD in the DayZ
Launcher and check its box before launching. Copying its folder alone does not
load it. The server BAT must also load it, using -mod rather than -servermod.
Spawn GRZ_Tracksuit_Pants or GRZ_Gunwraps_Red to check the vanilla-only items.
If missing after both installations are verified and enabled, inspect the client
and server RPT for loading/dependency/config errors. Packaging checks do not
prove that an installation has loaded the mod or compiled its configs in DayZ.

Manual installation:

1. Stop DayZ and the server. Extract @GRZ_Customs into BOTH the DayZ game folder
   and the DayZServer folder, beside @GRZ_Local.
2. Replace the previous @GRZ_Customs pack with this update. Move any other
   standalone HEXGRID.pbo and its old signature out of active mod
   Addons folders on BOTH installations. Keep a backup outside active mods.
   Do not load two copies of HEXGRID. If another combined PBO also defines
   UAV_HEXGRID, resolve that duplicate before loading this replacement.
3. Add ;@GRZ_Customs to your existing server -mod argument, for example:
   "-mod=@CF;@VPPAdminTools;@GRZ_Local;@GRZ_Customs"
   Preserve all other existing launch arguments. Use -mod, since clients need
   these models/textures too. Enable @GRZ_Customs as a local mod in the client
   launcher alongside your existing mods.
4. Restart both. Spawn GRZ_GunRunnerZ_Hexgrid for the custom vest, or UAV_HEXGRID
   for the original. Check ground, inventory, hands and male/female worn views.

This development pack is unsigned. For a server enforcing signatures, sign the
PBO with your own DayZ Tools key and install its public key on the server.

Validation: PAA mip dimensions and decoded colour channels, PBO checksum and
round-trip contents, internal resource paths, unchanged models/scripts/material,
and ZIP contents. DayZ rendering and script compilation require in-game testing.

Rebuild: python tools/build_customs.py --hexgrid <original HEXGRID.pbo>
Requires Pillow with DXT5 DDS encoding. Source artwork is in art/customs/hexgrid.

HEXGRID armour balance (0.1.8)

All variants inheriting HEXGRID_Base, including UAV_HEXGRID and
GRZ_GunRunnerZ_Hexgrid, use native GlobalArmor projectile health damage 0.045.
This replaces the supplied, ineffective GlobalHEXGRID block name. Blood/shock
and other damage-type values from the source armour block are retained. Vest
hitpoints increase from 150 to 10000 so it can remain protective across these hits.
Models/textures/cargo and ammo definitions are unchanged.

Supplied ammo base health damage and calculated player damage:
- UAV_Bullet_408: 1000 x 0.045 = 45; 2 hits leave 10 HP, third is lethal.
- UAV_Bullet_50CAL: 1300 x 0.045 = 58.5; second hit is lethal.
- UAV_Bullet_VULCAN: 5000 x 0.045 = 225; first hit is lethal.

These are calculations for a full-health player with direct, vest-covered torso
hits. Test with god mode off and a fresh vest; native hitzone/range effects,
other clothing and damage-altering mods can change outcomes. Other ammunition
also receives this native vest protection. No scripts replace native damage,
hitmarker or killfeed callbacks. This does not change BC_Ammo_20mm or other
20mm definitions; the supplied 20mm config specifically defines VULCAN.

rvk M100

Separate spawnable weapon: GRZ_RVK_M100. Display name: rvk M100. Description: designed by blake.
Inherits UAV_Snipex_M100_Tan from the installed original M100 weapon addon (CfgPatches UAV_M100).
The original M100 remains available. No new weapon script or replacement of original M100 config is required.
Six supplied colour and six normal textures are converted at 4096x4096 and assigned to camo through camo6.
Custom Super materials use supplied normals with restrained procedural specular defaults; original material files were not supplied.
Health remains 5000 with reskin materials kept at all original health thresholds. Other stats, including existing UAV_Ammo_VULCAN chambering, are inherited.
This item belongs in the loadout's 20mm section because it inherits VULCAN ammo.
Keep the original M100 weapon pack loaded on both client and server.
Native-size texture conversion, PBO/config/resource checks only; rendering and spawning need in-game testing.
