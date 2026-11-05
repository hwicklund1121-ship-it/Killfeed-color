SFG Sounds 0.1.0

Stop DayZ and the server. Copy SFG_Sounds.pbo into @GRZ_Local/Addons on BOTH
client and server, then restart both. Keep your existing SFG/weapon PBO loaded.
The ZIP extracts this file into @GRZ_Local/Addons. No additional launch argument
is needed when @GRZ_Local is already loaded.

This addon replaces both normal and suppressed SFG firing sounds. Each pair
of shaders uses the corresponding AX50 clip:
- sfg50_closeShot_SoundShader / sfgs50_closeShot_SoundShader -> SFG_Close.ogg
- sfg50_midShot_SoundShader / sfgs50_midShot_SoundShader -> SFG_Mid.ogg
- sfg50_distShot_SoundShader / sfgs50_distShot_SoundShader -> SFG_Far.ogg

It depends on the sfg50 addon present in the supplied SFG PBO. Both existing shot SoundSets select these replacement shader samples. Original
distance curves, volumes and ranges are retained (50/200/500 metres in the
supplied config). Ammo, model, weapon stats and non-firing sounds are unchanged.
Vamp_AR50 is not included. The supplied stereo Ogg files are converted to mono
44.1-kHz Vorbis for the gun's spatial audio. Originals are in art/audio/sfg-ax50.

Packaging, audio decoding and config/reference checks passed. This does not
compile the config or verify sound playback in DayZ. Test normal and suppressed SFG shots with
a nearby and a distant listener. If your installed SFG uses different shader or
addon names from the earlier uploaded SFG.pbo, send its current config.cpp.
Unsigned development PBO; sign with your own key if signatures are enforced.

Rebuild: python tools/build_sfg_audio.py (requires ffmpeg/ffprobe).
