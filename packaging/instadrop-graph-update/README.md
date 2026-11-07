# GRZ InstaDrop v0.1.8 on-back roll test

Target: the supplied vanilla DayZ 1.30.164.289 player graph.

Changes from v0.1.7:

+- Adds a narrow `CanRoll()` exception for the living, unrestrained rifle player
+  in prone/on-back gun-lift preparation. Stock `CanRoll()` explicitly rejects
+  `IsOnBack()`, which can block the right-look Q/A/D sequence before a graph
+  blend is considered. Stock positive decisions remain positive.
+- Keeps stamina, emote, action, weapon-operation, actual throw charge/release,
+  item-drop, life and movement-command guards on this new exception. Other
+  mods' independent negative roll policies may still conflict.
+- Keeps v0.1.7's entire generated graph unchanged: drop remains 0.10 seconds
+  and the prior direct-standing blend request remains 0.16 seconds. There is
+  no new pose timing experiment in this build.
+- Keeps the existing stance-input and stand-request source unchanged. No key
+  combinations are synthesized and rolling does not request standing by itself.
+- Logs the on-back roll permission decision and native roll-start/finish events
+  so the next test distinguishes a rejected request from graph playback.
+
Four existing edges in `ThrowIdleStanceSTM` are changed: standing/crouching to
prone and prone/on-back to standing. Input conditions are unchanged. Turn,
on-back selection, roll, throw charging/release, crouch recovery and normal
non-throwing movement graph data are unchanged. The shared throw-ready graph
also applies to other held-item profiles that use the same graph machine.

This is a test build. Graph/source, packaging, local-generation and installer
checks do not establish DayZ playback. The 0.10/0.16 values are requested blend
durations, not measured end-to-end motion times. The gun/arm pose and native
physical stance timing still need first/third-person and remote-view testing.

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
Interactive graph content with the GRZ four-edge patch.

The existing `@GRZ_Local/Addons` on both sides must contain GRZ Movement. The
installer verifies executable versions, file hashes and signatures, backs up
and replaces the three InstaDrop PBOs, verifies copies, and attempts rollback
on failure. Existing v0.1.6 graph PBOs are backed up too. It leaves startup
commands, GRZ Movement/gait data, camera settings, profiles and outfits alone.
Keep your normal dependency mod list plus `@GRZ_Local` enabled on both sides.

## Test

Restart both sides. Startup should report:

```
[GRZ InstaDrop] v0.1.8 graph selected: GRZ_InstaDropGraph/player_main.agr.
```

This verifies config selection; also inspect the RPT/error log for graph loading
errors. Test your G -> prone -> look right -> Q with A/D sequence, then your normal
stand input. Look for `v0.1.8 on-back roll` and `v0.1.8 native roll started` in
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
