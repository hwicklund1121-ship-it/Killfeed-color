GRZ Anti Hardscope v0.1.3

After 0.75 seconds continuously in the weapon optics camera, an opaque black panel covers the gameplay viewport with the red GRZ logo centered on screen. Exiting optics immediately clears it and resets the timer. Shorter quickscope aims stay clear. Re-entering optics gets a fresh 0.75-second window. The addon does not change firing, recoil or weapon animations.

It applies to weapon views reported by DayZ as IsInOptics(), including attached and built-in scopes. Ordinary iron sights, hipfire and handheld binoculars are excluded. Custom sights implemented entirely as iron-sight cameras are not detected. This is a client visual rule, not a server-enforced anti-cheat or a firing restriction. Native scopes render in different ways; 2D and 3D scopes need in-game confirmation.

The blackout covers the full gameplay viewport so peripheral vision cannot bypass it. Only the red logo is visible over the black backing; its artwork is reused from the previously supplied GRZ logo texture, at full opacity. The logo preserves its aspect ratio and is centered/sized for the current resolution. It disappears together with the blackout and is not a permanent HUD watermark. It is independent of Show HUD. Inventory/Esc/scripted menus remain visible and usable; the scope timer continues while a menu is open. Closing a menu while still scoped restores the blackout. Death, unconsciousness, dropping/switching the weapon, leaving optics and mission shutdown clear it. Cleanup releases the overlay on disconnect.

Install GRZ_AntiHardscope.pbo in @GRZ_Local/Addons on both client and server, preserving your other PBOs. Restart both. The normal @GRZ_Local startup argument includes this addon automatically.

In-game checks still required: repeatedly quickscope under 0.75 seconds; stay scoped past 0.75 seconds; un-scope after blackout; switch weapons; die/respawn; open/close inventory and Esc while blacked out; toggle Show HUD; test attached scopes, built-in scopes and your custom UAV guns at different aspect ratios. Confirm the previous HUD, hitmarker, killfeed and earmuffs still work.

For a different delay, edit GRZ_SCOPE_GRACE_SECONDS in GRZ_AntiHardscope/scripts/5_Mission/grzantihardscope.c and rebuild with tools/build_antihardscope.py. Remove only GRZ_AntiHardscope.pbo from both sides and restart to disable it. No mission or save files are modified.

Packaging and static API/lifecycle checks are available here. There is no DayZ runtime or Enforce compiler in this workspace, so actual blackout behavior is not yet verified.
