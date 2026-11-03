# GRZ Survival

DayZ PC development mod v0.1.0. Applies to all players on the server.

- No fall damage: blocks the fall-specific handler, including health/shock damage, fall injuries, bleeding and gear damage. Landing and movement still run through the native caller.
- Energy and water stay at their configured maximum, and hunger/thirst metabolism modifiers do not apply starvation or dehydration damage.
- Heat comfort stays neutral (zero); heat buffer and thermal bias are reset. Both native environment heat-comfort paths and the temperature modifier are covered so weather and temperature sources cannot make the player cold/hot or build heat stroke.

All stat changes are server-side and run before/after the normal scheduled tick. Other systems continue their native updates. This is not general invulnerability: combat, unrelated bleeding, sickness, drowning and direct fire/explosion damage remain possible. Existing damage is not healed. No health/blood values or hitmarker/audio files are changed.

Install GRZ_Survival.pbo on the server and clients, or use the updated @GRZ_Local pack. No config or slash command is required. Existing CF/VPP server startup settings remain unchanged. Build with `python3 tools/build_survival.py`.

Static/archive checks are performed in the build. In-game checks are still required: small/large falls and attached equipment, long sprinting, rain/snow/swimming, fires/hot clothing, reconnect/respawn, and ordinary weapon damage. Other mods that replace these hooks without calling super can need compatibility changes.
