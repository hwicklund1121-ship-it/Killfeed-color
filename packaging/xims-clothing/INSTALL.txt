XIMS Clothing — all four items
XIMS Sneakers — XIMS_Sneakers
XIMS Jean Shorts — XIMS_JeanShorts
XIMS Layered Shirt — XIMS_LayeredShirt
XIMS Canvas Bag — XIMS_CanvasBag
All descriptions: Made By Blake

GRZ_XimsClothing.pbo contains the vanilla-base sneakers, jean shorts and canvas bag. GRZ_XimsLayeredShirt.pbo requires ORIGINAL Windstride Clothing loaded on BOTH server and client (verified addon dependency Layered_Shirt, parent Layered_Shirt_Base). The layered shirt inherits the original ground/male/female models; its zbytek selection and texture/material ordering are verified against the supplied Models/Layered_Shirt/config.cpp. No female MLOD recompilation or original model replacement is needed.

Supplied 4K colours are included for all four items. The jean shorts and layered shirt include custom baked normals converted from OpenGL to DirectX. Sneakers retain native normal/material detail because the supplied export normal is flat. Canvas bag uses its supplied painted colour plus VANILLA normal/material detail: the custom normal ZIP still exceeds the 32 MiB transfer limit. Resend that normal at 2048px or split the ZIP into parts under 32 MiB to finish it. Shorts are all black exactly as supplied. No model, storage, inventory slot, weight, insulation, durability, armour or DamageSystem overrides. Native wear-state materials remain inherited.

INSTALL: close DayZ and stop the server. Copy BOTH PBOs into client and server @GRZ_Local/Addons, then restart both. Keep one active copy of each PBO per side. Clothing appears in the existing loadout SHOES/PANTS/SHIRT/BAG categories with AllInstalledClothing enabled (default); ordinary clothing, not VIP-only. Original Windstride files must already be installed and loaded. Never replace vanilla or Windstride PBOs. Config/material sources are included; unpack the PBOs for full texture assets.

Use the GRZ-All-Reskins-Update installer to combine these with prior GRZ clothing, CSR50, CS5, animated reskins and gameplay changes automatically. The installer does not download Windstride itself. Existing older GRZ Jeans/Bandana custom normals are still pending; those two items retain vanilla normals.

Package/resource and local PowerShell installer checks only. Live DayZ compilation, worn appearance and network downloads need an in-game check.

The combined installer checks for the original WindstrideClothing.pbo or Layered_Shirt.pbo under each side's @mod/Addons folders and the corresponding Steam Workshop content/221100 folder before installing the layered shirt. A .bisign alone is not enough. For external mod paths, pass -WindstrideServerPbo and -WindstrideClientPbo with the actual PBO paths, and keep that mod loaded. Use -SkipLayeredShirt if you only want the other updates. The installer does not change launcher/server mod arguments.
