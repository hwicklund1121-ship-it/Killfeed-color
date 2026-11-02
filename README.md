# GRZ Custom Hud

First source version of a **DayZ PC** HUD mod: health and blood survival indicators, a weapon/ammo panel, reload prompts and stance indicator, with black styling and thin black tribal-thorn frames. There are no third-party mod dependencies or downloaded textures. Critical health/blood pulses in opacity; vanilla status image levels and arrows still provide the status information.

The weapon panel, native bullet count, fire mode, zeroing, reload/action prompts and stance/crouch indicator remain available. The quickbar, stamina, noise, other survival badges, radio readout and vehicle panel are hidden. Crosshair, menus and connection warnings remain available. This is cosmetic: hunger, thirst, temperature, bleeding and all other survival mechanics still operate, even though their indicators are hidden. Black has limited contrast against dark scenery.

## Files

- `GRZ_CustomHud/`: addon source; this directory is the PBO source root.
- `mod.cpp`: launcher metadata, placed beside `Addons` in the installed mod.
- `previews/health-blood.svg`: design illustration, not an in-game screenshot.
- `tools/package_source.py`: creates a source ZIP outside the checkout without requiring DayZ Tools.

## Build on Windows

1. Install DayZ and **DayZ Tools** through Steam. Set up the DayZ Tools project drive according to its tools documentation.
2. Copy `GRZ_CustomHud` onto the project drive as `P:\GRZ_CustomHud`. Keep the folder name and `$PBOPREFIX$` unchanged.
3. Open **Addon Builder**. Set source to `P:\GRZ_CustomHud` and destination to a new `@GRZ_CustomHud\Addons` directory. Enable binarization. Do not include the repository root as the addon source.
4. Build `GRZ_CustomHud.pbo`. Copy the repository's `mod.cpp` into `@GRZ_CustomHud\mod.cpp`.
5. For a signature-checking server, generate your own private signing key with DayZ Tools, sign the PBO, distribute the `.bisign` beside the PBO, and put the public `.bikey` in the server's `keys` directory. Keep the private key private.
6. Load the mod on the PC client using the launcher or `-mod=@GRZ_CustomHud`. The server must allow the same signed mod; include it in the server's normal `-mod` list when distributing it to players. Do not use only `-serverMod`: the HUD runs on clients.

This is a PC script mod, not a console-compatible server configuration.

## In-game validation required

This Linux cloud machine has no DayZ runtime, Enforce Script compiler or Windows Addon Builder. The source has **not** been compiled or tested in DayZ. Do not upload it to Workshop as a verified release until these checks pass:

- Compile/build using the installed game's scripts; inspect the client script log for errors.
- Join a multiplayer server. Check full, reduced and critical health/blood against vanilla status changes. No client-side health RPC or guessed blood percentage is used.
- Confirm only health/blood survival icons remain, including after inventory changes, sprinting, vehicle entry and respawn. Check the weapon panel, bullet count, reload prompts and standing/crouching/prone indicator remain visible and update correctly.
- Check arrows and critical opacity pulsing, HUD toggle, menus, unconsciousness, reconnect, 1080p/1440p/ultrawide and HUD scaling.
- Check door/pickup prompts and network warnings remain usable. Test without other HUD mods first; other overrides of these classes may conflict.

The integration was checked against Bohemia Interactive's public `DayZ-Script-Diff` `IngameHud`, `ItemActionsWidget` and native widget API. Compatibility with your installed DayZ version still needs the checks above.

## Development

Use the existing checkout; each cloud task is already isolated. No worktree is needed. Source editing and source packaging require only Python 3:

```sh
cd /workspace/Killfeed-color
python3 tools/package_source.py --output /tmp/GRZ_CustomHud-source.zip
```

There is no application server or package installation step. PBO compilation and actual gameplay validation require DayZ Tools and DayZ on Windows.
