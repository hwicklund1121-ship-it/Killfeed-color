# GRZ InstaDrop v0.1.6 graph test

Target: DayZ 1.30.164.289, using the owner's supplied vanilla player graph export.
This build changes the gun-lift/throw-ready standing and crouching drop routes to
blend directly into the native prone gun-lift pose in a requested 0.03 seconds.
It avoids the intermediate `ErcToPne` / `CroToPne` states and their lower-animation
time gate. The graph change is four property lines in `ThrowIdleStanceSTM`.
The download contains no native graph payloads: the installer builds the modified
graph locally from your installed `Addons/anims_workspaces.pbo` and verifies the
original source hashes and generated PBO hash before installation.

This is a test build. Source and packaging checks passed; DayZ graph loading,
physical ground-contact timing and first/third-person/remote-player playback
cannot be established by those checks. A 0.03-second blend request is not a
measured result or a claim of restored DayZ 1.27 behavior.

The working `GRZInstaDropThrowReady.c` roll-to-stand source is byte-for-byte
unchanged from v0.1.5. Its old v0.1.4 trace labels are retained. The top-level
startup message is now v0.1.6. No new stand-up or roll graph edits are made.
The direct drop edges belong to the shared throw-ready stance machine, so other
held-item profiles using this same machine can also receive the faster drop.
Ordinary non-throwing prone transitions are unchanged.

## Install

Close DayZ and stop the server. Extract this ZIP to a folder in Downloads.
Open PowerShell in the extracted folder and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1
```

The existing local-addon setup must contain `@GRZ_Local\Addons\GRZ_Movement.pbo`
on client and server. Custom `-GamePath` and `-ServerPath` arguments are supported.
The installer checks the game/server executable version, package hashes, target
folders and signatures, backs up the three affected PBOs, copies and verifies
them, and attempts rollback if a copy fails. It does not edit startup commands.
Keep `@GRZ_Local` enabled on both sides and keep LUGU unloaded.

This package contains:

- `GRZ_InstaDrop.pbo`: existing stance policy and new startup graph check.
- `GRZ_InstaDropData.pbo`: disables the old 4x drop-speed requests.
- `Build-InstaDropGraph.ps1`, its authored C# helpers and graph config: generate
  `GRZ_InstaDropGraph.pbo` locally. This PBO is not bundled in the download.

`GRZ_Movement.pbo`, its gait data, camera settings, profiles and saved outfits
are not replaced. The graph is based only on your vanilla files. No native graph or `.anm`
payloads and no LUGU code are bundled or downloaded. The generated local graph
contains Bohemia Interactive content plus the two-edge GRZ modification. Original
and generated hashes are recorded in `GRAPH-CHANGES.json`.

## Check the test

Restart the server and client. The script log should contain:

```
[GRZ InstaDrop] v0.1.6 graph selected: GRZ_InstaDropGraph/player_main.agr.
```

This message verifies the config selection, not successful native graph parsing.
If another addon overrides the base graph, v0.1.6 logs that and disables itself.
Check the RPT/error log for graph load errors too. With a rifle in hand, enter G
gun-lift preparation and request prone from standing, then from crouching. Test
roll -> stand as before. Compare first person, third person and another player's
view. Check normal prone movement and normal throw charging/release as well.
Send a fresh short clip plus client/server logs if the requested result differs.

## Revert

Close both sides and run the supplied restore script with the exact backup
folder printed by the installer:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Restore-InstaDrop.ps1 -BackupPath "FULL BACKUP FOLDER"
```

New graph PBOs absent before installation are removed on restore; previous
InstaDrop files are restored. The backup contains `RESTORE.json` and original
copies for both sides. This is an unsigned build for the existing development
setup; server signature settings and signing keys are not changed.
