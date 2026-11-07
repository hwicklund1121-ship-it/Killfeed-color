import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from engine import CHANNELS, ROUTES, Store, match_channel, routes


def event(uid='boot-1', kind='kill', **fields):
    return dict(Schema=1, Id=uid, Kind=kind, PlayerName='Player', **fields)


class QueueTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.file = self.root / 'events-boot.jsonl'
        self.db = self.root / 'state.sqlite3'
        self.store = Store(self.db)

    def tearDown(self):
        self.store.close()
        self.temp.cleanup()

    def write(self, *events):
        self.file.write_bytes(b''.join(json.dumps(e).encode() + b'\n' for e in events))

    def test_all_channels_have_routes(self):
        actual = {channel for kind in ROUTES for channel in routes(event(kind=kind))}
        actual.update(routes(event(kind='weapon_change', Vip=True)))
        actual.update(routes(event(kind='clothing_change', Vip=True)))
        self.assertEqual(actual, set(CHANNELS))
        self.assertEqual(routes(event(kind='duel_loss')), ('duel-loses',))
        self.assertEqual(set(routes(event(kind='vip_prefix_change'))), {'vip-prefixs', 'vip-logs'})

    def test_partial_line_waits_then_finishes(self):
        raw = json.dumps(event()).encode()
        self.file.write_bytes(raw[:20])
        self.store.ingest(self.root)
        self.assertEqual(self.store.stats()['pending'], 0)
        with self.file.open('ab') as f:
            f.write(raw[20:] + b'\n')
        self.store.ingest(self.root)
        self.assertEqual(self.store.stats()['pending'], 1)
        self.store.ingest(self.root)
        self.assertEqual(self.store.stats()['pending'], 1)

    def test_restart_and_duplicate_sources(self):
        self.write(event())
        self.store.ingest(self.root)
        self.store.success(self.store.pending()[0])
        self.store.close()
        self.store = Store(self.db)
        (self.root / 'events-copy.jsonl').write_bytes(self.file.read_bytes())
        self.store.ingest(self.root)
        self.assertEqual(self.store.stats(), {'pending': 0, 'delivered': 1, 'rejected': 0})

    def test_rotation_truncation_and_replacement(self):
        self.write(event('old-event-long-name'))
        self.store.ingest(self.root)
        self.write(event('new'))
        self.store.ingest(self.root)
        # Same length replacement detected by checkpoint anchor.
        self.write(event('xyz'))
        self.store.ingest(self.root)
        (self.root / 'events-nextboot.jsonl').write_text(json.dumps(event('nextboot')) + '\n')
        self.store.ingest(self.root)
        self.assertEqual(self.store.stats()['pending'], 4)

    def test_bad_unknown_oversized_and_valid_lines(self):
        self.file.write_bytes(b'not json\n' + json.dumps(event(kind='unknown')).encode() + b'\n' + b'X' * 70000 + b'\n' + json.dumps(event()).encode() + b'\n')
        self.store.ingest(self.root)
        self.assertEqual(self.store.stats(), {'pending': 1, 'delivered': 0, 'rejected': 3})

    def test_queue_checkpoint_atomic_on_database_failure(self):
        self.write(event())
        self.store.db.execute("CREATE TRIGGER stop_queue BEFORE INSERT ON outbox BEGIN SELECT RAISE(ABORT, 'test failure'); END")
        with self.assertRaises(Exception):
            self.store.ingest_file(self.file)
        self.assertEqual(self.store.db.execute('SELECT count(*) FROM checkpoints').fetchone()[0], 0)
        self.store.db.execute('DROP TRIGGER stop_queue')
        self.store.ingest_file(self.file)
        self.assertEqual(self.store.stats()['pending'], 1)

    def test_retry_survives_restart_and_does_not_block_other_channel(self):
        self.write(event(kind='duel_invite'))
        self.store.ingest(self.root)
        first, second = self.store.pending()
        self.store.fail(first, 'Forbidden')
        self.store.success(second)
        self.store.close()
        self.store = Store(self.db)
        self.assertEqual(self.store.stats()['pending'], 1)
        self.assertEqual(self.store.pending(), [])
        self.store.db.execute('UPDATE outbox SET next_try=0')
        self.store.db.commit()
        retry = self.store.pending()[0]
        self.assertEqual(retry['attempts'], 1)
        self.assertEqual(retry['error'], 'Forbidden')

    def test_channel_matching_unique_and_overrides(self):
        a = SimpleNamespace(id=10, name='︱duel-logs')
        b = SimpleNamespace(id=20, name='・duel-logs')
        self.assertIs(match_channel('duel-logs', [a]), a)
        self.assertIsNone(match_channel('duel-logs', [a, b]))
        self.assertIs(match_channel('duel-logs', [a, b], '20'), b)
        self.assertIsNone(match_channel('duel-logs', [a], '20'))

if __name__ == '__main__':
    unittest.main()
