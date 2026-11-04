HEXGRID model-path repair

Stop DayZ and the server. Replace HEXGRID.pbo in @GRZ_Local/Addons on BOTH the DayZ game and server installations with this file. Keep your other PBOs. Restart both completely, then spawn a fresh UAV_HEXGRID and test it on the ground, in inventory/hands and worn by male/female characters.

The supplied PBO mounted at HEXGRID, while its three models, materials and textures referenced PVPPack/HEXGRID/HEXGRID/data. The repair mounts it at PVPPack/HEXGRID and retains its original internal HEXGRID/data folder. Existing script paths then resolve through the intended mount as well.

Item and patch definitions from the nested config are merged into a single root config. The redundant nested config and optional stale texHeaders.bin cache are omitted. Existing item identifiers and cargo are retained. Current downloads also apply the shared armour balance documented in GRZ-CUSTOMS.md: GlobalArmor projectile Health.damage 0.045 and vest hitpoints 10000. All three P3Ds, textures and the original animation-registration script remain byte-identical; geometry is unchanged; armour and durability are intentionally adjusted.

The ZIP contains only this replacement HEXGRID.pbo and these notes. Other mods and the full GRZ server-pack archives are unchanged.

Validation checks PBO prefix/checksum and round-trip contents, every packaged model/material/texture reference, byte-identical models/textures/scripts and ZIP integrity. There is no DayZ runtime or Enforce compiler here, so in-game visibility and compilation still require confirmation. If it remains invisible, send the latest CLIENT DayZ_x64 RPT log after reproducing it.

The original model-only repair is reproduced with tools/repair_hexgrid.py. Current balanced downloads use tools/build_customs.py --hexgrid <original HEXGRID.pbo>. GRZ-CUSTOMS-MANIFEST.json records the current build and balance; HEXGRID-REPAIR-MANIFEST.json describes the historical model-only build. Credits and original class names from the supplied addon are preserved.
