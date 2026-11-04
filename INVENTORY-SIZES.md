# GRZ Inventory Sizes

The Windows builder generates `GRZ_InventorySizes.pbo` using the installed DayZ class hierarchy:

- Vests, shirts/jackets, pants and belts get 10 columns × 10 rows of internal cargo (100 slots). Inherited inventory slots Body, Vest, Legs and Hips select the eligible types. All other clothing—including hats, shoes, gloves, masks, armbands, eyewear and backpacks—keeps its native cargo size.
- Guns get an inventory footprint 10 columns wide × 2 rows high (20 slots).

The gun change uses `itemSize`; the clothing change uses `itemsCargoSize`. Clothing's outer footprint, gun attachments/magazines, weapon stats and unrelated storage are preserved. The tool resolves original inherited cargo sizes and emits explicit normal-size restorations for excluded clothing, so a modified parent cannot accidentally give hats or other excluded types 100 slots. Native rifle/pistol/launcher descendants are covered; melee weapons and grenades are not resized.

Download/extract `downloads/GRZ-clothing-10x10-guns-10x2-builder.zip`, run `Build-Inventory-Mod.cmd`, and select the DayZ game folder containing `DayZ_x64.exe`. It reads native Addons/dta PBO config tables and writes **Desktop/GRZ-InventorySizes/GRZ_InventorySizes.pbo**. It does not change the installed game. No Python, DayZ Tools, downloads or admin rights are needed on Windows.

Stop the server and close DayZ. Copy the generated PBO into **@GRZ_Local/Addons on both client and server**, then restart both. Your existing @GRZ_Local startup line works. `config.cpp` and `Inventory-catalog.json` show the exact classes and dependencies generated from your game; these files are not required for installation.

To cover additional enabled clothing/weapon mods, pass their folders in `-ExtraModPaths` to the PS1. Only binarized, uncompressed `config.bin` resources are supported by this version. Source-only, unreadable, unsupported compressed configs and conflicting clothing/gun base classes abort the build rather than silently claiming complete coverage. Do not scan inactive alternative mods; generated dependencies must be installed on client and server. Rebuild after changing game/mod definitions. The builder excludes its own previously generated addon to avoid circular dependencies.

Back up server mission storage before changing inventory sizes. Existing packed guns may need rearranging; a 10-wide gun fits only at column zero in a 10-wide cargo grid. Removing expanded storage while items occupy extra slots can strand items. The generated development PBO is unsigned, matching the current local server setup with signature checks disabled.

Validation: actual PowerShell/C# execution against synthetic binary config/PBO fixtures, also parsing the supplied native bin.pbo during development. Tests cover inherited variants, exact size properties, original base classes, addon load dependencies, unrelated items remaining unchanged, reopening a class without its parent declaration, and refusing conflicting/unsupported input. Generated PBO bytes and SHA-1 trailer match the established Python packer. The Windows folder chooser and actual DayZ inventory/persistence still require an in-game check.

Build the downloadable tool ZIP with `python3 tools/build_inventory_builder.py`. Run its fixtures with `python3 tools/test_inventory_builder.py --pwsh <path-to-PowerShell>`. This builder is separate from the complete server ZIP because its resulting override depends on the classes installed on the user's machine.

Builder v0.1.1 reads dta engine defaults before addons, allows provider-backed updates to engine defaults, and checks parent conflicts only on relevant clothing/weapon ancestry. Normal redefinitions of unrelated classes such as Man no longer abort a build. Fixtures cover both unrelated Man redefinition and relevant engine-to-addon base replacement.

Builder v0.1.2 restricts expanded clothing to Body/Vest/Legs/Hips, preserves every excluded item’s original cargo, and retains gun footprints at 10 wide × 2 high. If neither feature appears, use `/grzvip inventory` in the updated GRZ pack: it reports whether the client loaded the generated addon and the effective configured sizes. The generated PBO must be copied to both client and server; the complete server ZIP does not contain this machine-specific generated PBO.

Builder v0.1.3 adds a second picker for explicit extra gun/clothing PBOs, with `-ExtraPboPaths` for CLI use. The user's loaded inventory report showed a modded UAV_CS5_BLACK still at 10×4 while native clothing expanded, and the server RPT identifies UAVWEAPONS.pbo in @GRZ_Local/Addons. That mod must be included so its own concrete sizes and defining addon dependencies are patched. Selected config.bin/config.cpp resources are exported under ExtraConfigs; unsupported source configs can be sent for integration without uploading all models/assets. Native extra-weapon fixtures validate the original 10×4 concrete class becomes 10×2 and its addon is a required dependency.
