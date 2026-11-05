"""Owner-only channel cleanup and audit-based protection for one Discord guild."""
import asyncio
import io
import logging
import os
import secrets
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path

import discord
from discord import app_commands
from dotenv import load_dotenv

from protection import Protection
from recovery import PlanStore, Rebuilder, collect_deleted, current_record

load_dotenv()
log = logging.getLogger("recovery")
try:
    GUILD_ID = int(os.environ.get("SERVER_ID", "0"))
    TRUSTED = {int(x.strip()) for x in os.environ.get("TRUSTED_USER_IDS", "").split(",") if x.strip()}
except ValueError:
    raise SystemExit("SERVER_ID and TRUSTED_USER_IDS in .env must be numeric Discord IDs.")
TOKEN = os.environ.get("DISCORD_BOT_TOKEN", "").strip()
AUTO_KICK = os.environ.get("AUTO_KICK", "true").lower() == "true"


@dataclass
class Preview:
    code: str
    expires: float
    channel_ids: tuple[int, ...]


class RecoveryBot(discord.Client):
    def __init__(self):
        intents = discord.Intents.none()
        intents.guilds = True
        intents.moderation = True  # AUDIT_LOG_ENTRY_CREATE; no Message Content intent.
        super().__init__(intents=intents, allowed_mentions=discord.AllowedMentions.none())
        self.tree = app_commands.CommandTree(self)
        self.pending: dict[int, Preview] = {}
        self.cleanup_lock = asyncio.Lock()
        self.policy = Protection()
        self.interventions = {}
        self.rebuild_pending = {}
        self.recovery_store = PlanStore(Path(__file__).resolve().parent / "recovery_data")
        self.rebuilder = Rebuilder(self.recovery_store, log)

    async def setup_hook(self):
        guild = discord.Object(id=GUILD_ID)
        self.tree.copy_global_to(guild=guild)
        await self.tree.sync(guild=guild)

    async def on_ready(self):
        guild = self.get_guild(GUILD_ID)
        if not guild:
            log.error("Configured server is unavailable. Check SERVER_ID and the bot invite.")
            return
        log.info("Ready in %s. Auto-kick=%s; roles are never edited by this bot.", guild.name, AUTO_KICK)
        me = guild.me
        if me:
            perms = me.guild_permissions
            for name in ("manage_channels", "view_audit_log", "kick_members", "view_channel", "send_messages", "attach_files"):
                if not getattr(perms, name):
                    log.error("Missing permission: %s", name)

    async def on_audit_log_entry_create(self, entry: discord.AuditLogEntry):
        if entry.guild.id != GUILD_ID:
            return
        actor_id = entry.user_id
        if actor_id is None:
            log.warning("Audit entry has no actor; cannot safely attribute it.")
            return
        # Ignore delayed/replayed actions; protection concerns current attacks.
        age = (datetime.now(timezone.utc) - entry.created_at).total_seconds()
        if age > 30 or age < -5:
            return
        exempt = actor_id in TRUSTED or actor_id == entry.guild.owner_id or actor_id == self.user.id
        action = entry.action.name
        if action in {"overwrite_create", "overwrite_update", "overwrite_delete"}:
            action = "channel_update"
        if action == "role_update":
            before = getattr(entry.before, "permissions", None)
            after = getattr(entry.after, "permissions", None)
            if before is not None and after is not None:
                dangerous = ("administrator", "manage_guild", "manage_roles", "manage_channels", "manage_webhooks", "ban_members", "kick_members")
                if any(getattr(after, name, False) and not getattr(before, name, False) for name in dangerous):
                    action = "permission_escalation"
        now = time.monotonic()
        if not self.policy.observe(entry.id, actor_id, action, now, exempt=exempt):
            return
        # One intervention per actor per minute; avoid repeated kick requests.
        last = self.interventions.get(actor_id)
        if last is not None and now - last < 60:
            return
        self.interventions = {uid:stamp for uid,stamp in self.interventions.items() if now-stamp < 60}
        self.interventions[actor_id] = now
        log.warning("PROTECTION: actor=%s action=%s reached threshold.", actor_id, action)
        if not AUTO_KICK:
            log.warning("Monitor-only mode: no kick attempted.")
            return
        try:
            member = await entry.guild.fetch_member(actor_id)
            me = entry.guild.me
            if not me or not me.guild_permissions.kick_members or member.top_role >= me.top_role:
                log.error("Cannot kick actor %s: missing Kick Members or bot role is too low.", actor_id)
                return
            await member.kick(reason=f"Recovery protection: rapid {action} audit events")
            log.warning("PROTECTION: kicked actor %s. Roles were not changed.", actor_id)
        except discord.NotFound:
            log.info("Actor %s is already absent.", actor_id)
        except discord.HTTPException:
            log.exception("Could not kick actor %s", actor_id)


