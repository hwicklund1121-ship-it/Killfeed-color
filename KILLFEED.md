# GRZ Killfeed

Separate **DayZ PC** mod, version **0.1.15**. The GRZ Custom Hud is packaged separately.

Each entry shows **killer name | actual gun model | distance | victim name** on one horizontal line. There is no gun-name text or arrow. The model includes the captured weapon attachments. Rows have plain black outlines and a 45%-opaque black background. Rows start at 580×60 pixels with 4-pixel spacing; prefixes add only their measured width, keeping names and gun apart without unused gaps. The gun viewport is 180×56 pixels with a closer model camera, distance text is 20 pixels, and both names use 24-pixel text with a thin black outline for readability. The newest five entries stay at the top right for eight seconds, fading during their last second.

The custom 1100×760 settings menu has a clean black background, red accents, the Gunrunnerz wordmark and a red GRZ Killfeed title. There are no thorn edges. Controls use white labels, and Save is highlighted in red. The tabs sit inside the main black panel and switch between separate Name and Prefix page layouts. The Name page separates its preview on the left from color and animation controls on the right. The Prefix page has only the centered word field, icon chooser and preview, with more spacing and larger buttons; it scales down to fit smaller screens. Select Primary or Secondary, then click one of twelve labeled color swatches; the active bank has a red underline and the selected swatch a white border. Style/speed controls and the Test feed/Save/Close actions are aligned beneath the palette. Players can select two of twelve colors (white, red, orange, yellow, green, blue, indigo, violet, cyan, pink, gray, black), six animation styles (steady, pulse, blink, color fade, color swap, rainbow) and three speeds. A saved style applies to that player's name as both killer and victim for every viewer.

This is an **unsigned development build**. Archive, resource and layout checks pass; DayZ compilation, networking, 3D rendering and Editor behavior have not been executed in this cloud machine.

The supplied client log confirms `/grzvip` reaches the mod and its layout initializes, but it was not remaining visible. Version 0.1.7 registers the menu with `MissionBase.CreateScriptedMenu`, opens it through `UIManager.EnterScriptedMenu`, and retains a strong reference until mission cleanup. The menu root has explicit full-screen sizing, a high draw order and placement derived from the workspace. Lifecycle logs identify Init, OnShow, first Update and OnHide. Actual display in Editor still needs verification.

## Closing and Alt-Tab

The settings menu closes when DayZ loses application focus or becomes hidden. Escape, Tab and the Close button use the same safe-close path. Controls are released immediately, even if native menu teardown waits until a later frame; guarded lock/unlock methods prevent the later OnHide from decrementing focus twice. The previous Editor cursor state is restored without resetting other menus' input focus.

Opening callbacks are canceled on dismissal, and inactive-app guards stop a delayed command from reopening the menu in the background. Reopen it with `/grzvip` after returning. Saved settings remain on the server; no automatic save is performed when closing. Alt-Tab behavior still needs testing in DayZ/Editor.

## Open and test the menu now

1. Install/load this PBO, then restart DayZ and enter your Editor mission or server.
2. Open chat, type **`/grzvip`**, and press Enter.
3. The menu opens immediately as a local preview. **No Steam-ID whitelist is needed just to open or preview it.** This is the temporary testing behavior requested by the user. F7 has been removed.
4. Select **Primary** or **Secondary**, click a color swatch, then choose the style and speed. Choose **Preview feed** to close the menu and display a private sample entry: your name, an M4 with optic/handguard/stock/suppressor/magazine, and a dummy victim, with 175 m beside the gun.

In an offline/local-host session, Save applies to local previews only. In normal multiplayer, the menu requests permission from the server after opening: an approved Steam ID enables persistent server saves; a denied request leaves the menu open in preview mode. A private preview is never broadcast as a real kill.

`/grzvip test` creates another local sample in offline/local-host testing. If Editor replaces the normal chat UI, a local chat-event fallback also recognizes `/grzvip`. The optional client launch parameter `-GRZKillfeedPreview` still auto-opens a private preview after the gameplay mission initializes. Remove it for normal play. It cannot grant server permissions.

Local preview does **not** validate the slash-command hook in every Editor variant, real kill attribution, server access, networking or persistence. Some Editor modes replace normal gameplay initialization and may need compatibility work.

## Install on server and clients

1. Extract `GRZ_Killfeed-mod.zip` and place `@GRZ_Killfeed` beside your other mods.
2. Load it on the **server and every client** with `-mod=@GRZ_Killfeed`. Do not use only `-serverMod`: the feed and menu run on clients.
3. With the HUD, use `-mod=@GRZ_CustomHud;@GRZ_Killfeed` (quote the full argument if your launcher/shell requires it).
4. Replace older versions on both sides. The weapon-snapshot RPC format requires matching server/client versions.
5. Signature-checking servers require you to sign the PBO with your own DayZ Tools key, distribute the `.bisign` beside it, and install the public `.bikey` on the server. No signing keys are bundled.

## Grant live customization access

Start the server once. It creates `GRZ_Killfeed/config.json` inside the active server **profiles** directory (`-profiles`), not the mod folder. Stop the server and edit:

```json
{
    "AllowedSteamIds": []
}
```

Place each approved player's **17-digit SteamID64 in quotes** inside that array, separated by commas, then restart. The empty list grants server-side customization to nobody. The example file in the ZIP is only a template; editing it does not grant access.

The whitelist is checked when loading and saving styles. Removing an ID and restarting reverts that player's live styling to default white, even if preferences were previously saved. Malformed config denies live customization and is preserved for repair.

