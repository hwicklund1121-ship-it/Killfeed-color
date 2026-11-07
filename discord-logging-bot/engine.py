"""Durable local DayZ audit tailer. No Discord dependency or credentials here."""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import re
import sqlite3
import time

CHANNELS = (
    'duel-logs', 'duel-invites', 'duel-accepts', 'duel-rejects', 'duel-wins',
    'duel-loses', 'weapon-changes', 'clothing-changes', 'killfeed', 'vip-logs',
    'vip-prefixs', 'vip-killfeed', 'vip-weapon-changes', 'vip-clothing-changes',
    'map-rotation-logs', 'map-votes', 'map-skips', 'leaderboard-logs', 'rank-ups',
)
ROUTES = {
    'duel_invite': ('duel-logs', 'duel-invites'),
    'duel_accept': ('duel-logs', 'duel-accepts'),
    'duel_reject': ('duel-logs', 'duel-rejects'),
    'duel_expire': ('duel-logs', 'duel-rejects'),
    'duel_finish': ('duel-logs',), 'duel_round': ('duel-logs',),
    'duel_start': ('duel-logs',), 'duel_win': ('duel-wins',),
    'duel_loss': ('duel-loses',), 'weapon_change': ('weapon-changes',),
    'clothing_change': ('clothing-changes',), 'kill': ('killfeed',),
    'vip_access': ('vip-logs',), 'vip_denied': ('vip-logs',),
    'vip_prefix_change': ('vip-logs', 'vip-prefixs'),
    'vip_killfeed_change': ('vip-logs', 'vip-killfeed'),
    'map_round_start': ('map-rotation-logs',),
    'map_vote_open': ('map-rotation-logs',), 'map_vote_close': ('map-rotation-logs',),
    'map_vote': ('map-votes',),
    'map_skip': ('map-rotation-logs', 'map-skips'),
    'map_skip_denied': ('map-rotation-logs', 'map-skips'),
    'leaderboard_view': ('leaderboard-logs',),
    'leaderboard_update': ('leaderboard-logs',),
    'leaderboard_stats': ('leaderboard-logs',),
    'rank_up': ('leaderboard-logs', 'rank-ups'),
}
MAX_LINE = 65536

def routes(event):
    result = list(ROUTES.get(event.get('Kind'), ()))
    if event.get('Vip') is True:
        extra = {'weapon_change': 'vip-weapon-changes', 'clothing_change': 'vip-clothing-changes'}.get(event.get('Kind'))
        if extra:
            result.extend(('vip-logs', extra))
    return tuple(dict.fromkeys(result))

def validate(event):
    if not isinstance(event, dict) or event.get('Schema') != 1:
        raise ValueError('Unsupported event schema')
    for key in ('Id', 'Kind'):
        if not isinstance(event.get(key), str) or not 0 < len(event[key]) <= 200:
            raise ValueError('Invalid ' + key)
    if not routes(event):
        raise ValueError('Unknown event kind: ' + event['Kind'])
    for key in ('Timestamp', 'PlayerId', 'PlayerName', 'OtherId', 'OtherName', 'Details'):
        if key in event and (not isinstance(event[key], str) or len(event[key]) > 16384):
            raise ValueError('Invalid ' + key)
    return event

def channel_key(name):
    # Discord names may start with a decorative divider or emoji.
    return re.sub(r'[^a-z0-9-]', '', name.lower()).lstrip('-')

def match_channel(key, channels, override=None):
    if override is not None:
        matches = [c for c in channels if c.id == int(override)]
    else:
        matches = [c for c in channels if channel_key(c.name) == key]
    if len(matches) != 1:
        return None  # Missing/ambiguous channels never get guessed.
    return matches[0]

