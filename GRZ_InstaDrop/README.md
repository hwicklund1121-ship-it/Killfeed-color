# GRZ InstaDrop v0.1.5 test build

Target: keep the G gun-lift/throw-ready pose while dropping to prone, then permit standing during a roll/on-back pose. The stance source is unchanged from v0.1.4, whose roll-to-stand behavior the user confirmed works. The new change is fast-drop animation tuning; instantaneous ground contact is not yet verified.

Animation rates have one owner: GRZ Movement. It applies a 4.0 native speed request to six indexed standing/crouching-to-prone rifle clips. It does not retime recovery, roll, throw release or ordinary gait clips. Existing gait data stays at the user's current 1.50 request. GRZ InstaDrop itself only handles stance permissions and does not call the animation-speed API or replace any graph. Native graph timing can still limit the visible result; an accepted clip request is not a measured transition time or a restoration of the 1.27 graph.

The ZIP contains GRZ_InstaDrop.pbo, GRZ_InstaDropData.pbo and an updated GRZ_Movement.pbo. No vanilla animations or LUGU assets/scripts are redistributed. Only public DayZ APIs and indexed native filenames are used.

Close DayZ and stop the server. Keep LUGU unloaded, and keep only your @GRZ_Local copy of GRZ Movement active. Extract the ZIP and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1
```

The installer backs up prior copies of all three PBOs on client and server before replacing them, verifies hashes and attempts rollback if copying fails. It does not change GRZ_MovementSpeedData.pbo, saved outfits, camera preferences or profiles. Custom ServerPath/GamePath parameters are supported. This is an unsigned development build for the existing local-addon test setup; signing/server signature settings are not changed.

Restart both sides. The script log should contain `[GRZ Movement] Fast-drop-only tuning: accepted 6/6 clips, request=4`. The unchanged stance script still prints v0.1.4 diagnostics; the top-level startup message identifies v0.1.5. Test G -> prone -> roll -> stand, including both first/third person and a second player's view. Compare actual time to ground contact with v0.1.4. If the clips are accepted but the drop remains slow, a graph transition/blend may be limiting the result; send that test's script log and a short clip.

Compatibility targets the supplied GRZ Movement setup. Stock positive stance decisions are preserved. The two firearm exceptions retain gun-lift preparation when dropping to prone and allow a roll/on-back standing request. Charge/release, dropping an item, restrictive actions, restraint, unconsciousness, weapon operations, water and non-movement commands stay guarded. The stand-up request uses native PlayerCanChangeStance and ForceStanceUp; no position teleport or persistent stance lock is used. Mods that also override negative stance decisions or the same clip rates can still conflict; universal movement-mod compatibility is not claimed.

Build with `python tools/build_instadrop.py --index INDEX --preset packaging/instadrop-update/TRANSITIONS.json`. Tests exercise the actual pure permission policy, package/source hashes and the PowerShell installer's copies/backups and rejection paths. They do not compile Enforce or run DayZ. Uploaded client logs confirmed v0.1.1 loaded and accepted its native clip paths; only the later v0.1.4 roll-to-stand result has been confirmed in gameplay.

To revert, stop both sides and restore the three PBOs from the corresponding GRZ-instadrop-backup folder. Do not restore an older GRZ Movement from an unrelated backup if you want to retain your current gait/camera setup.
