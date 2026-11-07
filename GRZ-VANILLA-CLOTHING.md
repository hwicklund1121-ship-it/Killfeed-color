GRZ Vanilla Clothing — all six items
Every description is exactly: made by blake

GRZ Surgical Gloves — GRZ_SurgicalGloves (SurgicalGloves_White)
GRZ T-Shirt — GRZ_TShirt (TShirt_Grey)
GRZ Cowboy Hat — GRZ_CowboyHat (CowboyHat_Brown)
GRZ Jeans — GRZ_Jeans (Jeans_Blue)
GRZ Bandana — GRZ_Bandana (Bandana_BlackPattern)
GRZ Sneakers — GRZ_Sneakers (Sneakers_Black)

All six colour textures are included. The supplied custom normals are included for gloves, T-shirt, cowboy hat and sneakers. Jeans and bandana currently use VANILLA normal/material detail because their normal ZIP uploads still exceed the 32 MiB transfer limit. Those two custom normals remain pending; their clothing items are usable now. Send a 2048px normal or split the original ZIP into files under 32 MiB to finish them.

The hat colour image supplied is entirely black and the bandana colour image entirely white; both are preserved exactly. The jeans texture is the supplied jeans_f_blue map, reused across the three clothing texture entries; male/female alignment still needs an in-game check. Custom normal maps use OpenGL-to-DirectX conversion and native NOHQ packing. Converted textures are 4096px; future 2048px normals are also supported.

Vanilla models, slots, storage, weight, insulation, durability, armour and wear behavior are inherited. No custom DamageSystem or stat overrides. Where a custom pristine material is supplied, inherited wear-state materials can replace it as the item degrades. Jeans and bandana retain their native materials throughout.

Vanilla DayZ only; no non-vanilla dependencies. Ordinary clothing, not VIP-only. Automatically discovered by the existing GRZ loadout clothing picker after restart. Gloves go under GLOVE, T-shirt SHIRT, jeans PANTS, sneakers SHOES; the hat and bandana retain their native attachment slots. VPP can spawn the classnames above.

INSTALL / UPDATE: close DayZ and stop the server. Replace the existing GRZ_VanillaClothing.pbo in BOTH DayZ/@GRZ_Local/Addons and DayZServer/@GRZ_Local/Addons with this new one, then restart both. Keep only one active copy per side. This replaces the previous three-item clothing pack and keeps those same classnames; do not load the old and new PBO in different active mod folders. Other mods and saved loadouts are unchanged. Do not replace or repack vanilla PBOs.

This ZIP contains the ready-to-use PBO plus source config/material files. The PAA texture assets are inside the PBO, not duplicated outside it; unpack with PBO Manager/DayZ tools for a complete editable addon. Complete source assets are also in the repository's GRZ_VanillaClothing directory. The raw PBO exceeds GitHub's file-size limit, so download the ZIP instead of a direct PBO URL.

Metadata, vanilla inheritance, resource bindings, texture conversion and PBO/ZIP integrity are checked. DayZ compilation and worn-model appearance require an in-game test.