bot = RecoveryBot()


def owner_only(interaction: discord.Interaction):
    return bool(interaction.guild and interaction.guild.id == GUILD_ID and interaction.user.id == interaction.guild.owner_id)


@bot.tree.command(name="cleanup_preview", description="Owner only: review the channels to delete. Roles stay.")
@app_commands.guild_only()
@app_commands.check(owner_only)
async def cleanup_preview(interaction: discord.Interaction):
    if bot.cleanup_lock.locked():
        await interaction.response.send_message("Cleanup is already running. Check the bot console.", ephemeral=True)
        return
    channels = await interaction.guild.fetch_channels()
    if not channels:
        await interaction.response.send_message("There are no channels to delete.", ephemeral=True)
        return
    code = secrets.token_hex(6)
    bot.pending[interaction.user.id] = Preview(code, time.monotonic() + 120, tuple(c.id for c in channels))
    listing = "\n".join(f"{c.id}\t{c.type}\t{c.name}" for c in channels)
    text = (
        f"Server: **{interaction.guild.name}** (`{GUILD_ID}`)\n"
        f"Preview contains **{len(channels)} channels/categories**. Deleting parents also removes their threads.\n"
        "Roles will not be edited or deleted. Channel deletion cannot be undone by this bot.\n"
        f"Within 2 minutes run `/cleanup_confirm server_id:{GUILD_ID} confirmation:{code}`.\n"
        "The attached list is the exact deletion list. A new preview replaces this confirmation."
    )
    await interaction.response.send_message(text, file=discord.File(io.BytesIO(listing.encode()), filename="channels-to-delete.txt"), ephemeral=True)


@bot.tree.command(name="cleanup_confirm", description="Owner only: delete the channels from your reviewed preview.")
@app_commands.guild_only()
@app_commands.check(owner_only)
async def cleanup_confirm(interaction: discord.Interaction, server_id: str, confirmation: str):
    preview = bot.pending.get(interaction.user.id)
    valid = preview and preview.expires > time.monotonic() and secrets.compare_digest(preview.code, confirmation)
    if server_id != str(GUILD_ID) or not valid:
        await interaction.response.send_message("Wrong server/confirmation or preview expired. Run /cleanup_preview again.", ephemeral=True)
        return
    if bot.cleanup_lock.locked():
        await interaction.response.send_message("Cleanup is already running.", ephemeral=True)
        return
    bot.pending.pop(interaction.user.id, None)  # Single-use even if later API requests fail.
    await interaction.response.defer(ephemeral=True, thinking=True)
    async with bot.cleanup_lock:
        log.warning("OWNER CONFIRMED CLEANUP: %s channels; roles untouched.", len(preview.channel_ids))
        await interaction.edit_original_response(content="Deleting the reviewed channels. The bot console will show completion; this channel will disappear too.")
        channels = {c.id:c for c in await interaction.guild.fetch_channels()}
        caller = interaction.channel_id
        if isinstance(interaction.channel, discord.Thread):
            caller = interaction.channel.parent_id
        targets = [channels[cid] for cid in preview.channel_ids if cid in channels]
        targets.sort(key=lambda c: (c.id == caller, isinstance(c, discord.CategoryChannel), c.id))
        deleted = 0
        failed = []
        for channel in targets:
            try:
                await channel.delete(reason="Server owner confirmed recovery cleanup; preserve all roles")
                deleted += 1
                log.info("Deleted channel %s (%s)", channel.name, channel.id)
            except discord.NotFound:
                pass
            except discord.HTTPException as exc:
                failed.append(channel.id)
                log.error("Could not delete channel %s: %s", channel.id, exc)
            # discord.py also respects Discord's own bucket/global rate limits.
            await asyncio.sleep(0.5)
        log.warning("CLEANUP COMPLETE: deleted=%s failed=%s; roles untouched. Failed IDs: %s", deleted, len(failed), failed)
        log.warning("Create a new channel manually in Discord to use slash commands again.")


