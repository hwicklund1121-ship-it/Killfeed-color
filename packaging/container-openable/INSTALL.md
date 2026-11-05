GRZ_Container_Openable - native open/close door variant

Spawn/display name: GRZ_Container_Openable. Description: Made By Blake.
Separate from the existing closed GRZ_Container and GRZ_Double_Container.

Both front doors open/close together using the standard DayZ building-door
action. The artwork follows the original 1A FBX UVs. The visual door leaves,
hinges, handles and locking bars have named selections. New convex wall/floor/
roof/rear collision components plus two independently animated door collision
panels match the model. Memory LOD supplies two hinge axes and an action point.
Model.cfg defines the door skeleton and paired rotations. No custom network
RPC or broad changes to other building actions are introduced.

IMPORTANT: this is a complete source/build project, not a precompiled door PBO.
The Windows DayZ Binarize tool is required to embed model.cfg animations into
the P3D. That tool cannot be run in this Linux workspace. Packaging a raw MLOD
and calling it a working animated PBO would not establish functional doors.

Build on Windows:
1. Install DayZ Tools and configure its project drive/game data. The builder
   uses P:\dz by default for vanilla penetration/roadway resources. A different
   data directory can be supplied with -GameDataPath.
2. Extract the entire ZIP into a writable folder. In PowerShell there, run:
   cmd /c .\Build-Openable-Container.cmd
3. For a non-default tools location, run:
   .\Build-Openable-Container.ps1 -ToolsPath 'D:\Steam\steamapps\common\DayZ Tools'
4. After Binarize succeeds, the script verifies the ODOL header and packs the
   result as BuildOutput\GRZ_Container_Openable.pbo. Raw/uncompiled models are
   rejected. Binarize errors go into BuildOutput\Binarize.log.
5. Stop the server/game; add the resulting PBO to @GRZ_Local\Addons on BOTH
   installations and restart both. Keep the existing single/double PBOs.
6. Spawn GRZ_Container_Openable and test the native Open/Close action, swinging
   direction, walking through the opening, closed-door collision and textures.

Advanced: if you already binarized this project's model with its model.cfg,
use -CompiledModelPath '<compiled container_openable.p3d>' to pack that output.
Do not pass a different container model: it would lack this door skeleton.

Source/model structure, UV assignment, named selections, collision components,
config links and ZIP/packing checks are available here. Native compilation,
door interaction, synchronization and collision need in-game confirmation.
The addon is unsigned; sign with your own key if your server requires it.

The native door pattern was checked against Bohemia's DayZ-Samples/Test_Building.
Model-format I/O uses the unmodified external Arma 3 Object Builder codec:
https://github.com/MrClock8163/Arma3ObjectBuilder (GPL-3.0, separate build tool).
