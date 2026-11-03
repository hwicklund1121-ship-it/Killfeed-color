# GRZ No Grass

Development build v0.1.0 for your current ChernarusPlus server (`dayzOffline.chernarusplus`). Overrides the map's terrain clutter drawing distance to zero. Terrain textures, trees, placed bushes and other world objects remain. Small plants drawn as terrain clutter disappear together with grass.

Included in @GRZ_Local. Stop DayZ and the server, replace @GRZ_Local in BOTH the client and server installations, then restart both. Enable only one copy of the GRZ pack in the client launcher. No config or startup command change is required.

This build targets ChernarusPlus; other maps need their own map patch. It uses the map addon's load dependency and changes no scripts, object collisions or game survival logic. Remove GRZ_NoGrass.pbo from both installations and restart to restore normal clutter rendering.

Build: `python3 tools/build_no_grass.py`. Archive/source checks run here; actual terrain rendering must be checked in DayZ (grass settings, reconnect and travel to another field).