Preferences are stored by Steam ID in `GRZ_Killfeed/styles.json` in the same server profiles directory, with a previous-file backup on save. Malformed stored data disables saving until repaired and the server is restarted. Every save is validated against the authenticated sender's own identity and the server whitelist; the client never supplies a Steam ID as a settings target.

## Gun images

The server captures the fatal gun's item types, attachment slots and nested parent relationships. A client builds a local inventory-preview model with that snapshot. Up to 32 items across four attachment levels are captured, including magazines. Local preview objects are nonpersistent and deleted on expiry, overflow and mission finish.

Required gun/attachment mods must be loaded on every client. Missing/unbuildable models leave player names and distance visible. Non-gun kills show no fake firearm. Wear, runtime material overrides, moving parts and ammunition state are not reproduced. Actual attachment reconstruction, framing and cleanup require in-game testing.

Only fatal sources with a player identity are credited. Suicides, infected/animal kills and environmental deaths are excluded. Delayed bleeding, disconnected attackers and detached explosives/traps without a player-owned fatal source may not appear; no killer is guessed from stale damage history. Distance is measured between player positions at death, not projectile travel distance.

## Required multiplayer checks

Use a private dedicated server with two clients and inspect server/client script logs:

- Verify compilation and `/grzvip` opening with an empty whitelist. Preview must work, but live customization must not.
- Add one real SteamID64 and restart. Verify only that player can save live styles, and that another player cannot impersonate it.
- Save a style, reconnect and restart; verify persistence. Revoke access and restart; verify default styling returns.
- Check gun, melee, fist and player-driven vehicle kills on both clients. Verify names, distance, gun models and all visible attachments.
- Check all colors/effects on killer and victim names, HUD toggle, death/respawn, reconnect and resolutions.
- Open the menu, Alt-Tab away and return repeatedly. Verify it closes, gameplay controls/cursor recover, and `/grzvip` can reopen it. Check Escape, Tab, Close, a pending command during focus loss, and returning to the Editor's existing cursor/parent menu.
- Test six quick entries, the five-row limit, expiry and removal of local preview objects. Late joins do not replay historical kills.
- Test malformed config/settings and unwritable profiles: files must be preserved and failed saves reported.

Other chat-menu/gameplay overrides can need compatibility work. RPC IDs `1647280101`–`1647280104` must not conflict with another installed mod.

## Build and package checks

From the repository root, with Python 3:

```sh
python3 tools/build_killfeed.py --output-dir downloads
python3 tools/validate_killfeed.py
```

Outputs `downloads/GRZ_Killfeed.pbo` and `downloads/GRZ_Killfeed-mod.zip`. The PBO prefix is `GRZ_Killfeed`; it contains source scripts/config, layouts and the transparent menu texture. Packaging does not compile Enforce Script or sign the addon.

To re-encode the red Gunrunnerz menu logo, install Pillow 12.3 or newer and run `python3 tools/build_killfeed_art.py` before packaging. The static validator checks PBO/ZIP payloads, resource and widget bindings, style wire-field order, and selected UI contracts. It does not execute DayZ permissions, RPCs, object creation or Editor controls.


### Prefix tab

Open `/grzvip` and select **PREFIX** at the top. Type your own word (up to 12 ASCII letters, digits, `_` or `-`; empty removes it), and cycle **Icon** to choose your custom symbol or None. The result is `icon word | PlayerName`, with no brackets. NAME opens the name page; PREFIX opens its own page with the word field and icon chooser. The name, prefix word and icon all use the same chosen colors, animation and speed, in sync. Color and animation controls are only on the Name page and edit this shared style. All 12 colors, six effects and three speeds remain available, with a combined live preview. Save and Preview kill use both settings. Existing Steam-ID access controls still govern live saves; offline Editor previews remain local.

The included custom choices are Skull, Bat, Cross, Fangs, Fanged Star, Biohazard and Crown, plus None. Custom icon artwork is bundled in the mod, with labels and texture paths in `GRZ_Killfeed/gui/prefix_icons.json`. The same updated PBO must be installed on the server and every client because the style RPC now includes prefix fields. Existing styles without prefix fields default to no icon and no word. Legacy separate prefix-animation fields are retained for saved-file/RPC compatibility but rendering uses the main name style. To add transparent PNG artwork, use `python3 tools/add_prefix_icon.py --label Skull --slug skull --image /path/skull.png`, then rebuild with `python3 tools/build_killfeed.py --output-dir downloads`. White artwork tints cleanly to every animation color. Append catalog entries; keep existing order so saved icon selections stay stable. Up to 64 packaged custom icons are supported; texture paths must reference existing assets inside this mod.

Package validation checks matching client/server style fields, widget bindings and custom icon assets. It does not compile Enforce Script or exercise DayZ multiplayer/Editor.

To regenerate the menu shell and page layouts, run `python3 tools/build_killfeed_menu.py` before packaging. The layout validator also checks that navigation belongs to the menu panel and that page controls fit inside their own containers.

Staff-style prefixes are blocked for every VIP: Admin, Owner, Mod/Moderator, Staff, Support, Helper, Manager, Dev/Developer, Founder, Operator, Official, GameMaster/GM, SuperUser and SysOp. Matching ignores case, underscores/hyphens and common numeric substitutions (for example ADM1N and M0D). Substring matching also blocks decorated titles and compounds. Existing reserved saved prefixes are omitted from live styles while keeping their colors/animation/icon. Enforcement runs in shared validation and on the server, including direct RPC saves.
