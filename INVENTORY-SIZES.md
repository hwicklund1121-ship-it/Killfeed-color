# GRZ Inventory Sizes

The Windows builder generates `GRZ_InventorySizes.pbo` using the installed DayZ class hierarchy:

- Clothing gets 10 columns × 10 rows of internal cargo (100 slots), including wearable classes that originally had no cargo and all discovered variants.
- Guns get an inventory footprint 10 columns wide × 2 rows high (20 slots).

The gun change uses `itemSize`; the clothing change uses `itemsCargoSize`. Clothing's outer footprint, gun attachments/magazines, weapon stats and unrelated storage are preserved. Backpacks/vests/headgear/footwear/eyewear/gloves are covered when they inherit the native Clothing class. Native rifle/pistol/launcher descendants are covered; melee weapons and grenades are not resized.

Download/extract `downloads/GRZ-clothing-10x10-guns-10x2-builder.zip`, run `Build-Inventory-Mod.cmd`, and select the DayZ game folder containing `DayZ_x64.exe`. It reads native Addons/dta PBO config tables and writes **Desktop/GRZ-InventorySizes/GRZ_InventorySizes.pbo**. It does not change the installed game. No Python, DayZ Tools, downloads or admin rights are needed on Windows.

Stop the server and close DayZ. Copy the generated PBO into **@GRZ_Local/Addons on both client and server**, then restart both. Your existing @GRZ_Local startup line works. `config.cpp` and `Inventory-catalog.json` show the exact classes and dependencies generated from your game; these files are not required for installation.

To cover additional enabled clothing/weapon mods, pass their folders in `-ExtraModPaths` to the PS1. Only binarized, uncompressed `config.bin` resources are supported by this version. Source-only, unreadable, unsupported compressed configs and conflicting clothing/gun base classes abort the build rather than silently claiming complete coverage. Do not scan inactive alternative mods; generated dependencies must be installed on client and server. Rebuild after changing game/mod definitions. The builder excludes its own previously generated addon to avoid circular dependencies.

Back up server mission storage before changing inventory sizes. Existing packed guns may need rearranging; a 10-wide gun fits only at column zero in a 10-wide cargo grid. Removing expanded storage while items occupy extra slots can strand items. The generated development PBO is unsigned, matching the current local server setup with signature checks disabled.

Validation: actual PowerShell/C# execution against synthetic binary config/PBO fixtures, also parsing the supplied native bin.pbo during development. Tests cover inherited variants, exact size properties, original base classes, addon load dependencies, unrelated items remaining unchanged, reopening a class without its parent declaration, and refusing conflicting/unsupported input. Generated PBO bytes and SHA-1 trailer match the established Python packer. The Windows folder chooser and actual DayZ inventory/persistence still require an in-game check.

Build the downloadable tool ZIP with `python3 tools/build_inventory_builder.py`. Run its fixtures with `python3 tools/test_inventory_builder.py --pwsh <path-to-PowerShell>`. This builder is separate from the complete server ZIP because its resulting override depends on the classes installed on the user's machine.

Builder v0.1.1 reads dta engine defaults before addons, allows provider-backed updates to engine defaults, and checks parent conflicts only on relevant clothing/weapon ancestry. Normal redefinitions of unrelated classes such as Man no longer abort a build. Fixtures cover both unrelated Man redefinition and relevant engine-to-addon base replacement.
