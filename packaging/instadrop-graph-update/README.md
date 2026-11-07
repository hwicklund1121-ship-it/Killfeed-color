# GRZ InstaDrop v0.2.0 directional landing test

Target: the supplied vanilla DayZ 1.30.164.289 player graph.

Changes from v0.1.9:

- Replaces the fixed belly-down gun-lift landing with a look-angle blend of
  DayZ's native front and on-back poses. Forward uses the native front pose;
  looking roughly 90 degrees right or left mixes the two into a side-facing
  pose; looking behind tends toward the native on-back pose.
- Uses the same AimX thresholds and S interpolation for the body and weapon
  hold layer, so one does not switch to the on-back pose ahead of the other.
- Matches the weapon stance blend to the softened 0.14-second drop request
  (previously 0.10). The short drop is retained while giving the landing a
  little more time to blend.
- Retains v0.1.9's roll-entry/recovery routes, on-back roll permission guards,
  and stand queue that requires an actual roll. Key bindings are unchanged.

The direction blend uses existing game poses with thresholds -180, 0, 180 and
the native S interpolation mode. This changes the visual pose; DayZ keeps its
native stance and collision handling. No position/velocity edits are used.

This is a test build. Source, graph structure, local generation and installer
checks do not establish the appearance in DayZ. Side appearance depends on the
native AimX angle relative to the character, rather than absolute world camera
heading. Actual smoothness, weapon alignment, and the full quick-motion combo
still need first-person, third-person and remote-view gameplay testing.

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
[GRZ InstaDrop] v0.2.0 graph selected: GRZ_InstaDropGraph/player_main.agr.
```

This confirms config selection; also check RPT/error logs for graph load errors.
With a rifle in G preparation, test dropping while already looking forward,
right, left and behind. Look for a gradual side-facing landing to the right,
with the rifle/arms following the torso. Then test the right-look Q/A/D roll
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
