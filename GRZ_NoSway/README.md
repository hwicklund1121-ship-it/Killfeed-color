GRZ No Sway v0.1.0

Removes the native aiming model breathing and horizontal wandering offsets for all weapons using that model. Calls the original helpers first to retain native state and synchronized random draws. Recoil, injury shakes, weapon dispersion, damage and projectile flight remain native.

Merge GRZ_NoSway.pbo into @GRZ_Local/Addons on both client and server, then fully restart both. Keep UAVWEAPONS.pbo and your generated GRZ_InventorySizes.pbo.

This addon does not implement hitscan or no bullet drop. The exposed TryFireWeapon function is native; EEFired/OnFire run after projectile creation. A second scripted raycast there can duplicate damage. A verified replacement for native projectile emission is still needed.

Packed and checked against native helper signatures; Enforce compilation and in-game behavior require DayZ testing.
