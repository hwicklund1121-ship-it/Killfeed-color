# GRZ InstaDrop v0.2.1 signed native landing test

Target: the supplied vanilla DayZ 1.30.164.289 player graph.

Changes from v0.2.0:

+- Targets the requested mapping: look right -> lie on the left side; look
+  left -> lie on the right side, while keeping the rifle aimed with the view.
+- Replaces the body blend's identical front/back endpoints with DayZ's recorded
+  directional prone pose (`Locomotion.PneRas.AimToBack`). Left and right view
+  angles now sample distinct native pose columns, including the intermediate
+  side poses, rather than relying on a symmetric two-pose interpolation.
+- Preserves the signed native AimX/AimY sampling and the weapon hold layer.
+  Camera controls and weapon aiming are not inverted.
+- Keeps the 0.14-second drop request, weapon blend timing, roll entry/recovery,
+  guarded on-back roll permission and actual-roll stand queue from v0.2.0.
+
+This is a test build. Structural checks establish distinct native pose samples
+for the two view directions, not which anatomical side contacts the ground in
+DayZ. The requested left/right ground-side mapping, weapon pose and smoothness
+still need in-game verification. AimX is relative to the character, rather than
+absolute world camera heading. DayZ retains its native stance and collision
+handling; no position/velocity edits are used.
+
## Install

Close DayZ and stop the server. Extract this ZIP to a new folder in Downloads,
open PowerShell in that extracted folder, and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1
```

`-GamePath` and `-ServerPath` support non-default locations. The installer reads
the game's native `Addons/anims_workspaces.pbo`, verifies the two source hashes,
builds the patched graph locally, and verifies its expected PBO hash. The ZIP
contains GRZ code, helpers, two GRZ PBOs and metadata. No native graph or `.anm`
payloads are bundled or downloaded. The generated local graph contains native
Bohemia Interactive graph content with the GRZ modifications.

Both existing `@GRZ_Local/Addons` folders must contain GRZ Movement. Executable
versions, package hashes and signatures are checked. The installer backs up and
replaces three InstaDrop PBOs, verifies copies, and attempts rollback on failure.
It leaves your startup command, other mods, movement/gait data, camera settings,
profiles and outfits alone. Use your normal dependency mods plus `@GRZ_Local`.

## Test

Restart both sides. The startup message begins:

```
[GRZ InstaDrop] v0.2.1 graph selected: GRZ_InstaDropGraph/player_main.agr.
```

This confirms config selection; also check RPT/error logs for graph load errors.
With a rifle in G preparation, test dropping while already looking right,
then while looking left. Check right-look -> left ground side and left-look ->
right ground side, with the rifle following the view. Also check forward and
backward views. Then test the right-look Q/A/D roll
combo and your normal stand input. Check normal throwing and ordinary movement
as well. Send a new short clip and client/server logs if the result still differs.
The roll permission/start/finish traces and actual-roll standing traces remain.

## Revert

Close both sides, then use the exact backup folder printed by the installer:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Restore-InstaDrop.ps1 -BackupPath "FULL BACKUP FOLDER"
```

This restores the previous InstaDrop PBOs on both sides, including an older
graph if present, and removes a graph PBO newly created by this installation.
This is unsigned for the existing development setup; signing keys and server
signature settings are unchanged.
