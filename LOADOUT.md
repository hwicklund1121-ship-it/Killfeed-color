GRZ Loadout 0.2.4 — Made By Blake

INSTALL
Copy GRZ_Loadout.pbo into @GRZ_Local/Addons on BOTH game and server.
Restart both. Enable @GRZ_Local in the client launcher. Press comma (,).
The key is rebindable in DayZ Controls > GRZ Loadout.

FEATURES
Large dark menu, transparent GunRunnerZ logo at top left, white accents.
Only bolt-action rifle config families are included; semi-auto guns are excluded.
AS50 and Barrett families excluded by class, display name and model path.
The SFG permanent suppressor is explicitly attached in the local preview and server grant.
If the SFG suppressor cannot be attached, Save fails before removing existing gear.
Compatible scope is selected from the gun mount slots and included in preview and equipped gun.
Magazine-fed guns also receive six full spare magazines in cargo, plus the loaded magazine on the gun.
Permanent ammo-count and inventory-replacement captions removed; operational status/errors remain.
Loadout tab: 20mm / .50 cal / .408, previous/next arrows, local 3D preview.
Gun-wrap arrows include None and installed GhillieAtt_ColorBase subclasses.
Save replaces EVERY carried gun and ALL magazines/ammo, including cargo.
New gun is loaded, compatible scope and chosen wrap attached, plus TEN FULL loose ammo stacks.
Magazine-fed guns also get SIX full spare magazines.
Non-weapon items and clothing are kept. Items attached to removed guns are removed with those guns.
Clothing tab: full character preview; left tabs HAT/MASK/SHIRT/BELT/PANTS/GLOVE/VEST/SHOES.
Click a slot to list all matching vanilla and installed custom clothing.
Clothing list names include colour/variant suffixes inferred from class and texture filenames, e.g. [Black] or [Red / White / Checked].
Skins without identifiable colour names use [Custom] for GRZ/Blake items or [Default] for other items. Use the mouse wheel over the item list to browse and preview clothing on the body without clicking.
Each wheel event selects the next/previous visible row; double-click keeps that choice.
Preview follows the highlighted list row, including native selection changes; double-click is only required to keep an item.
Wheel and double-click callbacks are registered directly on the list widget and unregistered when the menu closes.
Save & Apply ignores temporary previews and applies only kept selections.
Changing tabs or removing an item clears temporary previews. Character selection follows the same click/keep behavior.
Remove Item clears the selected preview slot. Select Character lists all available DayZ survivor characters.
Save & Apply equips the clothing now and saves the outfit/character per player for future new-character spawns.
Character body selection takes effect on next respawn; it does not kill/recreate a living player.
Outfits persist in $profile:GRZ_Loadout/players/<SteamID>.json with pending-write verification and backups.
Contents are moved to the new clothing or other worn cargo. If they do not fit, the swap cancels and restores old gear.
No old clothing is deleted before the successful transfer. Nested containers retain their contents.
Removed clothing is also emptied safely before deletion. Guns/ammo inside clothing are preserved.
Backpack, eyewear and armband are kept and included in the saved snapshot, with no extra sidebar tabs.
Clothing saves are separate from weapon loadout saves.
Custom mission scripts overriding CreateCharacter without super can bypass the survivor-body selection hook.
Custom respawn/equipment scripts running after this addon may override saved clothing and require integration.
AllInstalledClothing defaults true; set false in server config to restrict names to GRZ_ and blake_.

SERVER
First menu request creates profiles/GRZ_Loadout/config.json using your actual -profiles path.
Default AutoDiscover=true finds scope=2 bolt-action rifles from their compatible ammo configs.
Calibre names supported: VULCAN/20mm/20x102/20x110/20x138, 50CAL/50BMG/12.7x99/12.7x108, 408.
Matching checks ammo bullet class, ammo item class and display name; spaces, underscores and hyphens are normalized.
Server console logs the number of guns and wrap options found.
Custom configurations using different ammo names can be added to Guns in config.json:
{"Type":"YOUR_WEAPON_CLASS","Name":"Display name","Ammo":"LOOSE_AMMO_PILE_CLASS","MagazineType":"DETACHABLE_MAG_CLASS_OR_EMPTY","Category":0}
Category: 0=20mm, 1=.50 cal, 2=.408. Ammo must be the CfgMagazines ammo pile, not CfgAmmo bullet.
Restart server after configuration changes. Explicit entries take priority over discovery.
AmmoStacks defaults to 10. SaveCooldownSeconds defaults to 5.
All players can use the menu; server validates the requested classes against its catalogue.

SAVE BEHAVIOUR
Requires conscious living player, lowered gun, outside vehicles; non-weapon hand item must be put away.
Gun, attached wrap, chamber and ten ammo stacks are staged and validated before changing old gear.
Old guns/ammo temporarily move to an inaccessible staging location during the synchronous swap.
New gun goes into hands; all ten stacks must fit in clothing/backpack cargo.
If any move fails, new items are deleted and original gear locations are restored.
If another mod prevents restoration, preserved old items fall beside the owner and a script error is logged.
Only after all new items fit are the old guns/ammo deleted.
Weapon selections are not yet saved for respawn; saved clothing outfits are applied on new-character spawns.
Preview entities are local-only and deleted on change/close. No hit/damage/recoil callbacks changed.

VALIDATION
Static package/API checks only. Enforce compilation, preview sizing, live inventory rollback and multiplayer sync require testing in DayZ 1.30.
Unsigned development build; sign with your own server key if required.
