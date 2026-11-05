"""Audit-log rebuild plans. No role creation, role edits or role assignments."""
import asyncio
import json
from datetime import datetime, timedelta, timezone
from pathlib import Path

import discord

SUPPORTED = {0, 2, 4, 5, 13, 15, 16}


def snowflake(value):
    if value is None:
        return None
    return int(getattr(value, 'id', value))


def encode_overwrites(guild, pairs):
    result = []
    for target, overwrite in pairs:
        allow, deny = overwrite.pair()
        is_role = (guild.get_role(target.id) is not None or isinstance(target, discord.Role)
                   or getattr(target, 'type', None) is discord.Role)
        result.append(dict(id=target.id, kind='role' if is_role else 'member', allow=allow.value, deny=deny.value))
    return result


def deleted_record(entry):
    old = entry.before
    name = getattr(old, 'name', None)
    channel_type = getattr(old, 'type', None)
    kind = getattr(channel_type, 'value', channel_type)
    if not name or kind not in SUPPORTED or not entry.target:
        return None
    parent = getattr(old, 'parent_id', getattr(old, 'category', None))
    result = dict(old_id=entry.target.id, name=name, kind=kind,
                  parent_id=snowflake(parent), position=getattr(old, 'position', 0),
                  deleted_at=entry.created_at.isoformat(), deleted_by=entry.user_id,
                  permissions_known=hasattr(old, 'overwrites'))
    result['overwrites'] = encode_overwrites(entry.guild, getattr(old, 'overwrites', []))
    for key in ('topic', 'nsfw', 'slowmode_delay', 'bitrate', 'user_limit', 'default_thread_slowmode_delay'):
        if hasattr(old, key):
            result[key] = getattr(old, key)
    return result


def current_record(channel):
    result = dict(old_id=channel.id, name=channel.name, kind=channel.type.value,
                  parent_id=getattr(channel, 'category_id', None), position=channel.position,
                  permissions_known=True, overwrites=encode_overwrites(channel.guild, channel.overwrites.items()))
    for key in ('topic', 'nsfw', 'slowmode_delay', 'bitrate', 'user_limit', 'default_thread_slowmode_delay'):
        if hasattr(channel, key):
            result[key] = getattr(channel, key)
    return result


async def collect_deleted(guild, hours, actor_id, bot_id):
    after = datetime.now(timezone.utc) - timedelta(hours=hours)
    records = {}
    skipped = 0
    scanned = 0
    async for entry in guild.audit_logs(limit=5001, action=discord.AuditLogAction.channel_delete, after=after, oldest_first=False):
        scanned += 1
        if scanned > 5000:
            raise ValueError('More than 5,000 deletion records. Narrow the hours or attacker ID before rebuilding.')
        if actor_id is not None and entry.user_id != actor_id:
            continue
        # Do not bring back spam removed by this recovery bot unless explicitly selected.
        if actor_id is None and entry.user_id == bot_id:
            continue
        record = deleted_record(entry)
        if record:
            records.setdefault(record['old_id'], record)
        else:
            skipped += 1
    if not records:
        raise ValueError('No recoverable channel deletions found. Nothing will be deleted.')
    if len(records) > 500:
        raise ValueError('More than 500 old channels found. Narrow the hours or attacker ID to one incident.')
    return sorted(records.values(), key=lambda r: (r['kind'] != 4, r.get('position') or 0, r['old_id'])), skipped


class PlanStore:
    def __init__(self, directory):
        self.directory = Path(directory)

    def path(self, code):
        if len(code) != 12 or any(c not in '0123456789abcdef' for c in code):
            raise ValueError('Invalid recovery plan ID.')
        return self.directory / (code + '.json')

    def save(self, plan):
        self.directory.mkdir(parents=True, exist_ok=True)
        path = self.path(plan['code'])
        pending = path.with_suffix('.pending')
        pending.write_text(json.dumps(plan, indent=2), encoding='utf-8')
        pending.replace(path)

    def load(self, code):
        return json.loads(self.path(code).read_text(encoding='utf-8'))


