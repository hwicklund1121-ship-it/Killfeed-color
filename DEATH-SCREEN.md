GRZ Death Screen 0.2.0 — Made By Blake

Copy GRZ_DeathScreen.pbo into @GRZ_Local/Addons on both game and server.
Restart both and enable @GRZ_Local in the client launcher.
Uses the supplied artwork; no new text is added.
Shows the death camera for 2 seconds, then fades the artwork in over 0.8 seconds.
Repeated death/cleanup notifications do not restart the transition.
The respawn button and mouse focus activate when the fade finishes.
The gameplay HUD root is hidden throughout death/respawn, then its prior visibility is restored.
Death artwork has a visible RESPAWN button using the native InGameMenu.GameRespawn(false) flow.
Mouse cursor/focus is available while dead; ESC/native menus remain accessible.
Server respawn hooks determine the location: GRZ Map Rotation places the player in the active arena.
Keep GRZ_MapRotation.pbo installed on both sides; clothing/outfit hooks remain in the native spawn chain.
Artwork hides while ESC/respawn/confirmation menus are open so controls remain accessible.
Artwork clears on respawn and mission exit. Unconsciousness is unchanged.
Unsigned development addon. Sign with your own key if required by your server.
Package and native script signatures checked; not compiled or tested in DayZ.
