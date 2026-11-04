GRZ Customs 0.1.5

Fix: HEXGRID and tracksuit damage-material sections use nested config overrides
without self-referencing base declarations, preventing Undefined base class
DamageSystem at startup. Original item classes remain unchanged.

Gun wraps: 48 separate GRZ_Gunwraps_<Colour> items (36 solids and 12 mixes).
All display names match their spawn class and descriptions say Made By Blake.
Examples: GRZ_Gunwraps_Red, GRZ_Gunwraps_Black, GRZ_Gunwraps_White,
GRZ_Gunwraps_Red_Black, GRZ_Gunwraps_Cyan_Purple, GRZ_Gunwraps_Rainbow.
GUNWRAPS-ITEMS.json lists every variant. Original vanilla wraps remain available.

The wraps inherit GhillieAtt_Tan model, attachment slot, handling, stats and
native materials (including normal/specular detail). Only the colour texture
changes. The supplied native colour map's UV positions, cut-out alpha and
alpha-test flag are preserved. Mixed colours form alternating fabric patches.
They attach to weapons that already support vanilla gun ghillie wraps.
These items need the vanilla DZ_Weapons_Supports addon, not an extra Workshop mod.

Includes separate GRZ_Tracksuit_Pants item, display name GRZ_Tracksuit_Pants,
description Made By Blake. It inherits vanilla TrackSuitPants_Blue geometry,
clothing behaviour and stats, with supplied 2048x2048 colour/normal artwork.
No additional model dependency is required for these pants. Their custom
material remains selected at all damage levels.

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

This separate reskin pack currently contains the supplied Gunrunnerz HEXGRID:
black/white artwork and matching embossed normal map, both 2048x2048 PAA.
Additional reskins can be added to @GRZ_Customs/Addons as they are supplied.
Two separate spawnable items are available:
- UAV_HEXGRID: the original UAV vest with its original textures.
- GRZ_GunRunnerZ_Hexgrid: display name "GunRunnerZ Hexgrid", description
  "Made By Blake", with the supplied custom colour and normal maps.

HEXGRID.pbo supplies the original model and item, with its broken mount repaired.
GRZ_Customs_Hexgrid.pbo adds the new item through inheritance, with its own
texture/material paths. Original geometry, animation script and armour/cargo
stats are retained. Custom damage materials keep the skin as the vest wears.

Installation

Recommended: extract the entire ZIP, close DayZ and the server, and run
Install-GRZCustoms.cmd. This copies and verifies all five PBOs in the default
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