@bot.tree.command(name="protection_status", description="Owner only: show protection configuration.")
@app_commands.guild_only()
@app_commands.check(owner_only)
async def protection_status(interaction: discord.Interaction):
    await interaction.response.send_message(
        f"Auto-kick: **{AUTO_KICK}**\nWindow: **10 seconds**\n"
        "Limits: 3 channel deletes, 5 channel creates, 6 channel/overwrite edits, **1 role delete or dangerous permission grant**, "
        "5 role creates, 3 role edits, 3 bans or 3 webhook creates.\n"
        "Server owner, this bot and TRUSTED_USER_IDS are exempt. Protection needs View Audit Log and Kick Members. "
        "It cannot kick accounts whose highest role is at or above its own, and cannot stop actions before they happen. "
        "No roles or role assignments are changed.", ephemeral=True)



@bot.tree.command(name="rebuild_preview", description="Owner: review deleting current channels and recovering old ones from audit logs.")
@app_commands.guild_only()
@app_commands.check(owner_only)
async def rebuild_preview(interaction: discord.Interaction, hours: app_commands.Range[int, 1, 1080] = 24, deleted_by_id: str = ""):
    if bot.cleanup_lock.locked():
        await interaction.response.send_message("A cleanup/rebuild is already running.", ephemeral=True)
        return
    if deleted_by_id and not deleted_by_id.isdigit():
        await interaction.response.send_message("deleted_by_id must be the attacker's numeric user/bot ID, or left empty.", ephemeral=True)
        return
    bot.rebuild_pending.pop(interaction.user.id, None)
    await interaction.response.defer(ephemeral=True, thinking=True)
    try:
        old, skipped = await collect_deleted(interaction.guild, hours, int(deleted_by_id) if deleted_by_id else None, bot.user.id)
        current = await interaction.guild.fetch_channels()
        snapshot = [current_record(c) for c in current]
        # Preserve surviving categories that the old channels explicitly reference.
        old_ids = {r['old_id'] for r in old}
        needed_parents = {r.get('parent_id') for r in old}
        for record in snapshot:
            if record['kind'] == 4 and record['old_id'] in needed_parents and record['old_id'] not in old_ids:
                old.append(record)
        old.sort(key=lambda r: (r['kind'] != 4, r.get('position') or 0, r['old_id']))
        if len(old) > 500:
            raise ValueError("More than 500 channels/categories in the plan. Narrow the incident window.")
        code = secrets.token_hex(6)
        plan = dict(code=code, guild_id=GUILD_ID, owner_id=interaction.guild.owner_id,
                    created_at=datetime.now(timezone.utc).isoformat(), approved=False,
                    phase='prepared', old=old, current=snapshot,
                    current_ids=[c.id for c in current], restored={}, notes=[])
        bot.recovery_store.save(plan)  # Full local snapshot BEFORE any deletion.
        bot.rebuild_pending[interaction.user.id] = (code, time.monotonic() + 600)
        lines = ['CURRENT CHANNELS TO DELETE:']
        lines.extend(f"{r['old_id']} | type={r['kind']} | {r['name']}" for r in snapshot)
        lines.append('\nOLD CHANNELS/CATEGORIES TO RECREATE:')
        lines.extend(f"{r['old_id']} | type={r['kind']} | {r['name']} | parent={r.get('parent_id')} | permissions={'recorded' if r.get('permissions_known') else 'UNKNOWN: private fallback'} | deleted_by={r.get('deleted_by')}" for r in old)
        lines.extend(['', f'Skipped incomplete/unsupported audit entries: {skipped}',
                      'Missing category links/settings cannot be invented. Messages/files/old IDs cannot be recovered.',
                      'Review the deletion window/actor carefully: old spam can also appear in the audit log.'])
        text = (f"**REBUILD PREVIEW**: delete **{len(current)} current channels/categories**, then recreate **{len(old)} old channels/categories**. Roles stay.\n"
                "Review the attached lists. A local JSON backup/plan has been saved in recovery_data.\n"
                "Incomplete permissions will be restored private for owner review. Existing channels are not deleted until you confirm.\n"
                f"Within 10 minutes: `/rebuild_confirm server_id:{GUILD_ID} confirmation:{code}`\n"
                "If these aren't the right old channels, rerun this preview with fewer hours or deleted_by_id set to the attacker.")
        await interaction.edit_original_response(content=text, attachments=[discord.File(io.BytesIO('\n'.join(lines).encode()), filename='rebuild-preview.txt')])
    except ValueError as exc:
        await interaction.edit_original_response(content=str(exc))


