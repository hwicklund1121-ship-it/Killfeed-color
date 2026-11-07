# GRZ Discord logging bot

Run this on your PC now; move the complete folder to the server box later. The bot reads the **DayZ server's** `GRZ_Logs` files and posts to the existing channels and `anticheat-logs`. It never deletes, creates or renames Discord channels or roles. This is separate from the recovery bot.

## Set up on your PC

1. Install Python 3.11 or newer for Windows. Install the paired `GRZ-Arena-Menu-Update.zip` on the DayZ server and client using its existing `Install-Update.ps1`. Start DayZ server once; the first logged event creates `<server profiles>\GRZ_Logs\events-*.jsonl`.
2. Extract this bot ZIP. In Discord Developer Portal, create a new application and bot (or use your existing bot), copy the bot token into a **local** `.env` file copied from `.env.example`. Never send the token in chat or upload `.env`.
3. Fill `SERVER_ID` with your Discord server ID (enable Discord Developer Mode, right-click the server, Copy Server ID). Set `AUDIT_DIRECTORY` to the DayZ **server** profile `GRZ_Logs` directory. Use the `-profiles=` path in `StartServer.bat`, not the client `%LOCALAPPDATA%\DayZ` directory.
4. Invite the bot using scopes `bot` and `applications.commands`, and only **View Channels**, **Send Messages**, **Embed Links** (permission value `19456`). No privileged intents or Administrator permission are needed. Make sure the Deathmatch category allows these permissions in its channels.
5. Open CMD in the extracted folder and run:

   ```cmd
   Start-Logging-Bot.cmd
   ```

   From PowerShell: `cmd /c .\Start-Logging-Bot.cmd`. The first run creates `.env` and opens Notepad if missing; save it, then run the command again. Keep the window open. Ctrl+C stops the bot. Check `/grz_logs_status` in Discord (Manage Server permission required).

The bot matches these 20 names, including decorative separators/emoji before the name:

| Events | Channel(s) |
|---|---|
| Challenges sent | duel-logs, duel-invites |
| Challenges accepted | duel-logs, duel-accepts |
| Declined/cancelled/expired challenges | duel-logs, duel-rejects |
| Round starts, round scores, completed/forfeit/draw results | duel-logs |
| Match wins / losses | duel-wins / duel-loses |
| Equipped weapons and wraps | weapon-changes |
| Applied clothing / bags / removed clothing | clothing-changes |
| Player deaths and attributed kills | killfeed |
| VIP access and denied menu access | vip-logs |
| Saved prefixes / killfeed style | vip-prefixs / vip-killfeed, also vip-logs |
| Paid weapon / wrap changes | vip-weapon-changes, vip-logs, weapon-changes |
| Paid clothing / vest changes | vip-clothing-changes, vip-logs, clothing-changes |
| Arena starts/changes, vote opens/closes | map-rotation-logs |
| Votes and changed votes | map-votes |
| Admin skips and rejected skip attempts | map-skips, map-rotation-logs |
| Kill/death counter changes, saved duel Elo and leaderboard views | leaderboard-logs |
| Elo tier promotion | rank-ups, leaderboard-logs |
| Server gameplay review flags and startup status | anticheat-logs |

`duel-loses` and `vip-prefixs` intentionally match your channel spelling. Stats events distinguish live counters from saved duel ratings. Weapon grants during respawn/duels are marked as restores. Inventory previews and failed inventory transactions do not produce successful change logs. Kill attribution uses the existing death hook and the held weapon/source available there; delayed projectile/environmental kills may have an unknown source. Changes made directly by unrelated admin mods are outside these GRZ menu hooks.

If channel names are duplicated, or you want different names, copy `channel-overrides.example.json` to `channel-overrides.json` and map the standard keys to channel IDs, for example:

```json
{"duel-logs": "YOUR_NUMERIC_CHANNEL_ID", "vip-prefixs": "YOUR_NUMERIC_CHANNEL_ID"}
```

Replace the placeholders with numeric IDs. Overrides must point to text channels in `SERVER_ID`. Ambiguous/missing channels remain queued instead of being guessed. Restart after changing overrides.

## PC first, server box later

The bot needs access to **live server logs**. If DayZ server currently runs on your PC, use its local path. If DayZ runs on another machine, use a Windows shared path such as `\\YOUR-SERVER\DayZLogs` (read permission only), a mounted shared directory, or a continuously synced copy. A bot on your PC cannot read a remote box's `C:\...` path directly. Keep the source boot filenames unchanged when syncing.

