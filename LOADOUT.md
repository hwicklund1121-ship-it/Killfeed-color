GRZ Loadout 0.1.3 — Made By Blake

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
Clothing tab is a disabled placeholder for later work.

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
No persistent saved choices or automatic grants on respawn yet; Save equips the current selection.
Preview entities are local-only and deleted on change/close. No hit/damage/recoil callbacks changed.

VALIDATION
Static package/API checks only. Enforce compilation, preview sizing, live inventory rollback and multiplayer sync require testing in DayZ 1.30.
Unsigned development build; sign with your own server key if required.
