# GRZ Autorun v0.12 update

Holding or toggling native freelook no longer cancels autorun. Both Alt keys and remapped UALookAround/UALookAroundToggle bindings are allowed. Autorun still overrides only movement speed and angle; it does not override the camera/look inputs. Existing Q/E lean, permitted inventory/map/chat/LB screens and ordinary movement-key cancellation are preserved. The UAGRZAutorunToggle binding name is unchanged, so existing bindings survive.

This is an update to the user-supplied v0.11 LB Screens TEST sources. Original reference notices and license are preserved below/in the accompanying files. Verify Alt hold/release, toggle/remapped freelook, lean, manual movement cancellation and menu behavior in-game.

---

# GUNRUNNERZ AUTORUN v0.11 — inspected LB screen support

Targets the supplied LBmaster_Groups, LBmaster_Leaderboard and LBmaster_Groups_ATM PBOs. They were inspected read-only. None of their assets, configs, scripts or PBOs is modified or included in this ZIP. This release contains only GRZ_Autorun.

## Concrete compatibility changes

- Recognizes the actual LBGroupUI (map and group pages), LBLeaderboardGUI and LBATMMenu classes before vanilla numeric menu-ID checks. These names are present in the supplied source. Marker editing is managed within LBGroupUI; transfer/PIN/account controls are managed within LBATMMenu. Nested menus with an allowed parent are also recognized.
- Recognizes the actual UALBMGroupOpenMap, UALBMGroupOpenMapGroup and UALBMOpenLeaderboard opening actions. The previous generic detector did not explicitly cover all these classes/actions.
- For the selected/running ActionOpenATMMenu plus the default interaction input, grants up to two seconds for its server-requested menu data to arrive. Only a no-menu interval is exempted; unrelated menus remain blocked. No ATM access, distance, credit-card, PIN, withdrawal/deposit or other server validation is changed. A timeout or native action that raises the weapon/leaves the move command can still stop autorun.
- Removes the previous recursive widget/MapWidget/focus probing and loose name heuristics. UI support is now based on native menu IDs and the inspected LB class/action names. No claim is made that those previous probes caused the reported crashes.

Keeps v0.10 running takeover (hold Shift+W, press autorun, release Shift+W), chat typing, inventory Tab, Q/E lean exceptions, full forward sprint, silent HUD behavior and the existing server ownership/heartbeat/lease. Manual movement and other fresh keys outside allowed UI still stop. Pause/settings/main/loading/logout retain their stop behavior. Native death/unconsciousness/restraint/raised/non-move checks remain.

No hard LB script-class dependency is introduced: runtime names/action queries are used, so the addon can still load without these LB mods. The supplied versions' classes are covered; subclasses/other LB builds or custom popup parent relationships require runtime verification. Queries for absent actions are null-safe. A single [GRZAutorun v0.11] startup line identifies this build in script logs; no chat/notification is shown.

## Install and verify

Replace the previous GRZ_Autorun.pbo on BOTH server and clients, using normal shared -mod. Keep exactly one copy. Back up the previous PBO/signature outside loaded addons and re-sign the changed PBO normally; no signature is included. Keep all three LB PBOs unchanged in their existing installation. Do not add this beside another autorun version. No movement/stamina/scope/LUGU change or persistence wipe is needed.

Test on a controlled deployment first: start while sprinting; open/close the LB map and group page using their current bindings; pan/zoom; create/name/edit/delete markers; open/close leaderboard; interact with an ATM, wait for server response, type PIN/amount/name, use personal/group/transfer pages and close. Verify forward movement persists where native action eligibility allows it, and that text keys (including W/A/S/D and the autorun key) do not cancel while inside those screens. Test chat, inventory and Q/E lean; then confirm a fresh movement key outside UI, pause/settings, raised weapon, death and loss of heartbeat still stop. Inspect another player's view for server agreement/run-in-place regressions. Native banking validation remains enforced and may reject transactions as you move away.

The source comparison confirms distinct UI hooks and unchanged server protocol/HUD/input resources. PBO/source/ZIP checks and SERVER preprocessing pass locally. Enforce compilation, LB input-ordering, action selection timing, custom dialog behavior and actual multiplayer movement are NOT verified here. Prior inventory errors and native login crashes are not claimed fixed by this release. If an opening still cancels, send that session's client script log (check the v0.11 startup line), the exact opening key/action, and which screen. Rollback restores the prior PBO and matching signature.

