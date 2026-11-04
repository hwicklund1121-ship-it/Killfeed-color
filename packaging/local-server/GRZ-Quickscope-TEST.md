GRZ Quickscope v0.1.0 — custom test build

This is a custom quickscope approximation, not an exact 1.29 rollback. The official 1.29 (86974a0, build 1.29.163709) and 1.30 (78be1c4, build 1.30.164289) script snapshots have the same scope-entry transition constants and weapon-raise completion logic. Relevant scoped-camera changes concern night vision; native engine, weapon config and animation changes cannot be reconstructed from this script diff. No 1.29 engine or animation files are bundled.

Changes:

- Normal weapon camera transitions into and out of iron sights/optics are capped at 0.08 seconds. Handheld binoculars, vehicle, unconscious, jump and climb camera transitions retain native timing.
- The extra script weapon-raise delay completes as soon as a living, conscious player is natively raised and requesting ADS. Native obstruction, sprint, inventory/action and weapon-manager checks still run. This does not force firing while sprinting or reloading, or speed up bolt cycling or reload animations.
- Additional camera reticle lag and movement bob are removed while using weapon sight cameras. The separate no-sway addon handles breathing/wandering. Recoil and injury offsets remain in the native aiming model.
- Supported GRZ Hitscan shots fired while requesting/using ADS follow server-side base aiming angles plus native hand offsets instead of waiting for the animated muzzle direction to settle. The origin remains at the actual muzzle and the first blocking surface still stops the ray. No client-supplied target or damage RPC is introduced. Hipfire retains the existing hitscan muzzle direction.

Requires the matching updated GRZ_Hitscan.pbo and GRZ_NoSway.pbo. All five recent updates are in GRZ-latest-mod-updates.zip: Quickscope, corrected Hitscan, NoSway, AntiHardscope and Killfeed. Merge @GRZ_Local into BOTH game and server installations; preserve custom weapons and your generated inventory PBO. Restart both. The two-second anti-hardscope timer continues to start on actual optics entry and is not changed by quickscope.

Limitations: early-shot alignment applies only to the supported single-muzzle, nonexplosive, single-projectile GRZ Hitscan firearms and filtered target families described in GRZ-Hitscan-TEST.md. It cannot promise every custom gun's native animation, sight axis or networked aiming angles match its displayed reticle. Unsupported pellet/multi-muzzle/explosive weapons retain native projectile accuracy. Scope timing is a camera blend cap, not a replacement of all native raising animations.

Required in-game checks: first/third person, standing/crouched/prone ADS; tap shots before and after full scope entry at several distances; native and custom UAV guns; armour and head/body hits; nearby walls/cover; sprint, reload, bolt cycling, unconsciousness and death; hipfire; handheld binoculars; two-second blackout and reset; no delayed duplicate hits. Check that server-replicated aim follows client aim under normal latency. These behaviors are unverified in this workspace because no DayZ/Enforce runtime is available.

To disable only quickscope, remove GRZ_Quickscope.pbo from BOTH sides and restart. Corrected GRZ_Hitscan.pbo defaults to its original muzzle ray without this addon. No save or mission files are changed.
