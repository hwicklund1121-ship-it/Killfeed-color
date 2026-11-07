GRZ independent movement development — native animation index

Extract the ZIP, open PowerShell in that folder, run:
powershell -ExecutionPolicy Bypass -File .\Export-Native-Animations.ps1

For a different game installation:
powershell -ExecutionPolicy Bypass -File .\Export-Native-Animations.ps1 -GamePath "D:\SteamLibrary\steamapps\common\DayZ"

Send the resulting GRZ-Native-Animations.json. This helper reads only vanilla Addons PBO header filenames, never their animation/script contents. It changes no game files and installs no mod. Do not point it at LUGU or another third-party mod.

The target is the quick rifle/prone drop and recovery seen in the supplied reference video. The index identifies available native clips/graphs; it does not expose graph source or prove that the current engine can reproduce DayZ 1.27 exactly. Any independent GRZ implementation will need a client/server in-game check with LUGU disabled.
