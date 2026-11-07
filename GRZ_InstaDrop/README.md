# GRZ InstaDrop v0.3.2 roll-continuity test

The requested change is a smoother continuous roll, preserving the foot motion
and stand-up that the user likes. This builds on v0.3.1.

Changes:

- Prevents new rifle roll requests during an already active native roll. The
  callback latch clears on native roll finish and has a 1.5-second fallback;
  native `IsInRoll()` is also checked. Stance and ordinary movement inputs are
  unchanged, including standing during the roll.
- Adds an active-state tag to the native on-back roll and prevents the general
  front-roll entry from being selected while either roll state is active. This
  targets an on-back-to-front route switch or restart during the Q/A/D sequence.
- Keeps existing roll animation children, foot clips, directional landing,
  drop blend, standing routes and all existing motion durations unchanged.
  No recovery-speed tuning or v0.3.0 animated-get-up changes are included.

This is a test build for DayZ 1.30.164.289. The supplied log confirmed v0.3.1
loaded and recorded allowed stance requests but only one native roll start.
The guards address a possible roll restart/route switch; they do not prove
the cause of every visible head/chest dip. Whether the reported backward/forward
hitch is removed requires a fresh in-game comparison.

## Install

Close DayZ and stop the server. Extract the ZIP to a new Downloads folder,
open PowerShell in that folder, and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1
```

Custom `-GamePath` and `-ServerPath` are supported. Both existing
`@GRZ_Local/Addons` folders must contain GRZ Movement. Native graphs are read,
checked and patched locally; the output hash is verified before installation.
No native graph or animation payloads are distributed. The installer checks
versions, hashes and signatures, backs up three PBOs, verifies copies, and
attempts rollback on failure. Other mods, gait/camera settings, profiles,
outfits, signing keys and startup commands are unchanged.

## Check

Startup should identify `v0.3.2 graph selected` and the roll continuity guard.
Check the RPT/error log for graph errors. Repeat the same G -> drop/look right
-> Q/A/D -> stand sequence. Check for one continuous foot/body motion rather
than a back/front restart, and verify the accepted stand-up remains the same.
Native roll start/finish and suppressed repeated-request logs are available.
If the hitch remains, send a new short clip and client/server script logs.

## Revert

Close both sides and use the exact backup directory printed by the installer:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Restore-InstaDrop.ps1 -BackupPath "FULL BACKUP FOLDER"
```

This restores the prior InstaDrop PBOs on both sides. Restart both afterwards.
This is an unsigned development build for the existing local-mod setup.
