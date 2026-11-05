SFG Sounds, permanent suppressor and VULCAN chamber - replacement of the supplied current SFG50 folder

Both normal and suppressed shots now use the supplied AX50 close/mid/far
recordings, stored as SFG_Close.ogg, SFG_Mid.ogg and SFG_Far.ogg. The two old
firing clips are removed. The root config also chambers UAV_Ammo_VULCAN (the pile whose ammo field fires
UAV_Bullet_VULCAN) and uses a unique suppressor proxy. Existing volume, distance
curves/ranges, recoil and other weapon stats remain as supplied.

The server adds UAV_SFG_Sup after gun initialization and saved-game loading.
The suppressor slot is locked and CanReleaseAttachment rejects its removal.
Already attached suppressors are retained, including their condition; ruined
suppressors are not automatically healed. Existing ammo in an old gun is not
converted; unload it or spawn a fresh gun for the VULCAN test. The UAV VULCAN
ammo addon must be loaded on both installations.

Proxy naming: the generic Proxysuppressor/proxy/suppressor.p3d becomes the
unique Proxysfg_muzzle/proxy/sfg_muzzle.p3d. Equal-length path edits preserve
the main ODOL's size/header and geometry; suppressor model bytes stay identical.
The redundant nested proxy/config.cpp is removed because these definitions
already live in the supplied root config. Textures/materials are unchanged.
This addresses a potential global proxy-name collision; it does not establish
the cause of the reported invisibility without in-game verification.

Install into the existing weapon pack:
1. Stop the game/server and back up the current weapon PBO.
2. Replace the entire SFG50 folder in the unpacked UAVWEAPONS with this ZIP's
   SFG50 folder. Delete/move the old folder first so its old audio is removed.
3. Repack UAVWEAPONS.pbo with its existing prefix/other folders preserved.
4. Replace that PBO on both client and server, then restart both.

Remove the earlier SFG_Sounds.pbo audio override from both Addons folders if
installed. This replacement has the audio built in and needs no extra sound PBO.

Original stereo clips were converted to mono 44.1-kHz Vorbis for spatial audio.
Audio decode, sample/resource paths, model size/header, PBO checksum/round-trip
and ZIP integrity were checked. DayZ playback and runtime config loading have
not been tested here. New PBOs are unsigned; use your own signing key if needed.

Rebuild: python tools/build_sfg_audio.py, then
python tools/build_sfg_replacement.py --source-zip <supplied SFG50.zip>
python tools/build_sfg_permanent.py