class Store:
    def __init__(self, path):
        Path(path).parent.mkdir(parents=True, exist_ok=True)
        self.db = sqlite3.connect(path)
        self.db.row_factory = sqlite3.Row
        self.db.execute('PRAGMA journal_mode=WAL')
        self.db.execute('PRAGMA synchronous=FULL')
        self.db.executescript('''
        CREATE TABLE IF NOT EXISTS checkpoints(path TEXT PRIMARY KEY, offset INTEGER, anchor TEXT);
        CREATE TABLE IF NOT EXISTS outbox(
          id INTEGER PRIMARY KEY, event_id TEXT, channel TEXT, payload TEXT,
          attempts INTEGER DEFAULT 0, next_try REAL DEFAULT 0, delivered REAL,
          error TEXT, UNIQUE(event_id, channel));
        CREATE TABLE IF NOT EXISTS rejected(
          path TEXT, offset INTEGER, reason TEXT, sample TEXT,
          PRIMARY KEY(path, offset));
        ''')

    @staticmethod
    def anchor(file, offset):
        file.seek(max(0, offset - 128))
        return hashlib.sha256(file.read(min(offset, 128))).hexdigest()

    def ingest_file(self, path, limit=500):
        """Checkpoint and queue commit atomically; partial last lines wait for next poll."""
        path = Path(path).resolve()
        key = str(path)
        row = self.db.execute('SELECT * FROM checkpoints WHERE path=?', (key,)).fetchone()
        with path.open('rb') as file, self.db:
            offset = row['offset'] if row else 0
            if offset > path.stat().st_size or (row and self.anchor(file, offset) != row['anchor']):
                offset = 0
            file.seek(offset)
            count = 0
            while count < limit:
                start = file.tell()
                raw = file.readline(MAX_LINE + 1)
                if not raw:
                    break
                if len(raw) > MAX_LINE:
                    # Drain oversize complete records in bounded chunks.
                    while raw and not raw.endswith(b'\n'):
                        raw = file.readline(MAX_LINE + 1)
                    if not raw.endswith(b'\n'):
                        file.seek(start)
                        break
                    self.db.execute('INSERT OR REPLACE INTO rejected VALUES(?,?,?,?)', (key, start, 'Record exceeds 64 KiB', ''))
                elif not raw.endswith(b'\n'):
                    file.seek(start)
                    break
                else:
                    try:
                        event = validate(json.loads(raw))
                        payload = json.dumps(event, ensure_ascii=False)
                        for route in routes(event):
                            self.db.execute('INSERT OR IGNORE INTO outbox(event_id,channel,payload) VALUES(?,?,?)', (event['Id'], route, payload))
                    except (ValueError, UnicodeError) as exc:
                        self.db.execute('INSERT OR REPLACE INTO rejected VALUES(?,?,?,?)', (key, start, str(exc)[:300], raw[:500].decode('utf-8', errors='replace')))
                offset = file.tell()
                count += 1
            anchor = self.anchor(file, offset)
            self.db.execute('INSERT OR REPLACE INTO checkpoints VALUES(?,?,?)', (key, offset, anchor))
        return count

    def ingest(self, directory):
        # Old boot logs are retained; checkpoints make these inexpensive to revisit.
        total = 0
        for path in sorted(Path(directory).glob('events-*.jsonl')):
            try:
                total += self.ingest_file(path)
            except (FileNotFoundError, PermissionError):
                continue
        return total

    def pending(self, limit=25):
        return self.db.execute('SELECT * FROM outbox WHERE delivered IS NULL AND next_try<=? ORDER BY id LIMIT ?', (time.time(), limit)).fetchall()

    def success(self, item):
        with self.db:
            self.db.execute('UPDATE outbox SET delivered=?,error=NULL WHERE id=?', (time.time(), item['id']))

    def fail(self, item, reason):
        delay = min(300, 2 ** min(item['attempts'] + 1, 8))
        with self.db:
            self.db.execute('UPDATE outbox SET attempts=attempts+1,next_try=?,error=? WHERE id=?', (time.time() + delay, reason[:300], item['id']))

    def stats(self):
        return { 'pending': self.db.execute('SELECT count(*) FROM outbox WHERE delivered IS NULL').fetchone()[0],
                 'delivered': self.db.execute('SELECT count(*) FROM outbox WHERE delivered IS NOT NULL').fetchone()[0],
                 'rejected': self.db.execute('SELECT count(*) FROM rejected').fetchone()[0] }

    def close(self):
        self.db.close()
