# Discord Recovery Bot

Rebuilds ONE configured server by deleting its current channels and recreating old channels from Discord audit-log metadata. Standalone channel cleanup is also available. Never edits/deletes roles or changes anyone's role assignments. Only the Discord server owner can command cleanup. No real Discord server was accessed while building this package.

## Windows setup

1. Install Python 3.11 or newer from python.org.
2. Create an application and Bot at https://discord.com/developers/applications . Copy its bot token into your LOCAL `.env`, never into chat or screenshots. `Start-Bot.cmd` creates `.env` from the example on first run.
3. Set `SERVER_ID` in `.env`. Enable Discord Settings > Advanced > Developer Mode, then right-click your server > Copy Server ID.
4. In the Developer Portal's Installation/OAuth2 URL generator, select **bot** and **applications.commands**. Request **Manage Channels**, **View Audit Log**, and **Kick Members**, **View Channels**, **Send Messages**, and **Attach Files**. Administrator and Manage Roles are NOT needed. Invite it to your server.
5. Move the bot role above the bots/accounts it must be able to kick. Channel permission overwrites must allow it to manage the channels being cleaned.
6. Add legitimate moderation/builder bots to `TRUSTED_USER_IDS` before running it, or protection may kick them during bulk actions. Owner and this bot are always exempt. Leave `AUTO_KICK=true` for active protection; false is monitor-only.
7. Run `Start-Bot.cmd` and keep its window running. Message Content/Server Members privileged intents are not required. The code enables guild moderation events; audit events also require View Audit Log permission.

## Rebuild: delete CURRENT channels, recreate OLD ones, keep roles

Keep one temporary text channel available to run commands. If you already used the old cleanup command, create a temporary channel manually first. Use these commands as the Discord SERVER OWNER:

1. `/rebuild_preview hours:24` scans recent channel deletions and saves a local snapshot of current channel settings in `recovery_data/`. It does not delete anything. To isolate the attack, optionally set `deleted_by_id` to the attacker's numeric user/bot ID, and adjust hours from 1 to 1080 (45 days).
2. Read the attached **current channels to delete** and **old channels to recreate** lists. Make sure the old list contains your original channels rather than deleted spam. Rerun the preview with a narrower window or attacker ID if necessary. By default, deletion entries from this recovery bot are excluded; selecting its ID explicitly includes them.
3. Run `/rebuild_confirm server_id:YOUR_SERVER_ID confirmation:CODE` using the values from the preview within 10 minutes. This is single-use. If the current channel IDs changed after preview, it refuses deletion and requires a fresh preview.
4. The bot creates a private temporary progress channel before wiping the reviewed current channels. It checks permissions first and refuses to proceed without a non-empty recovery list. Required Community channels must be unprotected by disabling Community manually before the wipe can proceed. If that disables old announcement/forum/stage/media types, first let ordinary channels recover, re-enable Community using the recreated rules/updates channels, then resume the same plan for remaining channel types.
5. It recreates categories first, then text, announcement, voice, stage, forum and media channels where supported by the server. Existing roles and role assignments are never modified. It copies recorded channel permission overwrites only onto existing roles/members. Incomplete permissions, missing referenced roles, or permission bits the bot cannot grant cause a PRIVATE channel for owner review, rather than an accidental public channel.
6. Progress, the plan ID, private fallbacks and failures appear in the bot console. Keep that window running. Messages in the original command channel disappear during deletion. The temporary progress channel is removed after complete success.

**If interrupted:** keep `recovery_data/` and run `/rebuild_resume plan_id:CODE`. This continues the already-confirmed original deletion list if necessary, then the pending restores. It does not add new channels to the deletion list. It tracks created IDs and reconciles matching bot creation audit entries after an interrupted journal write to reduce duplicate restores. Do not rerun a fresh rebuild over partially restored channels unless you intend to delete them too.

Audit logs are retained for about 45 days and do not contain messages, attachments or full channel backups. Names/types are required for recovery. Category membership, ordering, permission overwrites and other settings are restored only where recorded; missing category links leave channels uncategorized. Forum tags/threads/posts, original IDs, links, webhook tokens and missing roles cannot be reconstructed. News/stage/forum/media creation can fail if the server no longer supports those channel types. Discord permissions, channel limits and API failures can cause partial recovery. Current-channel snapshots are metadata backups, not message backups. Nothing has been run on your server by the package author.

For intentional standalone removal without restoration, `/cleanup_preview` followed by `/cleanup_confirm` remains available. Use the rebuild flow above for recovery.

## Anti-nuke protection

Within a 10-second window, an untrusted actor is kicked at these thresholds:

| Action | Threshold |
|---|---:|
| Delete role | 1 |
| Grant a dangerous role permission | 1 |
| Delete channel | 3 |
| Create channel | 5 |
| Edit channel / permission overwrites | 6 |
| Create role | 5 |
| Edit role | 3 |
| Ban member | 3 |
| Create webhook | 3 |

Dangerous grants include Administrator, Manage Server/Roles/Channels/Webhooks, Ban Members and Kick Members.

`/protection_status` shows the settings. The bot's confirmed cleanup is exempt. Duplicate/stale audit events do not count. Storage is bounded. Edit thresholds in `protection.py`. Keep the bot running on a trusted machine for continuous protection.

Protection is reactive: it cannot prevent the first actions, restore deleted data, stop the server owner, or kick an actor whose highest role is at/above its own. It does not revoke existing compromised webhooks. Review accounts/bots with dangerous permissions before rebuilding.

## Verification

After installing requirements, run `python -m unittest discover -v -p "test_*.py"`. Tests use mocks, never real Discord connections. They cover owner restrictions, confirmation, snapshot-only deletion, channel order, exemptions, duplicate/expired audit events and kick hierarchy. Not tested against a live Discord server.
