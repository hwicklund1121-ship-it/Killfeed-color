"""GRZ logging bot: read-only access to local DayZ logs, send-only Discord access."""
from __future__ import annotations
import asyncio
import json
import logging
import os
from pathlib import Path
import sys
import time
import discord
from discord import app_commands
from dotenv import load_dotenv
from engine import CHANNELS, Store, match_channel

ROOT = Path(__file__).resolve().parent
load_dotenv(ROOT / '.env')
LOG = logging.getLogger('grz.logging')

def safe(value, length=1000):
    return discord.utils.escape_markdown(discord.utils.escape_mentions(str(value)))[:length] or '—'

def embed_for(event):
    colour = 0xE6A23C if event['Kind'].startswith('gameplay_') else 0xEEEEEE
    embed = discord.Embed(title=safe(event['Kind'].replace('_', ' ').title(), 200), description=safe(event.get('Details', ''), 3000), color=colour)
    for prefix, label in (('Player', 'Player'), ('Other', 'Other player')):
        name = event.get(prefix + 'Name')
        uid = event.get(prefix + 'Id')
        if name or uid:
            embed.add_field(name=label, value=safe(name or 'Unknown', 200) + '\n' + safe(uid or 'No Steam ID', 80))
    embed.set_footer(text=str(event.get('Timestamp', ''))[:80] + ' | ' + str(event['Id'])[:200])
    return embed

class LoggingBot(discord.Client):
    def __init__(self, server_id, directory, overrides, state):
        intents = discord.Intents.none()
        intents.guilds = True
        super().__init__(intents=intents, allowed_mentions=discord.AllowedMentions.none())
        self.tree = app_commands.CommandTree(self)
        self.server_id = server_id
        self.directory = directory
        self.overrides = overrides
        self.store = Store(state)
        self.worker = None
        self.lock_file = None
        self.tree.add_command(app_commands.Command(name='grz_logs_status', description='Check GRZ logging queue and channel setup', callback=self.status), guild=discord.Object(id=server_id))

    async def setup_hook(self):
        # A separate local lock prevents two copies racing to send the same queue.
        self.lock_file = open(ROOT / 'logging-bot.lock', 'a+b')
        try:
            if os.name == 'nt':
                import msvcrt
                self.lock_file.seek(0)
                self.lock_file.write(b'0'); self.lock_file.flush(); self.lock_file.seek(0)
                msvcrt.locking(self.lock_file.fileno(), msvcrt.LK_NBLCK, 1)
            else:
                import fcntl
                fcntl.flock(self.lock_file, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError:
            raise RuntimeError('Another GRZ logging bot is already running from this folder.') from None
        try:
            await self.tree.sync(guild=discord.Object(id=self.server_id))
        except discord.HTTPException:
            LOG.warning('Could not register /grz_logs_status; check applications.commands invite scope.')
        self.worker = asyncio.create_task(self.work())

    def channels(self):
        guild = self.get_guild(self.server_id)
        if not guild:
            return {}
        return {key: match_channel(key, guild.text_channels, self.overrides.get(key)) for key in CHANNELS}

    @app_commands.default_permissions(manage_guild=True)
    async def status(self, interaction: discord.Interaction):
        if interaction.guild_id != self.server_id or not interaction.permissions.manage_guild:
            await interaction.response.send_message('Manage Server permission is required.', ephemeral=True)
            return
        mapping = self.channels()
        missing = [key for key in CHANNELS if not mapping.get(key)]
        summary = self.store.stats()
        await interaction.response.send_message(
            f"Pending: {summary['pending']} | Delivered: {summary['delivered']} | Rejected records: {summary['rejected']}\n"
            + ('Missing/ambiguous channels: ' + ', '.join(missing) if missing else f'All {len(CHANNELS)} channels matched.')
            + '\nAudit directory: ' + ('available' if self.directory.is_dir() else 'NOT FOUND'), ephemeral=True)

    async def on_ready(self):
        LOG.info('Online as %s. Audit directory: %s', self.user, self.directory)
        missing = [key for key in CHANNELS if not self.channels().get(key)]
        if missing:
            LOG.warning('Missing/ambiguous channels (logs remain queued): %s', ', '.join(missing))
        if not self.directory.is_dir():
            LOG.warning('Audit directory not found yet. Start updated DayZ server or fix AUDIT_DIRECTORY.')

    async def work(self):
        await self.wait_until_ready()
        last_error = 0.0
        while not self.is_closed():
            try:
                # Short bounded file batches; SQLite access stays on this event loop.
                self.store.ingest(self.directory)
                mapping = self.channels()
                for item in self.store.pending():
                    channel = mapping.get(item['channel'])
                    if channel is None:
                        self.store.fail(item, 'Channel missing/ambiguous; set channel-overrides.json')
                        continue
                    try:
                        await channel.send(embed=embed_for(json.loads(item['payload'])), allowed_mentions=discord.AllowedMentions.none())
                    except discord.HTTPException as exc:
                        self.store.fail(item, 'Discord HTTP ' + str(exc.status))
                        continue
                    self.store.success(item)
            except Exception:
                if time.time() - last_error > 60:
                    LOG.exception('Logging worker problem; retrying without discarding queued logs')
                    last_error = time.time()
            await asyncio.sleep(1)

    async def close(self):
        if self.worker:
            self.worker.cancel()
            try:
                await self.worker
            except asyncio.CancelledError:
                pass
        self.store.close()
        if self.lock_file:
            self.lock_file.close()
        await super().close()

def main():
    logging.basicConfig(level=logging.INFO, format='%(asctime)s %(levelname)s %(message)s')
    token = os.getenv('DISCORD_BOT_TOKEN', '').strip()
    server = os.getenv('SERVER_ID', '').strip()
    directory = os.getenv('AUDIT_DIRECTORY', '').strip().strip('"')
    if not token or not server.isdecimal() or not directory:
        sys.exit('Fill DISCORD_BOT_TOKEN, numeric SERVER_ID and AUDIT_DIRECTORY in .env. Never post your token in chat.')
    overrides_path = ROOT / 'channel-overrides.json'
    overrides = json.loads(overrides_path.read_text(encoding='utf-8')) if overrides_path.exists() else {}
    if not isinstance(overrides, dict) or any(k not in CHANNELS or not str(v).isdecimal() for k, v in overrides.items()):
        sys.exit('channel-overrides.json must map known channel names to numeric channel IDs.')
    bot = LoggingBot(int(server), Path(directory), overrides, ROOT / 'logging_data' / 'queue.sqlite3')
    try:
        bot.run(token, log_handler=None)
    except discord.LoginFailure:
        sys.exit('Discord rejected the bot token. Update your local .env.')
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