class Rebuilder:
    def __init__(self, store, log):
        self.store, self.log = store, log

    async def overwrites(self, guild, record):
        if not record.get('permissions_known'):
            return self.private_overwrites(guild), True
        result = {}
        for row in record['overwrites']:
            # Discord cannot grant/deny overwrite bits the bot lacks itself.
            # Prefer an owner-review private channel over a failed/public restore.
            available = guild.me.guild_permissions.value
            if not guild.me.guild_permissions.administrator and ((row['allow'] | row['deny']) & ~available):
                return self.private_overwrites(guild), True
            if row['kind'] == 'role':
                target = guild.get_role(row['id'])
                if target is None:
                    # Never create missing roles or quietly broaden access.
                    return self.private_overwrites(guild), True
            else:
                target = guild.get_member(row['id'])
                if target is None:
                    try:
                        target = await guild.fetch_member(row['id'])
                    except discord.NotFound:
                        continue  # This person has left the server.
                    except discord.HTTPException:
                        return self.private_overwrites(guild), True
            result[target] = discord.PermissionOverwrite.from_pair(discord.Permissions(row['allow']), discord.Permissions(row['deny']))
        return result, False

    @staticmethod
    def private_overwrites(guild):
        return {guild.default_role: discord.PermissionOverwrite(view_channel=False),
                guild.me: discord.PermissionOverwrite(view_channel=True, send_messages=True, attach_files=True)}

    async def create(self, guild, record, plan, live):
        parent = None
        parent_id = record.get('parent_id')
        if parent_id:
            mapped = plan['restored'].get(str(parent_id), parent_id)
            parent = live.get(int(mapped))
            if not isinstance(parent, discord.CategoryChannel):
                parent = None
                plan['notes'].append(f"{record['name']}: original category could not be resolved; restored uncategorized.")
        perms, private = await self.overwrites(guild, record)
        if private:
            plan['notes'].append(f"{record['name']}: incomplete permissions; restored private for owner review.")
        common = dict(overwrites=perms, reason=f"GRZ rebuild {plan['code']} source={record['old_id']}")
        if record.get('position') is not None:
            common['position'] = max(0, int(record['position']))
        kind = record['kind']
        if kind == 4:
            return await guild.create_category(record['name'], **common)
        common['category'] = parent
        if kind in (0, 5, 15, 16):
            for key in ('topic', 'nsfw', 'slowmode_delay', 'default_thread_slowmode_delay'):
                if key in record and record[key] is not None:
                    common[key] = record[key]
            if kind in (0, 5):
                return await guild.create_text_channel(record['name'], news=kind == 5, **common)
            return await guild.create_forum(record['name'], media=kind == 16, **common)
        for key in ('bitrate', 'user_limit', 'nsfw'):
            if key in record and record[key] is not None:
                common[key] = record[key]
        if 'bitrate' in common:
            common['bitrate'] = max(8000, min(int(common['bitrate']), int(guild.bitrate_limit)))
        method = guild.create_stage_channel if kind == 13 else guild.create_voice_channel
        return await method(record['name'], **common)

    async def run(self, guild, plan):
        if not plan.get('approved') or plan['guild_id'] != guild.id or plan['owner_id'] != guild.owner_id:
            raise ValueError('Plan is not approved for this server and owner.')
        if not plan['old']:
            raise ValueError('No recoverable channels. Refusing deletion.')
        me = guild.me
        for permission in ('manage_channels', 'view_audit_log', 'view_channel', 'send_messages', 'attach_files'):
            if not me or not getattr(me.guild_permissions, permission):
                raise ValueError(f'Bot needs {permission} before rebuilding. Nothing further was deleted.')
        live = {c.id:c for c in await guild.fetch_channels()}
        # Discord will reject deletion of required Community channels. Stop before wiping anything.
        if 'COMMUNITY' in guild.features:
            protected = [getattr(guild, name, None) for name in ('rules_channel', 'public_updates_channel')]
            if any(c and c.id in plan['current_ids'] and c.id in live for c in protected):
                raise ValueError('Disable Community in server settings before deleting its required channels, then resume this plan.')
        control = live.get(plan.get('control_id'))
        if control is None:
            control = await guild.create_text_channel('recovery-progress', overwrites=self.private_overwrites(guild), reason=f"GRZ rebuild {plan['code']} control")
            plan['control_id'] = control.id
            self.store.save(plan)
        if plan['phase'] in ('prepared', 'deleting'):
            plan['phase'] = 'deleting'
            self.store.save(plan)
            targets = [live[cid] for cid in plan['current_ids'] if cid in live]
            targets.sort(key=lambda c: isinstance(c, discord.CategoryChannel))
            for channel in targets:
                try:
                    await channel.delete(reason=f"Owner-confirmed GRZ rebuild {plan['code']}; roles preserved")
                except discord.NotFound:
                    pass
                except discord.HTTPException:
                    self.log.exception('Stopped rebuild: cannot delete channel %s. Resume after fixing permissions.', channel.id)
                    raise ValueError(f"Deletion stopped at channel {channel.id}. Fix permissions and use /rebuild_resume plan_id:{plan['code']}.")
                self.log.info('Rebuild deleted current channel %s', channel.id)
                await asyncio.sleep(0.5)
            plan['phase'] = 'restoring'
            self.store.save(plan)
        live = {c.id:c for c in await guild.fetch_channels()}
        # Reconcile a successful creation whose journal write was interrupted.
        async for entry in guild.audit_logs(limit=1000, action=discord.AuditLogAction.channel_create, after=datetime.fromisoformat(plan['created_at'])):
            prefix = f"GRZ rebuild {plan['code']} source="
            if entry.reason and entry.reason.startswith(prefix) and entry.user_id == me.id and entry.target and entry.target.id in live:
                old_id = entry.reason[len(prefix):]
                if old_id.isdigit():
                    plan['restored'][old_id] = entry.target.id
        self.store.save(plan)
        failed = []
        category_ids = {r['old_id'] for r in plan['old'] if r['kind'] == 4}
        for record in plan['old']:
            existing_id = plan['restored'].get(str(record['old_id']))
            if existing_id in live:
                continue
            parent_id = record.get('parent_id')
            if parent_id in category_ids and plan['restored'].get(str(parent_id)) not in live:
                failed.append(record['old_id'])
                self.log.error('Waiting to restore %s until its category is restored.', record['name'])
                continue
            try:
                channel = await self.create(guild, record, plan, live)
            except discord.HTTPException as exc:
                failed.append(record['old_id'])
                self.log.error('Could not restore %s: %s', record['name'], exc)
                continue
            plan['restored'][str(record['old_id'])] = channel.id
            live[channel.id] = channel
            self.store.save(plan)
            self.log.info('Restored %s: old=%s new=%s', record['name'], record['old_id'], channel.id)
            await asyncio.sleep(0.5)
        plan['failed'] = failed
        plan['phase'] = 'restoring' if failed else 'complete'
        self.store.save(plan)
        self.log.warning('REBUILD RESULT: restored=%s failed=%s plan=%s. Roles untouched.', len(plan['restored']), len(failed), plan['code'])
        for note in dict.fromkeys(plan['notes']):
            self.log.warning('Recovery note: %s', note)
        if not failed:
            try:
                await control.delete(reason='GRZ rebuild completed; removing temporary control channel')
            except discord.HTTPException:
                self.log.warning('Remove the temporary recovery-progress channel manually.')
        return len(plan['restored']), failed
