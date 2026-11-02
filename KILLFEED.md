# GRZ Killfeed

Separate **DayZ PC** mod, version **0.1.5**. The finished GRZ Custom Hud is unchanged.

Each entry shows **killer name | actual gun model | victim name**, with the **distance centered underneath the gun**. There is no gun-name text or arrow. The model includes the captured weapon attachments. Rows have plain black outlines and a 45%-opaque black background. The newest five entries stay at the top right for eight seconds, fading during their last second.

The custom settings menu has a solid black background, white text and white tribal thorn edges. Players can select two of twelve colors (white, red, orange, yellow, green, blue, indigo, violet, cyan, pink, gray, black), six animation styles (steady, pulse, blink, color fade, color swap, rainbow) and three speeds. A saved style applies to that player's name as both killer and victim for every viewer.

This is an **unsigned development build**. Archive, resource and layout checks pass; DayZ compilation, networking, 3D rendering and Editor behavior have not been executed in this cloud machine.

## Open and test the menu now

1. Install/load this PBO, then restart DayZ and enter your Editor mission or server.
2. Open chat, type **`/killfeed`**, and press Enter.
3. The menu opens immediately as a local preview. **No Steam-ID whitelist is needed just to open or preview it.** This is the temporary testing behavior requested by the user. F7 has been removed.
4. Change colors and flashing style. Choose **Preview feed** to close the menu and display a private sample entry: your name, an M4 with optic/handguard/stock/suppressor/magazine, and a dummy victim, with 175 m underneath the gun.

In an offline/local-host session, Save applies to local previews only. In normal multiplayer, the menu requests permission from the server after opening: an approved Steam ID enables persistent server saves; a denied request leaves the menu open in preview mode. A private preview is never broadcast as a real kill.

`/killfeed test` creates another local sample in offline/local-host testing. If Editor replaces the normal chat UI, a local chat-event fallback also recognizes `/killfeed`. The optional client launch parameter `-GRZKillfeedPreview` still auto-opens a private preview after the gameplay mission initializes. Remove it for normal play. It cannot grant server permissions.

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

- Verify compilation and `/killfeed` opening with an empty whitelist. Preview must work, but live customization must not.
- Add one real SteamID64 and restart. Verify only that player can save live styles, and that another player cannot impersonate it.
- Save a style, reconnect and restart; verify persistence. Revoke access and restart; verify default styling returns.
- Check gun, melee, fist and player-driven vehicle kills on both clients. Verify names, distance, gun models and all visible attachments.
- Check all colors/effects on killer and victim names, HUD toggle, death/respawn, reconnect and resolutions.
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

To re-encode the generated white menu ornament, install Pillow 12.3 or newer and run `python3 tools/build_killfeed_art.py` before packaging. The static validator checks PBO/ZIP payloads, resource and widget bindings, style wire-field order, and selected UI contracts. It does not execute DayZ permissions, RPCs, object creation or Editor controls.
