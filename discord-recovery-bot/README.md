# Discord Recovery Bot

Deletes reviewed channels/categories in ONE configured server. Never edits/deletes roles or changes anyone's role assignments. Only the Discord server owner can command cleanup. No real Discord server was accessed while building this package.

## Windows setup

1. Install Python 3.11 or newer from python.org.
2. Create an application and Bot at https://discord.com/developers/applications . Copy its bot token into your LOCAL `.env`, never into chat or screenshots. `Start-Bot.cmd` creates `.env` from the example on first run.
3. Set `SERVER_ID` in `.env`. Enable Discord Settings > Advanced > Developer Mode, then right-click your server > Copy Server ID.
4. In the Developer Portal's Installation/OAuth2 URL generator, select **bot** and **applications.commands**. Request **Manage Channels**, **View Audit Log**, and **Kick Members**. Administrator and Manage Roles are NOT needed. Invite it to your server.
5. Move the bot role above the bots/accounts it must be able to kick. Channel permission overwrites must allow it to manage the channels being cleaned.
6. Add legitimate moderation/builder bots to `TRUSTED_USER_IDS` before running it, or protection may kick them during bulk actions. Owner and this bot are always exempt. Leave `AUTO_KICK=true` for active protection; false is monitor-only.
7. Run `Start-Bot.cmd` and keep its window running. Message Content/Server Members privileged intents are not required. The code enables guild moderation events; audit events also require View Audit Log permission.

## Delete channels, keep roles

As server OWNER, run `/cleanup_preview`. Review the attached channel list. Run `/cleanup_confirm` with its server ID and confirmation code within two minutes. The confirmation is single-use. Only listed channels are deleted; roles stay. Deleting parent channels also removes their messages and threads. This cannot be undone by this bot.

The command channel is deleted last. Completion and failed channel IDs appear in the bot console, since the interaction channel may no longer exist. After cleanup, create a new channel manually in Discord to use slash commands again. Failed deletions can be retried using a new preview. Discord can block deletion of required Community channels; disable Community in server settings first if those also need to go.

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

After installing requirements, run `python -m unittest -v test_bot.py`. Tests use mocks, never real Discord connections. They cover owner restrictions, confirmation, snapshot-only deletion, channel order, exemptions, duplicate/expired audit events and kick hierarchy. Not tested against a live Discord server.