@bot.tree.command(name="rebuild_confirm", description="Owner: execute the reviewed delete-current/recreate-old channel plan.")
@app_commands.guild_only()
@app_commands.check(owner_only)
async def rebuild_confirm(interaction: discord.Interaction, server_id: str, confirmation: str):
    pending = bot.rebuild_pending.get(interaction.user.id)
    if (server_id != str(GUILD_ID) or not pending or pending[1] <= time.monotonic()
            or not secrets.compare_digest(pending[0], confirmation)):
        await interaction.response.send_message("Wrong/expired confirmation. Run /rebuild_preview again.", ephemeral=True)
        return
    if bot.cleanup_lock.locked():
        await interaction.response.send_message("Another operation is running.", ephemeral=True)
        return
    await interaction.response.defer(ephemeral=True, thinking=True)
    async with bot.cleanup_lock:
        plan = bot.recovery_store.load(confirmation)
        current = await interaction.guild.fetch_channels()
        if {c.id for c in current} != set(plan['current_ids']):
            await interaction.edit_original_response(content="Channels changed after the preview. Run /rebuild_preview again; nothing was deleted.")
            return
        bot.rebuild_pending.pop(interaction.user.id, None)
        plan['approved'] = True
        bot.recovery_store.save(plan)
        await interaction.edit_original_response(content=f"Starting rebuild {confirmation}. This channel will be deleted. Progress is in the bot console. If interrupted, use /rebuild_resume with this plan ID.")
        try:
            await bot.rebuilder.run(interaction.guild, plan)
        except ValueError as exc:
            log.error("Rebuild paused: %s", exc)
            try:
                await interaction.edit_original_response(content=str(exc))
            except discord.HTTPException:
                pass


@bot.tree.command(name="rebuild_resume", description="Owner: resume an already-confirmed rebuild plan after an interruption.")
@app_commands.guild_only()
@app_commands.check(owner_only)
async def rebuild_resume(interaction: discord.Interaction, plan_id: str):
    if bot.cleanup_lock.locked():
        await interaction.response.send_message("Another operation is running.", ephemeral=True)
        return
    try:
        plan = bot.recovery_store.load(plan_id)
    except (ValueError, FileNotFoundError):
        await interaction.response.send_message("Plan not found. Keep the original recovery_data folder.", ephemeral=True)
        return
    if not plan.get('approved') or plan['guild_id'] != GUILD_ID or plan['owner_id'] != interaction.user.id:
        await interaction.response.send_message("That plan was not confirmed by this server owner.", ephemeral=True)
        return
    await interaction.response.defer(ephemeral=True, thinking=True)
    async with bot.cleanup_lock:
        plan = bot.recovery_store.load(plan_id)
        await interaction.edit_original_response(content="Resuming the confirmed plan. Only its original deletion list is eligible; restored channel IDs are tracked. See the bot console.")
        try:
            await bot.rebuilder.run(interaction.guild, plan)
        except ValueError as exc:
            log.error("Rebuild paused: %s", exc)
            try:
                await interaction.edit_original_response(content=str(exc))
            except discord.HTTPException:
                pass


@bot.tree.error
async def command_error(interaction: discord.Interaction, error: app_commands.AppCommandError):
    if isinstance(error, app_commands.CheckFailure):
        text = "Only the configured server's owner can use these commands."
    else:
        log.error("Slash command failed", exc_info=(type(error), error, error.__traceback__))
        text = "Command failed. Check the bot console."
    try:
        if interaction.response.is_done():
            await interaction.followup.send(text, ephemeral=True)
        else:
            await interaction.response.send_message(text, ephemeral=True)
    except discord.HTTPException:
        log.error("Could not send command result; the channel may have been deleted.")


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(name)s: %(message)s")
    if not TOKEN or TOKEN.startswith("PASTE_") or GUILD_ID <= 0:
        raise SystemExit("Set DISCORD_BOT_TOKEN and SERVER_ID in .env first. Never share the token.")
    bot.run(TOKEN, log_handler=None)
