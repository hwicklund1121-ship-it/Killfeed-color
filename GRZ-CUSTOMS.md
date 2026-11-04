GRZ Customs 0.1.0

This separate reskin pack currently contains the supplied Gunrunnerz HEXGRID:
black/white artwork and matching embossed normal map, both 2048x2048 PAA.
Additional reskins can be added to @GRZ_Customs/Addons as they are supplied.
Original HEXGRID geometry, animation script, material, armour/cargo stats and
item class UAV_HEXGRID are retained. The original broken model mount is repaired.

Installation

1. Stop DayZ and the server. Extract @GRZ_Customs into BOTH the DayZ game folder
   and the DayZServer folder, beside @GRZ_Local.
2. Move any old standalone HEXGRID.pbo and its old signature out of active mod
   Addons folders on BOTH installations. Keep a backup outside active mods.
   Do not load two copies of HEXGRID. If another combined PBO also defines
   UAV_HEXGRID, resolve that duplicate before loading this replacement.
3. Add ;@GRZ_Customs to your existing server -mod argument, for example:
   "-mod=@CF;@VPPAdminTools;@GRZ_Local;@GRZ_Customs"
   Preserve all other existing launch arguments. Use -mod, since clients need
   these models/textures too. Enable @GRZ_Customs as a local mod in the client
   launcher alongside your existing mods.
4. Restart both. Spawn a fresh UAV_HEXGRID. Check ground, inventory, hands and
   male/female worn views. There is no separately renamed item to spawn.

This development pack is unsigned. For a server enforcing signatures, sign the
PBO with your own DayZ Tools key and install its public key on the server.

Validation: PAA mip dimensions and decoded colour channels, PBO checksum and
round-trip contents, internal resource paths, unchanged models/scripts/material,
and ZIP contents. DayZ rendering and script compilation require in-game testing.

Rebuild: python tools/build_customs.py --hexgrid <original HEXGRID.pbo>
Requires Pillow with DXT5 DDS encoding. Source artwork is in art/customs/hexgrid.
