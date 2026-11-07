GRZ VEST/TOP/PANTS/BELT 10x10 + GUNS 10x2 BUILDER v0.1.3

1. Close DayZ and extract this ZIP into a folder.
2. Double-click Build-Inventory-Mod.cmd (or run cmd /c .\Build-Inventory-Mod.cmd).
3. Select the DayZ GAME folder, then select extra weapon/clothing PBOs
   such as UAVWEAPONS.pbo. Cancel the second picker for vanilla-only items.
4. It produces Desktop\GRZ-InventorySizes\GRZ_InventorySizes.pbo.
5. Copy that PBO into @GRZ_Local\Addons on BOTH client and server; restart both.

Uses built-in Windows PowerShell/.NET. No Python, DayZ Tools, admin rights,
software downloads or network access are needed. Reads installed binarized
config tables so individual item overrides are covered, rather than guessing
item names or changing only the base class. See INSTALL.txt for details.

Version 0.1.4 reads nested config.bin/config.cpp resources, including Windstride Models folders, and prioritizes binary configs over source siblings. Standalone source configs are supported; unsupported preprocessing and compressed config entries fail before producing an override. Slot/cargo inheritance and existing exclusions still apply.