Earlier version notes below are historical where superseded above.

## v0.10 — activate while already running

Press the autorun binding while holding W/Shift (or other directional movement) to transfer to full forward autorun. Directional actions already held at activation are temporarily exempt from the movement-input cancel check until each is released. Releasing those initial keys does not stop the run. Pressing a movement key again after release stops normally; a newly pressed direction that was not held at activation stops normally too. The native full-sprint input overrides and server protocol are unchanged; this does not preserve an initial strafe direction.

The prior v0.9 code could start and immediately cancel because W was still held. This change captures the initially held movement actions when starting. Fresh-key detection still suppresses repeat events for already held keys. Keep the autorun key bound separately from W/A/S/D, Q/E or Tab; conflicting bindings can defeat expected stop/start behavior.

Chat, map/marker UI, Tab/inventory, Q/E exceptions, silent operation, HUD guard, server heartbeat/lease and native state stop checks remain as in v0.9. LBmaster UI and custom bindings still require runtime verification. Only the autorun addon changes.

Replace the old autorun PBO with v0.10 on both client/server via shared -mod; one copy only. Back up the prior PBO/signature outside loaded addons and re-sign normally. Test: hold Shift+W, activate autorun, continue holding for several seconds, release W/Shift and verify sprint continues; press W again to stop. Repeat with the autorun key to stop, and with an unrelated key. Check starting from rest, chat/inventory/map/markers, Q/E lean, menu/death/reconnect, and remote player movement. Confirm no return of running in place. Rollback restores the previous PBO/signature; no persistence wipe.

PBO/source/ZIP integrity and SERVER preprocessing are checked locally. Enforce compilation and running takeover have NOT been tested in DayZ.

Earlier version notes are historical where superseded above.

## v0.9 — chat/map/marker UI test

Keeps an existing autorun active while typing in native chat, using inventory, and using map/marker UI. Q/E/Tab exceptions from v0.8 remain. Ordinary keys outside permitted UI still stop autorun. The key handler now defers a stop decision until after native mission/UI input is processed, so opening chat/a map is not immediately canceled by that opening key. Keys pressed while permitted UI is already open do not cancel, including letters used in chat/marker names and keys used to close that UI. Manual directional-input cancellation is suspended while the permitted UI has focus; normal directional cancellation returns after it closes.

Native inventory/chat/map menu IDs are recognized. Custom map menus are recognized through visible MapWidget children, parent-menu relationships, or LB-named map/marker classes. A focused widget map overlay is also supported without importing/repacking LBmaster or adding a dependency on its classes. **The actual LBmaster/GunRunnerZ map implementation was not supplied, so its menu/marker naming and focus behavior are not runtime-verified.** A standalone marker popup with no recognizable map relationship may still stop autorun; provide its client log/map script details if so. Custom chat menus need similar verification.

Pause, options, main/loading/logout menus still stop autorun. Escape used to close permitted UI can leave the run active; opening pause with Escape outside it stops. Starting inside restricted UI remains blocked. Death/unconsciousness/restraint, weapon raise and leaving the move command still stop; inventory/map actions that trigger those states may therefore stop it. This does not force native controls enabled inside menus.

Server protocol, fixed speed/angle, ownership checks, heartbeat/lease, silent operation, Unsorted keybind and stamina HUD guard remain unchanged. Sprint leaning still depends on GRZ Movement permitting it. No movement/stamina/scope/LUGU or LBmaster file is bundled or changed.

## Install and test

Replace the previous GRZ_Autorun PBO on both clients/server via normal shared -mod. Keep only one copy. Back up old PBO/signature outside loaded addons; use your normal signing workflow. Bind autorun to a key other than Q/E/Tab. Test in a controlled session: start autorun; open chat, type a sentence including W/A/S/D and the autorun key, submit; open the LB map with its actual binding; pan/zoom; create, name, edit and delete a marker; close and reopen; repeat inventory Tab and Q/E leaning. Verify each action retains forward sprint and server movement matches. Check an ordinary key outside UI, W/A/S/D outside UI, pause/settings, weapon raise, death/respawn and lost heartbeat still stop. Test marker popups and holding a text key while closing a menu. No speed-selection setting or on-screen notification is added.

