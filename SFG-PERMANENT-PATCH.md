GRZ SFG Permanent Suppressor

Copy GRZ_SFGPermanent.pbo into @GRZ_Local/Addons on BOTH game and server.
Keep the SFG weapon pack and latest GRZ_Loadout.pbo installed. Restart both.
Spawn a fresh SFG from VPP or save an SFG loadout.
Requires the original sfg50 weapon addon; does not add a second SFG weapon classname.

Supplies the suppressor config, matching custom slot, visual proxy and required model assets.
Server attaches UAV_SFG_Sup and HuntingOptic after initialization, loading and debug spawning, with retries.
An existing hunting-slot optic is preserved; the scope remains removable.
Slot locking and CanReleaseAttachment prevent normal removal. Inventory slot stays hidden.
Suppressor remains scope 1 so it cannot be selected as an ordinary separate spawn-menu item.
Corrected visual proxy has a private asset namespace and preserves original geometry bytes.
The latest loadout creates the same suppressor in its local preview.
VPP's own local model preview may omit runtime attachments; check the actual spawned gun.
Does not change weapon ammunition, damage, recoil or sound settings.
An admin or another server script can still forcibly delete entities; normal player removal is blocked.

Asset references, PBO, ZIP and source checked. Not compiled or tested in DayZ.
Unsigned build; sign with your own server key if signature verification is enabled.
