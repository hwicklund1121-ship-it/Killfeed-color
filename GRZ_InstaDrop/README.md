# GRZ InstaDrop prototype

Independent Enforce source using Bohemia's public `DayZPlayerUtils.DebugOverrideAnimationSpeed` API. No LUGU source or assets were used. This is a prototype for faster vanilla prone transitions; it does not restore the 1.27 animation graph or guarantee the reference video's motion.

Uses the ordinary prone/recovery controls. The graph still decides which transition is played; no forced stance changes, position jumps or new hotkeys. Initial requests: drop 2.0, recovery 1.6. Those numbers are animation API requests, not measured transition times.

Status: first rifle test build, using 6 drop and 10 recovery paths confirmed present in the supplied 8,992-entry native filename index. Standing/crouching, lowered/raised, moving drop/recovery and on-back recovery variants are included. Injured and non-rifle transitions retain native behavior. Path names identify candidate directions, but graph usage and visible motion still require in-game verification. DayZ compilation and multiplayer playback have not been tested here.

Build: `python tools/build_instadrop.py --index INDEX --preset packaging/instadrop-update/TRANSITIONS.json`. Only filenames are used; native animations are not redistributed. The index checks existence, not animation contents or graph usage.

Install: close DayZ and stop the server. Extract the prototype ZIP and run `powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-InstaDrop.ps1`. It backs up prior copies and installs only the two new addons in @GRZ_Local/Addons on both sides, verifies hashes and attempts rollback if copying fails. It does not replace GRZ_Movement or alter its 1.50 setting, outfits or profiles. This unsigned development build assumes your current local-addon testing setup; signing and server signature settings are not changed.

For an in-game test, install both generated PBOs on client and server alongside the existing GRZ movement addon. Remove LUGU from both active mod lists; GRZ refuses to activate when its known patch is present. Restart both. Compare normal prone, running-to-prone, raised rifle drop/recovery, first/third person and a second player's view. Check logs for every accepted/rejected clip. Confirm weapons, collision, inputs and current running speed still behave correctly. Roll back by removing only the two new GRZ InstaDrop PBOs and restarting both.

This prototype is kept separate from the existing reskin/movement updater. Nothing has been published for this task.

Version 0.1.1: pressing a native stance-change control while a firearm is in throw-ready/charge mode cancels that preparation, then delegates to native movement. This covers dropping to prone and returning from prone to crouch/stand using ordinary controls. It does not force or teleport stances, bypass roll/action/water checks, or interrupt the actual item-release animation. Grenade/non-firearm throwing remains native. The early command hook and the stance-query hook cover both callback orders; passive stance queries do not cancel throwing. Hold/release behavior and standing during an in-progress transition still require an in-game check; this is not a guaranteed restoration of the 1.27 graph.
