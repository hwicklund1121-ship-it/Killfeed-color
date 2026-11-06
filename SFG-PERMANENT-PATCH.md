GRZ SFG Permanent Suppressor

Copy GRZ_SFGPermanent.pbo into @GRZ_Local/Addons on BOTH game and server.
Keep the SFG weapon pack and latest GRZ_Loadout.pbo installed. Restart both.
Spawn a fresh SFG from VPP or save an SFG loadout.
Requires the original sfg50 weapon addon; does not add a second SFG weapon classname.

Supplies the suppressor config, matching custom slot, visual proxy and required model assets.
The SFG uses an integrated visual suppressor and its existing suppressed firing sound sets directly.
A separate suppressor item is no longer required for loadout creation.
Server equips HuntingOptic after initialization, loading and debug spawning, with retries.
An existing hunting-slot optic is preserved; the scope remains removable.
Slot locking and CanReleaseAttachment prevent normal removal. Inventory slot stays hidden.
Suppressor remains scope 1 so it cannot be selected as an ordinary separate spawn-menu item.
The gun muzzle proxy uses simulation alwaysshow, making the suppressor model permanent on the gun, including local previews.
The permanent visual proxy has no inventory slot; firing mode uses the suppressed sound sets directly.
The loadout recognizes the integrated-suppressor config marker and does not require a separate attachment.
The private asset namespace preserves original geometry bytes.
The latest loadout creates the same suppressor in its local preview.
VPP's own local model preview may omit runtime attachments; check the actual spawned gun.
Ammunition, damage, recoil and reload timing are preserved; firing sounds use the existing suppressed SFG sound sets.
An admin or another server script can still forcibly delete entities; normal player removal is blocked.

Asset references, PBO, ZIP and source checked. Not compiled or tested in DayZ.
Unsigned build; sign with your own server key if signature verification is enabled.
