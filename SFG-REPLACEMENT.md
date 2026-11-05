SFG Sounds - replacement of the supplied current SFG50 folder

Both normal and suppressed shots now use the supplied AX50 close/mid/far
recordings, stored as SFG_Close.ogg, SFG_Mid.ogg and SFG_Far.ogg. The two old
firing clips are removed. Only the six shader sample arrays change in the
root config. Existing volume, distance curves/ranges, ammo, attachments,
scripts, models, textures, materials and weapon stats remain as supplied.

Install into the existing weapon pack:
1. Stop the game/server and back up the current weapon PBO.
2. Replace the entire SFG50 folder in the unpacked UAVWEAPONS with this ZIP's
   SFG50 folder. Delete/move the old folder first so its old audio is removed.
3. Repack UAVWEAPONS.pbo with its existing prefix/other folders preserved.
4. Replace that PBO on both client and server, then restart both.

Remove the earlier SFG_Sounds.pbo audio override from both Addons folders if
installed. This replacement has the audio built in and needs no extra sound PBO.

Original stereo clips were converted to mono 44.1-kHz Vorbis for spatial audio.
Audio decode, sample/resource paths, unchanged files, PBO checksum/round-trip
and ZIP integrity were checked. DayZ playback and runtime config loading have
not been tested here. New PBOs are unsigned; use your own signing key if needed.

Rebuild: python tools/build_sfg_audio.py, then
python tools/build_sfg_replacement.py --source-zip <supplied SFG50.zip>
