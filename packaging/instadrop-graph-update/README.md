# GRZ InstaDrop v0.3.1 pose-conflict test

This build uses the v0.2.1 movement shown in the user's clip as the approved
baseline. The requested fix is the weapon/body hitch before the stand-up;
the stand-up itself is to stay unchanged.

The only locomotion-graph difference from that baseline is the weight of
`ThrowStrengthIdleB`: uncharged preparation uses the underlying native landing
pose, and actual charging enables the existing strength-pose layer. This targets
the full-strength pose masking/conflicting with the prepared landing. The
0.14-second drop, directional pose source, roll routes, direct stand routes,
blend durations, cancellation guards and stance-input policy match v0.2.1.

The v0.3.0 animated recovery and clip-speed requests are not used. If v0.3.0
was tested, fully close both game processes before installing/restarting so
its runtime animation-rate requests are cleared. This ZIP's data matches the
v0.2.1 configuration and contains no new recovery-rate module.

This is a test build targeting DayZ 1.30.164.289. Source checks prove that
restoring the single pose-weight property reproduces the approved baseline
graph exactly. They do not establish native visual playback or prove that
the hitch is fixed. Check the whole sequence and real throw charging/release.

## Install

Close DayZ and stop the server. Extract to a new folder in Downloads, open
PowerShell in that folder, and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1
```

Custom `-GamePath` and `-ServerPath` are supported. Existing GRZ Movement in
`@GRZ_Local/Addons` is required on both sides. The installer verifies source
graphs and builds the patched graph locally, checks hashes/versions/signatures,
backs up three InstaDrop PBOs, verifies replacement copies and attempts rollback
on failure. No native graph or animation payloads are bundled/downloaded.
Startup commands, other mods, gait data, cameras, profiles and outfits remain.
Use the usual dependency list plus `@GRZ_Local` on client and server.

## Check

Startup should identify `v0.3.1 graph selected` and `Stand/roll routes match
v0.2.1`. Check RPT/error logs for graph errors. Repeat the exact sequence from
the clip and check whether the gun/body still rocks backward then forward
before the accepted stand-up. Test normal charging/release too. Send a short
new clip and client/server script logs if the hitch remains. Drop and stand
timing are intentionally unchanged from the approved baseline.

## Revert

Close both sides and use the exact printed backup folder:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Restore-InstaDrop.ps1 -BackupPath "FULL BACKUP FOLDER"
```

Previous InstaDrop PBOs are restored on both sides. This is unsigned for the
existing development setup; signing keys/signature settings are unchanged.
