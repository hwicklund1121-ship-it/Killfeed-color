# GRZ Earmuffs

Press **N** to toggle earmuffs. While on, **−** lowers and **+** raises game effects volume in 10% steps, clamped to 10%–100%. The main keyboard =/+ and - keys and numpad +/- are bound by default. Rebind all three actions under Options > Controls > GRZ Earmuffs if needed.

Earmuffs start off; the first activation uses 20%. N switches between your selected level and normal volume, remembering your selection during the session. Percentages are relative to your normal effects volume, so 100 restores your own setting. +/- does nothing while earmuffs are off.

Each press shows a speaker symbol with the percentage to its right at the middle-left of the screen. It holds for 1.25 seconds then fades out over 0.75 seconds; another press restarts the display. The volume reduction stays active after the symbol disappears. There is no permanent watermark.

Voice chat, radio, music and speech volume sliders are untouched. No physical item or server permission is needed. The selected level lasts until death or mission exit. Earmuffs stay active in inventory, chat and the Escape menu. Hotkeys are blocked while these menus are open, so typing N/+/- cannot change the volume. The original effects volume is restored on exit/death and temporarily while unconscious or the app is inactive; the selected reduction is reapplied when play resumes. The mod does not write audio settings to disk.

Included in @GRZ_Local; replace the pack on both client and server and restart both. Standalone ZIP is also available. Build with `python3 tools/build_earmuffs.py`.

PBO/archive and API checks are performed here. DayZ compilation, actual audio mixing, N rebinding/chat/menu handling, repeated presses/fade, death/reconnect and compatibility with other sound mods require an in-game check.
