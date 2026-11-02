# GRZ Killfeed

A separate **DayZ PC** mod. The finished GRZ Custom Hud files remain unchanged.

Version **0.1.2** adds **F7** to open the menu in an offline Editor mission, even when the Editor already has a menu open. Closing the preview returns to its parent menu. Multiplayer access still requires the server's Steam-ID whitelist.

Version **0.1.1** renames the fields `Effect` to `AnimationMode` and `Weapon` to `WeaponName` because the old names collide with DayZ engine types. Replace the old killfeed PBO on both server and clients. RPC field order and the Steam-ID whitelist format are unchanged. This fixes the reported type-name conflict; full DayZ compilation is still not available in this cloud machine.

- Top-right player killfeed: killer > victim, fatal weapon/source name, rounded player-to-player distance in meters.
- Up to five entries, newest first; each lasts eight seconds and fades during its last second.
- Transparent black thorn border and white default names. No filled feed background.
- `/killfeed` opens personal name styling after the server verifies access.
- Two colors from twelve choices (white, red, orange, yellow, green, blue, indigo, violet, cyan, pink, gray, black), six effects (steady, pulse, blink, color fade, color swap, rainbow cycle), and three speeds.
- Custom black settings menu, white labels/buttons, and white tribal thorn edges matching the HUD. The username preview displays its selected animated colors.
- A name's chosen style appears for everyone, whether that player is killer or victim.
- Only server-approved Steam IDs can save styles. Other players still see the killfeed.

This is an **unsigned development PBO**, not a DayZ-tested release. Archive/layout/API checks have been performed; this cloud machine cannot compile or execute Enforce Script or run DayZ Editor.

## Install on the server and clients

1. Extract `GRZ_Killfeed-mod.zip`. Place `@GRZ_Killfeed` beside your other mods.
2. Load it on the **server and every client** using `-mod=@GRZ_Killfeed`. Do not use only `-serverMod`, since the UI is client-side.
3. To run with the HUD: `-mod=@GRZ_CustomHud;@GRZ_Killfeed` (quote the full argument if your launcher/shell requires it).
4. Signature-checking servers require you to sign the PBO using your own DayZ Tools key, distribute the `.bisign` beside it, and install the public `.bikey` on the server. No signing keys are bundled.
5. Start the server once. It creates `GRZ_Killfeed/config.json` under the server's active **profiles** directory (`-profiles`), not inside the mod folder.

## Give players access

Stop the server and edit its profiles file `GRZ_Killfeed/config.json`:

```json
{
    "AllowedSteamIds": []
}
```

Put each approved player's **17-digit SteamID64 in quotes** inside that array, separated by commas, then restart the server. The shipped empty list grants customization to nobody. The example file inside the mod ZIP is only a template; changing it does not grant access.

The config is read on server startup. Removing an ID and restarting disables that player's special styling, including previously saved preferences. Malformed config denies customization and is preserved for repair.

Styles are stored by Steam ID in `GRZ_Killfeed/styles.json` in the same server profiles directory. Saving retains a backup of the previous file. Invalid stored data disables saves until repaired and the server is restarted. No Steam ID is accepted from the client as a settings target: the server uses the authenticated sender's own identity and validates every option.

Players press the chat key, enter `/killfeed`, and submit. The command is intercepted locally rather than sent as a normal chat message. Approved players get a live preview; **Save** persists on the server, and **Preview in feed** shows a local sample without sending a fake kill to anyone else. Unapproved players get an access-denied message.

## Test with DayZ Editor

Load both DayZ Editor and `@GRZ_Killfeed` in the DayZ launcher, and start an **offline** editor mission with a player and the normal HUD/chat available:

1. Press **F7**, or enter `/killfeed` in chat. In an offline mission this opens a clearly marked local preview menu without needing a Steam whitelist. F7 is reserved for this preview while the mod is loaded offline; it is not intercepted in multiplayer.
2. Cycle colors, effect and speed. **Save** applies only to that offline preview session; it does not modify server settings.
3. Choose **Preview in feed** to close the menu and show a local sample entry.
4. Enter `/killfeed test` to show another local sample using the current preview style. Verify placement, transparency, clipping, five-entry cap and eight-second expiry.

Offline previews **do not validate real kill attribution, networking, server authorization or persistence**. `/killfeed test` is disabled in multiplayer. The authorized menu's preview button can still show a private local sample on a multiplayer server.

Editor launch modes differ: if your editor session reports itself as multiplayer, it uses the normal server-controlled path. A mode that replaces the standard chat menu may intercept commands first. Test in an offline mission with standard chat, or use the multiplayer procedure below.

## Required multiplayer validation

Use a private dedicated server with two clients and inspect server/client script logs:

- Confirm the PBO compiles and the config appears under the active profiles directory.
- With an empty whitelist, confirm menu access is denied while ordinary killfeed entries still display.
- Add one actual SteamID64 and restart. Confirm only that player gets the menu; another player cannot change its style or impersonate its identity.
- Save a style, reconnect and restart the server; confirm the preference persists. Remove access and restart; confirm it reverts to plain white names.
- Check gun, melee, fist and player-driven vehicle kills on both clients, with correct names, source and plausible distance. Confirm suicides, infected/animal kills and environmental deaths are excluded.
- Check all effects on killer and victim names, death/respawn, reconnect, HUD toggle, resolution changes and compatibility with the HUD mod.
- Test six quick kills, expiry and late joins; entries are live events, not history replay.
- Test malformed config/settings and unwritable profiles: config is preserved, access fails closed, and failed saves are reported.

Only fatal sources with a player identity are credited. Delayed bleeding, disconnected attackers and detached explosives/traps without a player-owned fatal source may not appear; the mod does not guess a killer from stale damage history. Distance is between player positions at death, not projectile travel distance.

Native kill handling follows DayZ's `EEKilled` source ownership, not client-supplied events. RPC IDs `1647280101`–`1647280104` must not conflict with another installed mod. Other chat-menu overrides may require compatibility work.

## Build

From the repository root, with Python 3:

```sh
python3 tools/build_killfeed.py --output-dir downloads
python3 tools/validate_killfeed.py
```

Outputs `downloads/GRZ_Killfeed.pbo` and `downloads/GRZ_Killfeed-mod.zip`. The PBO has prefix `GRZ_Killfeed` and contains source scripts/config plus the transparent texture. Building the PBO does not compile Enforce Script or sign it. No extra runtime mod dependency is required.

To re-encode the generated white menu-border artwork, install Pillow 12.3 or newer and run `python3 tools/build_killfeed_art.py` before packaging. The menu's text/control rectangles were checked against the scaled border alpha to avoid overlapping thorns. The validator checks the actual PBO/ZIP, layout bindings, resources and style wire-field order; it does not test server authorization or Enforce compilation.
