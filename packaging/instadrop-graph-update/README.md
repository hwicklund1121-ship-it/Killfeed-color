# GRZ InstaDrop v0.3.0 animated recovery test

Target: the supplied vanilla DayZ 1.30.164.289 player graph.

This update addresses the comparison between the current curled-pose/upright
snap and the reference's connected ground-turn/get-up/move sequence.

Changes from v0.2.1:

- The throw-strength pose no longer fully masks the underlying landing while
  ActionProgress is zero. Its weight ramps to the original strength layer at
  the start of actual charging; throw release clips are unchanged.
- Retains the native signed directional prone landing and the 0.14-second
  drop request, so preparation uses the landing pose rather than a static
  strength-pose frame.
- Restores the native prone/on-back get-up clips in the gun-lift stance machine.
  Their exits use remaining clip time rather than a fixed 0.7/0.8-second gate.
- Rolls transition into native front/back recovery animation nodes instead of
  directly blending into an upright idle. Recovery can finish into idle or
  movement, and a changed stance can cancel the custom recovery route.
- Requests 2.2 playback speed on exactly four reviewed normal rifle standing
  recovery clips. Drop, roll, crouch-recovery, gait and throw-release clip rates
  are not changed. Native get-up leg/arm motion is retained.
- Keeps the existing roll permission guards and stand queue that requires an
  actual roll. Input bindings and player position/velocity are unchanged.

This is a test build. Graph/source, package, generator and installer checks do
not establish native playback, actual get-up timing or a match to the reference.
Check body/weapon alignment, moving recovery and normal throw charging/release
in first person, third person and another player's view. The speed value is a
request; an accepted request is not a measured transition duration.

## Install

Close DayZ and stop the server. Extract the ZIP to a new folder in Downloads,
open PowerShell there, and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1
```

Custom `-GamePath` and `-ServerPath` are supported. The installer reads the
installed native `Addons/anims_workspaces.pbo`, verifies the two graph hashes,
builds the patch locally and verifies the expected output PBO hash. Native
graph or `.anm` payloads are not bundled or downloaded. The generated local
graph contains Bohemia Interactive content with the GRZ patch.

Both existing `@GRZ_Local/Addons` folders must contain GRZ Movement. The
installer checks executable versions, package hashes and stale signatures,
backs up/replaces three InstaDrop PBOs, verifies copies and attempts rollback
on failure. Startup commands, other mods, gait data, camera settings, profiles
and outfits are not modified. Use your usual dependencies plus `@GRZ_Local`.

## Test

Restart both sides and check the script log for:

```
[GRZ InstaDrop] v0.3.0 graph selected: GRZ_InstaDropGraph/player_main.agr.
[GRZ InstaDrop] v0.3.0 native get-up speed: accepted 4/4; request=2.2.
```

These messages verify config/rate requests; check RPT/error logs for native
graph errors too. Test G -> drop while looking right -> Q/A/D roll -> stand/move.
Compare the leg movement and rifle pose with the supplied reference. Check the
other look direction, ordinary prone/stand/crouch, canceling recovery with a
new stance request, and real throw charging/release. Send a new short clip and
client/server script logs if it still differs. Roll start/finish and queued
standing traces remain available.

## Revert

Close both sides and use the exact backup folder printed during installation:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Restore-InstaDrop.ps1 -BackupPath "FULL BACKUP FOLDER"
```

Previous PBOs, including an older graph, are restored on both sides. A graph
newly added by this installation is removed on restore. Restart both sides
after reverting so old animation rate requests are cleared. This remains an
unsigned development build; signature settings and signing keys are unchanged.
