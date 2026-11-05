AR50 Sounds - replacement folder

Replace the entire old ar50a1 folder inside your unpacked UAVWEAPONS weapon pack
with this ZIP's ar50a1 folder. Repack UAVWEAPONS.pbo with its existing prefix and
all other weapon folders preserved. Replace the weapon PBO on BOTH client and
server and restart both. No extra sound PBO is needed.

The supplied Vamp_AR50.ogg recording is renamed AR50_Shot.ogg. All three existing
AR50 close/mid/far firing shaders use that recording. Original shader volumes,
ranges and curves are retained. Models, textures, materials, scripts, recoil,
ammo and other weapon configuration values remain unchanged.

The stereo 48-kHz source is converted to mono 44.1-kHz Vorbis for spatial weapon
audio, preserving its duration. Source is retained in art/audio/ar50. Audio
decoding, resource/config references, unchanged files and ZIP integrity passed.
Actual DayZ playback and the repacked combined PBO have not been tested here.

Rebuild: python tools/build_ar50_replacement.py --source-zip <supplied ar50a1.zip>
