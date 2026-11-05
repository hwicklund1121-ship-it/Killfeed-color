# GRZ Hitmarker

Separate DayZ PC mod, development version 0.1.8. Install on the server and every client. Settings live in **Options > Game > GRZ HITMARKER**, appended to the existing game settings list, using native left/right option selectors. The existing General, Gunrunnerz FOV, HUD and Chat sections remain intact. No slash command or VIP access is needed.

Controls: show hitmarkers, show damage info, hitmarker sound, highlight kills, normal-hit color, headshot color, size (25–150%, default 50%) and volume (0–100%, default 100%). Colors include white, red, orange, yellow, green, blue, indigo, violet, cyan, pink, grey and black. Compact rows use a 45%-opaque black background. A boxed, marker-only live preview sits at the bottom right; all test buttons are removed. Changing body or head color shows that color in the preview. The marker consists of exactly four short diagonal line segments, drawn on a canvas with a fine outline and a clear center. Press the normal Options Apply button to save; Cancel/Revert restores saved selections and Defaults restores the hitmarker defaults. Client preferences are saved in `$profile:GRZ_Hitmarker/settings.json`.

The centered X briefly appears only for confirmed hits against players, infected and animals. The server captures living-target health before applying damage, confirms the hit afterward, and sends feedback only to the attacking player's client. The damage number represents global health lost, which can be zero for shock-only damage. Head/brain zones use the headshot color and sound, with a hit-component damage-zone fallback when the callback zone is generic or empty. Optional kill highlighting makes the X larger and adds KILL text; unconsciousness alone is not a kill. Corpse hits, self-hits, scenery and other players' shots do not grant hitmarkers. Delayed environmental deaths are not guessed from previous attackers. Marker, damage text and sound toggles can be used independently. Native HUD/menu visibility hides the overlay.

## Sound

The supplied Rust headshot clip is installed. Native game samples are mono 44.1 kHz Vorbis OGG with unique `body_hit018` and `headshot018` basenames; these basenames have no competing WAV file. Original 48 kHz WAV intermediates remain for conversion checks.

Version 0.1.8 replaces the raw 2D player that never reported starting in the supplied client log. It uses native `EffectSound.SoundPlayEx` at the current camera on `WAVEEFFECTEX`, matching the standard effects channel. EffectSound retains its builder and parameters during asynchronous loading and validates the sample header. Each voice owns its EffectSound until completion or timeout. Sound sets explicitly define frequencyFactor=1, volumeFactor=1, volumeRandomizer=0 and soundShadersLimit=1, with spatial/doppler/loop disabled. The camera position is refreshed during playback, and shader gain stays flat over the supported range. Options Sound/Volume still control feedback; normal game sound must be audible. No global audio slider is changed.

The client script log records the first confirmed body/head hits and first successful sound starts, or reports invalid configuration, failed playback or a three-second startup timeout. Audibility is not verified without running DayZ.

To import a headshot clip, run `python3 tools/build_hitmarker_audio.py --headshot /path/clip.wav` (ffmpeg required), then rebuild. Running the audio script with no arguments restores the original temporary sounds.

## Build and validation

Run `python3 tools/build_hitmarker_layouts.py` after changing layout definitions, then `python3 tools/build_hitmarker.py --output-dir downloads`. Packaging does not regenerate/replace imported audio. Run `python3 tools/validate_hitmarker.py` to check archive/source agreement, widget bindings, option wiring, RPC ordering and audio format/peaks. These are static/package checks, not an Enforce compiler or DayZ runtime test.

In-game checks: Options > Game section visibility/scrolling alongside GRZ Movement; Apply/Cancel/Defaults/persistence; body and head hit sounds at several volumes; 25/50/150% size; body/head shots on armored/unarmored players, infected and animals; kill versus unconscious state; corpse/self/scenery filtering; no feedback on another shooter's client; HUD hiding and mission cleanup. Confirm damage-callback ordering on the installed DayZ version. Other mods that omit `super` in damage hooks can require compatibility changes.

Development PBO is unsigned. The local server bundle uses signature checking disabled. The same updated PBO is required on server and clients. RPC ID 1647280201 must remain unique.
