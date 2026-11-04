GRZ Low Recoil v0.1.0

Reduces ADS/quickscope recoil modifiers to 10% of their normal values. It applies while ADS is requested as well as while already in iron sights or optics. Hipfire recoil keeps its normal value. Native attachment modifiers are obtained first, then scaled. Native RecoilBase.PostInit snapshots the modifier per shot and uses all three components for mouse, hand and camera-Z recoil, so unscoping immediately does not restore a full-strength recoil tail.

This standalone addon changes only GetRecoilModifier. It does not change camera transition timing, weapon raise completion, native aiming tolerances, damage, hit reporting or projectile flight. It does not reintroduce GRZ_AimControl or GRZ_Quickscope. GRZ_BetterAim and the 0.75-second GRZ-logo blackout remain separate.

Put GRZ_LowRecoil.pbo in @GRZ_Local/Addons on BOTH client and server, then fully restart both. Keep the current native-damage compatibility GRZ_Hitscan.pbo and remove the old GRZ_Quickscope.pbo/GRZ_AimControl.pbo. Preserve custom weapons and the generated inventory override. To disable low recoil, remove only GRZ_LowRecoil.pbo from both sides and restart.

Standard RecoilBase weapon profiles are covered. A custom weapon recoil class that overrides GetRecoilModifier without calling super may require its own integration. The actual amount of movement still depends on the weapon's profile; this is a 90% modifier reduction, not a guarantee of a particular pixel displacement.

Static checks compare the override signature and parameter name to the native API, confirm native PostInit uses the returned modifier for camera kick, and verify archive payloads. No Enforce compiler or DayZ runtime is available here. Required in-game checks: single/repeated ADS shots, immediate unscope, normal hipfire, scoped and unscoped native/custom UAV guns, retained native damage/hitmarkers, and no change to scope-entry timing.
