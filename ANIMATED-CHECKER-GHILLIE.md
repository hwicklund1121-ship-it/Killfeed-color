GRZ Animated Checker Wrap — Made by Blake

Gun-wrap class: GRZ_Animated_Ghillie_Checker
Display name: GRZ Animated Checker Wrap
VIP-only in the existing loadout ghillie selector. Requires the current GRZ_Loadout addon.
Native GhillieAtt_ColorBase model and weaponWrap slot are inherited.

Two 2048x2048 textures use the original alpha mask pixel-for-pixel.
Frame B is the exact RGB inverse of frame A: black and white swap in place.
Swap every 0.25 seconds, full A/B cycle every 0.5 seconds. Eight by eight checks.
PNG and PAA transparency is retained. The GIF has a neutral gray preview background only.

One client-cosmetic timer serves all active wraps, including local gun previews.
No dedicated-server animation timer or recurring animation RPCs. Visual phases are local to each viewer.
Timer unregisters each deleted wrap and stops when none remain.

The combined GRZ-Arena-Menu-Update.zip installer copies this addon to both client and server.
Alternatively copy only GRZ_AnimatedGhillie.pbo to @GRZ_Local/Addons on both sides, keeping GRZ_Loadout installed.
Restart server and client. Open the VIP GUN RESKINS tab and choose the checker wrap under GHILLIE WRAP.
No UAV weapon-pack repack required. Existing gun must support a native ghillie wrap.

Exact alpha/RGB inversion, decoded PAA pixels, GIF timing, asset paths and package checked.
No DayZ runtime or Enforce compiler available; verify held-gun appearance and animation in game.
