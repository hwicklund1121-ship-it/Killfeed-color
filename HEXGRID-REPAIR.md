HEXGRID model-path repair

Stop DayZ and the server. Replace HEXGRID.pbo in @GRZ_Local/Addons on BOTH the DayZ game and server installations with this file. Keep your other PBOs. Restart both completely, then spawn a fresh UAV_HEXGRID and test it on the ground, in inventory/hands and worn by male/female characters.

The supplied PBO mounted at HEXGRID, while its three models, materials and textures referenced PVPPack/HEXGRID/HEXGRID/data. The repair mounts it at PVPPack/HEXGRID and retains its original internal HEXGRID/data folder. Existing script paths then resolve through the intended mount as well.

Item and patch definitions from the nested config are merged into a single root config. The redundant nested config and optional stale texHeaders.bin cache are omitted. Existing item identifiers and clothing/armour settings are retained. All three P3Ds and the original animation-registration script remain byte-identical; no geometry or gameplay stats are edited. The current download also includes the supplied Gunrunnerz custom colour and normal maps, converted to 2048x2048 PAA. The original material and specular map are retained.

The ZIP contains only this replacement HEXGRID.pbo and these notes. Other mods and the full GRZ server-pack archives are unchanged.

Validation checks PBO prefix/checksum and round-trip contents, every packaged model/material/texture reference, byte-identical models/scripts/material and ZIP integrity. There is no DayZ runtime or Enforce compiler here, so in-game visibility and compilation still require confirmation. If it remains invisible, send the latest CLIENT DayZ_x64 RPT log after reproducing it.

The initial model-only repair can be reproduced with tools/repair_hexgrid.py. For the current reskinned download, use tools/build_customs.py --hexgrid <original HEXGRID.pbo>; GRZ-CUSTOMS-MANIFEST.json records its hashes. HEXGRID-REPAIR-MANIFEST.json describes the historical model-only build. See GRZ-CUSTOMS.md for installing this PBO in the separate @GRZ_Customs pack instead. Load only one copy. Credits and original class names from the supplied addon are preserved.
