# GRZ Custom Hud

Development version of a **DayZ PC** HUD mod: white heart and blood-drop artwork with black contour thorns, a white-text weapon/ammo panel with a black thorn outline, reload prompts and a stance indicator. No third-party mod dependencies are required. Status labels use DayZ's replicated notifier stages and tendency; critical states pulse opacity. No guessed numeric health/blood percentages are shown.

The weapon panel, native bullet count, fire mode, zeroing, reload/action prompts and stance/crouch indicator remain available. The quickbar, stamina, noise, other survival badges, radio readout and vehicle panel are hidden. Crosshair, menus and connection warnings remain available. This is cosmetic: hunger, thirst, temperature, bleeding and all other survival mechanics still operate, even though their indicators are hidden. Black has limited contrast against dark scenery.

## Files

- `downloads/GRZ_CustomHud-mod.zip`: installable folder containing an unsigned development PBO, launcher metadata and install notes.
- `downloads/GRZ_CustomHud.pbo`: standalone unsigned PBO.
- `GRZ_CustomHud/`: addon source; this directory is the PBO source root.
- `mod.cpp`: launcher metadata, placed beside `Addons` in the installed mod.
- `previews/health-blood.svg`: design illustration, not an in-game screenshot.
- `tools/package_source.py`: creates a source ZIP outside the checkout without requiring DayZ Tools.

## Download the PBO

Version **0.1.5** removes the weapon canvas entirely and uses a transparent outline-only PAA instead. The decoded center region is verified to have zero alpha. Icon/outline widgets explicitly enable native `SOURCEALPHA` and `BLEND` flags; image layouts use `mode blend`. The obsolete canvas layout/helper is removed. This addresses the reported filled areas, but the actual DayZ render still needs checking.

Version **0.1.4** uses the approved heart/drop artwork as transparent DXT5 PAA textures with complete mip chains. Their thorns follow the icons' silhouettes, replacing the rectangular status canvases. A separate status layout replaces the hidden native notifier panel. Custom weapon frames/canvases explicitly use blank styling and alpha blending. Texture transparency, PAA mip decoding, PBO contents and checksum are checked; actual DayZ rendering is not verified here.

`art/status-sheet.png` retains the approved artwork. `tools/build_status_textures.py` extracts and encodes the two runtime textures (requires Pillow 12.3 or newer); the packaged PAA files do not require Pillow to run. Rebuild them with `python3 tools/build_status_textures.py`, then rebuild the PBO. `art/health-preview.png` and `art/blood-preview.png` show the individual assets.

Version **0.1.3** replaces the visible stock weapon panel with a dedicated transparent frame-and-text layout. The old weapon backing is hidden entirely; no parent panels are tinted white. Vanilla setters populate the new white name/ammo/fire-mode/zeroing text. Health and blood icons/arrows are white inside larger black thorn frames (96-pixel cells, 42-pixel icons at 1080p). There are no background images in the custom weapon layout. In-game verification is still required.

Version **0.1.2** puts the vanilla weapon name, ammo, fire mode and zeroing inside the frame, makes text white, removes backing images, and gives health and blood separate larger frames with centered 38-pixel icons in 80-pixel cells at 1080p. Status sizes scale with display height. Reload prompts remain native. These presentation changes still require in-game verification.

Version **0.1.1** replaces the unavailable `CanvasWidgetTypeID` with a packaged canvas layout for weapon, health and blood frames. Replace the previous PBO; do not load both versions at once. This fixes the reported API reference; client compilation and gameplay validation remain required.

Download [`GRZ_CustomHud-mod.zip`](downloads/GRZ_CustomHud-mod.zip), extract `@GRZ_CustomHud` into your DayZ directory and add that folder as a local mod in the launcher (or use `-mod=@GRZ_CustomHud`). The ZIP already contains `Addons/GRZ_CustomHud.pbo` and `mod.cpp`.

This is an **unsigned, untested development build**. Its archive structure, prefix, source contents and checksum have been checked, but DayZ has not compiled or run the scripts here. Signature-checking servers require you to sign it with your own DayZ Tools key and install the matching public key; this download does not include a signing key or signature.

Rebuild the source-script PBO on any machine with Python 3:

```sh
python3 tools/build_pbo.py --output-dir downloads
```

The packer writes a standard uncompressed PBO with a prefix extension and SHA-1 trailer. It leaves `config.cpp` and scripts as source. It does not replace the in-game validation below.

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
