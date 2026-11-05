NOREEN50 Sounds - replacement folder

Replace the entire old NOREEN50 folder inside your unpacked UAVWEAPONS weapon
pack with this ZIP's NOREEN50 folder. Repack UAVWEAPONS.pbo with its existing
prefix and other weapon folders preserved. Replace the weapon PBO on BOTH
client and server and restart both. No extra sound PBO is needed.

Uses the same supplied Vamp_AR50 recording as the AR50 replacement, renamed
NOREEN50_Shot.ogg. All three existing NOREEN close/mid/far firing shaders use
that recording. Original volumes, ranges and curves are retained. All other
config values, models, textures, materials and scripts are unchanged.

The recording is mono 44.1-kHz Vorbis for spatial audio. Audio decoding, sample
and resource references, unchanged files and ZIP integrity passed. DayZ playback
and the repacked combined PBO have not been tested here.

Rebuild: python tools/build_ar50_replacement.py --weapon NOREEN50
         --source-zip <supplied NOREEN50.zip>