Local checks cover PBO/ZIP/source integrity, SERVER preprocessing and unchanged protocol/HUD/input files. DayZ compilation, focus timing, chat/marker entry and LBmaster compatibility require runtime validation. Do not assume packaging proves those behaviors. Rollback restores the previous PBO and matching signature on both sides; no persistence wipe.

Previous version notes are historical where superseded above.

## v0.8 — explicit Q/E lean exceptions

Fix candidate for the owner's v0.7 report that Q/E still cancel autorun. The exact runtime cause was not logged. Q, E and Tab are now checked directly as physical key codes BEFORE any profile/action binding lookup and cannot cancel autorun through its fresh-key handler. This does not depend on lean inputs being unlocked or the legacy profile API mapping those keys correctly. Rebound lean keys still use the v0.7 profile lookup; their behavior remains unverified.

Because the explicit exceptions have priority, do not use Q, E or Tab as the autorun start/stop binding. If already bound there, it can start but that same key will not stop it; another non-exempt key still stops. Bind a different key under Controls > Unsorted. Manual movement cancellation and other state checks still apply, including if Q/E are also bound to a directional movement or raising action.

Inventory continuity from v0.7 remains. Full sprint, silent operation, HUD guard and server RPC/lease are unchanged. The actual sprint lean animation continues to depend on GRZ Movement permitting it; this addon does not force-unlock lean inputs.

Replace v0.7 (or v0.6) on both client and server via shared -mod; only one GRZ_Autorun PBO may be loaded. Back up the prior PBO and matching signature outside loaded addons; re-sign this changed PBO with your normal workflow. Test start > Q held > E held > alternating Q/E > release > Tab open/close > stop with another key. Confirm forward sprint continues throughout leaning, key-repeat does not stop, and the chosen autorun key still stops without restarting. Also check W/A/S/D, Escape, inventory interactions, aim/raise, death/reconnect and server movement agreement. If Q/E still stop it, provide that session's client script log and loaded-mod list so another copy or another stop path can be identified.

Package/source checks and SERVER preprocessing are local checks only. DayZ compilation and the Q/E fix are NOT runtime-verified. Rollback restores the prior PBO/signature; no persistence wipe. No movement/stamina/scope/LUGU file is included or changed.

Earlier version notes are historical where superseded above.

## v0.7 — inventory and lean exceptions

While autorun is active, Tab and keys assigned to lean left/right no longer cancel it. Lean keys are read from the current input profile using DayZ's action-key API; Q/E are not hard-coded. If a key is also assigned to the autorun action, that action retains its stop behavior. Key-repeat handling and fresh-key detection are unchanged. Custom/combination bindings need runtime verification.

The normal inventory screen (MENU_INVENTORY) may remain open during an already active autorun, including its normal input suppression. Closing it with Tab also keeps the run active. Autorun cannot be started inside inventory. Other menus, pause/settings/chat, other fresh keyboard keys, manual directional movement, raised weapon, death/unconsciousness/restraint and leaving the move command still cancel. Movement overrides and the existing server heartbeat continue while inventory is open. If inventory actions themselves raise the weapon or start a non-movement command, those existing checks still stop the run. Do not assume a custom inventory UI with a different menu ID is supported.

This change does not force-enable or unlock lean inputs. GRZ Movement already permits sprint leaning. Vanilla/other movement mods may lock lean during sprint even though this autorun addon no longer cancels on the lean key. No lean/animation code from other mods is copied here.

Full native sprint, the Unsorted GUNRUNNERZ AUTORUN binding, silent operation, stamina-bar visibility guard, fixed speed/angle, server ownership validation, 1-second heartbeat and 3.5-second lease are retained. GRZ_ActionStamina still provides punch/breath-only drain. No movement, stamina, scope or LUGU PBO is included or changed.

## Install/test/rollback

Replace the v0.6 GRZ_Autorun PBO with this build on both server and clients via normal shared -mod. Keep only one autorun copy. Back up the old PBO/signature outside loaded addons and re-sign the changed PBO through your normal signing workflow. The included PBO is unsigned.

