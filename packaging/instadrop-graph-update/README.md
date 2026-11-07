# GRZ InstaDrop v0.1.9 roll-entry and recovery test

Target: the supplied vanilla DayZ 1.30.164.289 player graph.

Changes from v0.1.8:

+- The stand helper now requires an actual roll; being on the back alone no
+  longer triggers `ForceStanceUp`. Explicit stand intent is queued for up to
+  750 ms and a new prone/crouch request clears that queue.
+- Adds native-roll graph entry routes from gun-lift mode for front and on-back
+  poses, without the general roll route's aiming-angle restriction. A native
+  roll command is still required; actual throw entry/release/cancel phases are
+  excluded. Existing Q/A/D bindings and input processing are not replaced.
+- Adds standing exits from both native roll states after a minimum 0.12-second
+  animation segment, with a 0.16-second requested blend. These edges require
+  native standing stance; they do not request standing on their own.
+- Keeps the 0.10-second drop request and the guarded on-back roll permission
+  exception. Stamina, emotes, action, weapon, item-drop and actual throw guards
+  remain on the exception. Other mods' separate roll policies can still conflict.
+
+This is a test build based on the 13:09 client log: v0.1.8 loaded, only two native
+roll-start events were recorded, and numerous forced stand requests appeared
+soon after the drop without a preceding roll-start. Existing ordinary native
+standing input remains native; the change removes the helper's redundant early
+force. The graph and callback changes still require DayZ gameplay verification.
+The requested timing values are not measured end-to-end movement times.
+
+No native graph or animation payload is included in this download. Native game
+graphs are read and patched only on the user's computer. Graph/source changes
+are checked against the supplied DayZ 1.30.164.289 export and generated PBO hash.
+
## Install

Close DayZ and stop the server. Extract this ZIP to a folder in Downloads, open
PowerShell in that extracted folder, and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1
```

The installer supports `-GamePath` and `-ServerPath` for non-default locations.
It reads the installed game's `Addons/anims_workspaces.pbo`, checks two source
hashes, and builds `GRZ_InstaDropGraph.pbo` locally. The generated PBO must match
the checked output hash before installation. The download contains only GRZ
code/helpers, two GRZ PBOs and metadata; no native graph or animation payloads
are bundled or downloaded. The locally generated graph contains Bohemia
Interactive graph content with the GRZ patch.

The existing `@GRZ_Local/Addons` on both sides must contain GRZ Movement. The
installer verifies executable versions, file hashes and signatures, backs up
and replaces the three InstaDrop PBOs, verifies copies, and attempts rollback
on failure. Existing v0.1.6 graph PBOs are backed up too. It leaves startup
commands, GRZ Movement/gait data, camera settings, profiles and outfits alone.
Keep your normal dependency mod list plus `@GRZ_Local` enabled on both sides.

## Test

Restart both sides. Startup should report:

```
[GRZ InstaDrop] v0.1.9 graph selected: GRZ_InstaDropGraph/player_main.agr.
```

This verifies config selection; also inspect the RPT/error log for graph loading
errors. Test your G -> prone -> look right -> Q with A/D sequence, then your normal
stand input. Look for `v0.1.9 on-back roll` and `v0.1.9 native roll started` in
the script log. Check weapon/arm placement throughout, ordinary prone
and crouch movement, and ordinary throw charging/release. Native clearance and
physical stance checks remain active. Send a new clip and client/server logs
if playback or quick-input response still differs from the intended sequence.

## Revert

Close both sides. Use the exact backup folder printed by the installer:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Restore-InstaDrop.ps1 -BackupPath "FULL BACKUP FOLDER"
```

The restore script reinstates the prior PBOs on both sides, including a previous
graph PBO if one existed, and removes a graph PBO newly created by this install.
This is an unsigned build for the existing development setup; signing keys and
server signature settings are not changed.
