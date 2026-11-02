# GRZ Hitmarker

Separate DayZ PC mod, development version 0.1.1. Install on the server and every client. Settings live in **Options > Game > GRZ HITMARKER**, appended to the existing game settings list, using native left/right option selectors. The existing General, Gunrunnerz FOV, HUD and Chat sections remain intact. No slash command or VIP access is needed.

Controls: show hitmarkers, show damage info, hitmarker sound, highlight kills, normal-hit color, headshot color, size (25–150%, default 50%) and volume (0–100%, default 100%). Colors include white, red, orange, yellow, green, blue, indigo, violet, cyan, pink, grey and black. A live preview and Test Hit/Test Headshot/Preview Kill buttons use the pending selections. Press the normal Options Apply button to save; Cancel/Revert restores saved selections and Defaults restores the hitmarker defaults. Client preferences are saved in `$profile:GRZ_Hitmarker/settings.json`.

The centered X briefly appears only for confirmed hits against players, infected and animals. The server captures living-target health before applying damage, confirms the hit afterward, and sends feedback only to the attacking player's client. The damage number represents global health lost, which can be zero for shock-only damage. Head/brain zones use the headshot color. Optional kill highlighting makes the X larger and adds KILL text; unconsciousness alone is not a kill. Corpse hits, self-hits, scenery and other players' shots do not grant hitmarkers. Delayed environmental deaths are not guessed from previous attackers. Marker, damage text and sound toggles can be used independently. Native HUD/menu visibility hides the overlay.

## Sound

Body and head hits use local 2D playback, without distance falloff or world occlusion. Volume is adjustable with a 100% default, and still respects DayZ's effects/master volume. The bundled short body sound and **temporary headshot sound are original generated sounds, not Rust assets**. The Options section labels the missing Rust clip. The exact Rust headshot audio is pending the user's WAV/OGG upload.

To install the supplied clip, use `python3 tools/build_hitmarker_audio.py --headshot /path/clip.wav` (ffmpeg required). It converts to mono 48 kHz PCM WAV, controls loudness/peaks, and marks the clip installed. It does not fetch game assets. Rebuild afterward. Running the audio script with no arguments restores the original temporary sounds.

## Build and validation

Run `python3 tools/build_hitmarker_layouts.py` after changing layout definitions, then `python3 tools/build_hitmarker.py --output-dir downloads`. Packaging does not regenerate/replace imported audio. Run `python3 tools/validate_hitmarker.py` to check archive/source agreement, widget bindings, option wiring, RPC ordering and audio format/peaks. These are static/package checks, not an Enforce compiler or DayZ runtime test.

In-game checks: Options > Game section visibility/scrolling alongside GRZ Movement; Apply/Cancel/Defaults/persistence; both sound tests at several volumes; 25/50/150% size; body/head shots on armored/unarmored players, infected and animals; kill versus unconscious state; corpse/self/scenery filtering; no feedback on another shooter's client; HUD hiding and mission cleanup. Confirm damage-callback ordering on the installed DayZ version. Other mods that omit `super` in damage hooks can require compatibility changes.

Development PBO is unsigned. The local server bundle uses signature checking disabled. The same updated PBO is required on server and clients. RPC ID 1647280201 must remain unique.