To migrate: stop the PC bot with Ctrl+C, copy this folder **including `.env` and `logging_data`** privately to the server box, update `AUDIT_DIRECTORY`, and run `Start-Logging-Bot.cmd`. Copying `logging_data` preserves delivered-message checkpoints and the pending queue. `.venv` can be omitted and recreated automatically. Run only one copy; two machines with separate databases can duplicate messages. For Task Scheduler, run this CMD file at startup with the bot folder as “Start in”; ensure that account can read the audit directory. Network shares may require a UNC path instead of a mapped drive.

## Reliability and troubleshooting

- SQLite queues each event and its file checkpoint in the same transaction. It retries after Discord outages, rate limits and permission errors; a missing channel does not block other channels.
- The local SQLite database and source audit files are retained. Monitor disk space and archive old logs only after delivery. Do not delete `logging_data` unless you want a fresh replay. Partial final lines wait until complete; malformed/unknown records go into the database's `rejected` table for review.
- Delivery is at least once: a crash immediately after Discord accepts a message but before the local acknowledgement can duplicate that message. The event ID in the footer identifies it.
- Logs contain player names and Steam IDs. Keep log channels staff-only and protect your PC/server folders.
- No messages: check `/grz_logs_status`, the bot console, channel permission overrides, and that **new** `events-*.jsonl` records appear when you change gear or vote. Original unmodified DayZ/GRZ addons do not generate these audit events; install the paired update on both sides.
- Audit write errors appear once per server session as `[GRZ Audit]` in the server script log. Logging failures do not cancel gameplay actions.
- Run offline tests without Discord or a token: `python -m unittest discover -s tests -v` (the formatting tests require installed requirements).

Validation covers local file ingestion, durable queue/restarts, routing, and mocked Discord delivery. The actual Windows DayZ/Discord integration still needs an in-game event test after installation.

## Gameplay alert update

Create **#anticheat-logs** and let the bot view it, send messages and embed links. To use a differently named channel, put its numeric ID under `"anticheat-logs"` in `channel-overrides.json`. The bot never creates or changes channels. Staff-only access is recommended because alerts include player Steam IDs and positions.

Stop the bot with Ctrl+C. Replace `bot.py` and `engine.py` with this update (or copy the updated source files over your current bot folder), preserving `.env`, `channel-overrides.json` and the whole `logging_data` folder. Run `.\Start-Logging-Bot.cmd` again in PowerShell. Install the paired DayZ update and restart server and client. Check `/grz_logs_status`; server startup posts a gameplay alert status message.

The visible **GRZ Audit Logs and Gameplay Alerts** addon uses server gameplay events only. There is no keyboard capture, Insert detection or hidden no-grass modification. These are staff review flags, never a verdict, kick or ban:

- On-foot speed: over 16 m/s for 3 consecutive samples, sampled once per second. Skips vehicles, falls, climbing, swimming, dead/unconscious players, short samples, and server stalls over 2.5 seconds.
- Position jumps: a horizontal move exceeding 120 metres in one valid sample without a GRZ teleport exemption. Admin tools or replication correction can explain these; review context.
- Firing cadence: at least 6 observed shots spanning at least 2 seconds per window, with average spacing below 55% of that weapon's runtime shot interval, repeated over 2 windows. Skips burst/multi-muzzle weapons and resets on weapon owner/mode/timing changes and long pauses. Server callbacks and network buffering cannot establish every client shot time.
- Projectile damage: at least 2 calculated zone-health damage outliers within 10 seconds, each exceeding 8 times the ammo's configured base health damage. Other damage/zone mods can explain these. Unknown ammo configurations are skipped.

Each check has a 60-second per-player cooldown. New player entities receive 20 seconds of spawn grace. GRZ arena rotation, boundary returns, duel side resets and duel returns explicitly exempt their teleports for 10 seconds. Other admin/teleport mods can call `GRZGameplayAlerts.AllowTeleport(player, seconds)` before moving a player, or you can add staff Steam IDs to `ExemptSteamIds`.

Settings are created on the server at `profiles/GRZ_Logs/gameplay-alerts.json`. Set `Enabled` or individual `MovementEnabled`, `FireRateEnabled`, `DamageEnabled` to false to disable checks; tune thresholds for your installed movement/weapon/damage mods, then restart. Existing no-grass files, VIP permissions, saved gear, Discord credentials and queue state are unchanged.

These heuristics can flag replication/lag and legitimate mod/admin actions, and cannot detect all cheats. Validate thresholds with normal gameplay before acting on alerts. The pure numerical rules and Discord routing have automated tests; live DayZ multiplayer behavior still requires an in-game check.