Test: start autorun, open/close inventory with Tab several times, hold lean left/right, alternate lean, then release lean. Rebind lean to other simple keyboard keys and repeat. Check that an unrelated key and the autorun key still stop, and pressing the autorun key to stop cannot restart it that frame. Test W/A/S/D while leaning, Escape/settings, inventory interactions, raising a gun, menu opening by mouse, death/respawn, reconnect, loss of heartbeat and server/client agreement. Verify no run-in-place regression or stamina HUD flashing. Check starting inside inventory remains blocked. Profile key-code mapping, inventory update timing and custom bindings require actual DayZ tests.

Local verification covers package payloads/integrity, unchanged server protocol/input XML/localization/HUD code, and client/SERVER preprocessing. DayZ compilation, inventory autorun, rebound lean keys and multiplayer behavior are not verified here. Rollback restores v0.6 PBO and matching signature on both sides; no persistence wipe.

Previous version documentation below is historical where changed by v0.7.

## v0.6 — shared input-state test

Press the rebindable GUNRUNNERZ AUTORUN key once to start normal full sprint. Any fresh keyboard key press afterward stops it, including the same key. Held-key repetition is ignored. There is no speed setting, speed key, notification, chat ping or continuous log output. Bind the existing action under Controls > Unsorted (runtime display verification required). Old speed JSON files are ignored.

## Correction from v0.5

v0.5 applied input overrides only locally. v0.6 applies matching fixed native speed/angle overrides on the owning player's server entity via a dedicated reliable state RPC. The server accepts only that entity's authenticated owner and a boolean start/stop state; no client-selected speed, angle, position or velocity is accepted. Speed is fixed at native 3.0 and forward angle at 0.0. No position or velocity is written and native sprint restrictions remain in place.

A one-second active heartbeat renews a 3.5-second server lease. Stop clears local owned input immediately and sends stop to the server. The server also clears its input on lease expiry, death/unconsciousness/restraint/raised state or leaving the move command. There are no call-queue timers, no per-frame RPCs, no changes to engine reconciliation and no promise of general desync repair. RPC traffic/delivery and input behavior must be checked in DayZ. Packets in flight may delay the remote stop until delivery or lease expiry.

This corrects a concrete omission, not a verified root-cause diagnosis of every run-in-place symptom. API declarations and the public Carim/Schana autorun architecture were inspected as references; the original third-party addon is not bundled. Attribution and license notices are in the ZIP.

## Existing behavior

Manual movement also cancels as a fallback. Menus/disabled controls, raised hands/weapon, death, unconsciousness, restraint or leaving the normal movement command stop autorun. Mission initialization/reconnect starts OFF. A native HUD visibility guard suppresses stamina-bar dimming while active without changing its amount or forcing the parent HUD visible. Custom HUD behavior is unverified.

This addon does not change stamina costs. Keep the existing GRZ_ActionStamina addon for empty-hands punch/breath-only drain. Existing movement/FOV/LUGU files are not part of this download and are not changed.

## Install and rollback

Stop client AND server. Back up the old GRZ_Autorun PBO/signature outside all loaded addon directories. Replace it with this exact build on BOTH sides, distributed via shared -mod; do not load duplicate versions or another autorun addon. Installing only on the client cannot supply the new server RPC handler. Use the owner's normal signing/public-key distribution workflow where required; do not disable verification.

Release manual movement keys, start autorun, and verify forward sprint rather than running in place. Stop with an unbound keyboard key, modifier, the autorun key and movement keys. Test menus, aim, jump/vehicle transitions, death/respawn and reconnect. Check that cancellation cannot restart in the same frame, that server movement stops after loss of heartbeat, and that stamina/punch/breath and HUD behavior are preserved. Do this on the test deployment first.

Rollback: stop both, restore the previous PBO/signature or remove this addon from both shared launch lists, restart. No persistence wipe is required.

Local validation covers PBO checksum, compressed payload/source round-trip, ZIP CRC, XML/localization consistency, source rebuild and client/SERVER preprocessing of the shared protocol. Enforce compilation, live server authority/ownership checks, heartbeat timing, sprinting and animation behavior remain unverified without DayZ.
