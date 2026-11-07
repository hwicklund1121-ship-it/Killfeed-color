# v0.1.9 validation

Validated against the supplied vanilla graph export for DayZ 1.30.164.289.

```sh
python tools/test_instadrop_graph.py /path/to/GRZ-DropGraph-export.zip
```

Five tests passed: exact four-edge graph change with all other graph text
preserved; unsupported/previously patched graph rejection; source hash/schema/
version checks; and reproducible ZIP/PBO checks including embedded hashes,
resource references, no `.anm` payloads, disabled old rate settings and the
unchanged working roll-to-stand source hash.

```powershell
pwsh -NoProfile -File tools/test_instadrop_graph_installer.ps1 -PackageZip /path/to/GRZ-InstaDrop-v0.1.9.zip -NativeArchive /path/to/fixture/anims_workspaces.pbo
```

Passed real fixture file operations for installation and hash checks on both
sides, backup and explicit restore (first install and upgrade), injected copy failure and rollback, wrong
version rejection, stale signature rejection and corrupted package rejection.
Only process detection and Windows executable version metadata are mocked.
Executed under PowerShell 7.6.9 on Linux; scripts use Windows PowerShell 5.1
syntax, but this is not a Windows integration test.

Build:

```sh
python tools/build_instadrop_graph_release.py --export /path/to/GRZ-DropGraph-export.zip --output downloads/GRZ-InstaDrop-v0.1.9.zip
```

The export is not committed. Source graph hashes and the change description are in GRAPH-CHANGES.json;
the authored patcher describes the eight property edits. No native graph or clip payload is packaged in the download. The PowerShell
installer builds the graph locally from the user's installed native archive;
source and generated-output hashes must match the checked prototype. The C#
patcher and PBO writer are exercised by the installer tests against a local
fixture of the supplied native graphs.

No Enforce compilation, native graph parsing or DayZ gameplay test is available
in this Linux environment. Actual drop timing, throw behavior, roll-to-stand
playback and network agreement still need the owner's in-game test.

Additional v0.1.9 check:

```sh
python tools/test_instadrop_roll_policy.py
```

The actual pure Enforce roll-policy class is host-compiled as compatible C++
and tested for the eligible rifle/on-back/preparation case, each mandatory
guard and combinations of blocked actions. This does not compile the native
PlayerBase wrapper or establish that DayZ invokes CanRoll for this input.
The roll-entry edges require native CMD_Roll. Recovery edges require native standing stance and at least 0.12 seconds of roll playback. The host policy test verifies that an on-back pose alone cannot trigger a forced stand.
