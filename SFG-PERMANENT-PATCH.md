GRZ SFG Permanent Suppressor

Copy GRZ_SFGPermanent.pbo into @GRZ_Local/Addons on BOTH game and server.
Keep the SFG weapon pack and latest GRZ_Loadout.pbo installed. Restart both.
Spawn a fresh SFG from VPP or save an SFG loadout.
Requires the original sfg50 weapon addon; does not add a second SFG weapon classname.

Supplies the suppressor config, matching custom slot, visual proxy and required model assets.
The SFG uses an integrated visual suppressor and its existing suppressed firing sound sets directly.
A hidden, locked suppressor entity uses the standard weaponMuzzleM4 slot so native held-weapon rendering can display it.
Server equips HuntingOptic after initialization, loading and debug spawning, with retries.
An existing hunting-slot optic is preserved; the scope remains removable.
Slot locking and CanReleaseAttachment prevent normal removal. Inventory slot stays hidden.
Suppressor remains scope 1 so it cannot be selected as an ordinary separate spawn-menu item.
The gun muzzle proxy uses native ProxyAttachment rendering with weaponMuzzleM4 for both held guns and local previews.
The visual proxy and suppressor item use the same native muzzle slot. The slot is locked and hidden on the SFG.
The loadout creates that attachment before committing the equipment transaction.
The private asset namespace preserves original geometry bytes.
The latest loadout creates the same suppressor in its local preview.
VPP's own local model preview may omit runtime attachments; check the actual spawned gun.
Ammunition, damage, recoil and reload timing are preserved; firing sounds use the existing suppressed SFG sound sets.
An admin or another server script can still forcibly delete entities; normal player removal is blocked.

Asset references, PBO, ZIP and source checked. Not compiled or tested in DayZ.
Unsigned build; sign with your own server key if signature verification is enabled.
