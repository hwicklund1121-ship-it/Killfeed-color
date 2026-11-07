# GRZ Discord logging bot

Run this on your PC now; move the complete folder to the server box later. The bot reads the **DayZ server's** `GRZ_Logs` files and posts to the existing 19 channels. It never deletes, creates or renames Discord channels or roles. This is separate from the recovery bot.

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

The bot matches these names, including decorative separators/emoji before the name:

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
