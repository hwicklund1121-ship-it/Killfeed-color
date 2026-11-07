"""Regression checks against the supplied native export; no DayZ runtime claim."""
import hashlib
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from zipfile import ZipFile
from build_instadrop_graph import BASE, named_block, patch_locomotion, build_graph
from build_instadrop_graph_release import ROOT, release
from build_pbo import build

EXPORT = Path(sys.argv.pop(1))


def unpack(data):
    if data[-21] != 0 or hashlib.sha1(data[:-21]).digest() != data[-20:]:
        raise ValueError('Invalid PBO checksum')
    offset = 0
    def string():
        nonlocal offset
        end = data.index(b'\0', offset)
        value = data[offset:end].decode()
        offset = end + 1
        return value
    entries = []
    prefix = None
    while True:
        name = string()
        method, original, reserved, timestamp, size = struct.unpack_from('<5I', data, offset)
        offset += 20
        if not name and method == 0x56657273:
            while True:
                key = string()
                if not key:
                    break
                value = string()
                if key == 'prefix':
                    prefix = value
            continue
        if not name:
            break
        if method != 0 or original != size:
            raise ValueError('Unexpected compression')
        entries.append((name.replace('\\', '/'), size))
    files = {}
    for name, size in entries:
        files[name] = data[offset:offset + size]
        offset += size
    if offset != len(data) - 21:
        raise ValueError('Invalid PBO table/body size')
    return prefix, files


class DropGraphTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with ZipFile(EXPORT) as z:
            cls.native = z.read('native/' + BASE + 'locomotion.agf').decode()
            cls.native_root = z.read('native/' + BASE + 'player_main.agr').decode()

    def test_exactly_two_edges_change_and_roll_graphs_preserved(self):
        patched, changes = patch_locomotion(self.native)
        self.assertEqual(len(changes), 2)
        restored = patched
        for row in changes:
            a, b = named_block(restored, 'AnimSrcNodeTransition', row['Transition'])
            edge = restored[a:b]
            self.assertIn('ToState "Pne"', edge)
            self.assertIn('Duration "0.03"', edge)
            self.assertIn('Condition "Stance == 2"', edge)
            edge = edge.replace('ToState "Pne"', 'ToState "' + row['Before'] + '"').replace('Duration "0.03"', 'Duration "0.2"')
            restored = restored[:a] + edge + restored[b:]
        self.assertEqual(restored, self.native)

    def test_already_patched_or_changed_native_edges_rejected(self):
        patched, _ = patch_locomotion(self.native)
        with self.assertRaises(ValueError):
            patch_locomotion(patched)
        changed = self.native.replace('FromState "Erc"\n       ToState "ErcToPne"', 'FromState "Unexpected"\n       ToState "ErcToPne"')
        with self.assertRaises(ValueError):
            patch_locomotion(changed)

    def test_source_hash_and_version_rejection(self):
        with tempfile.TemporaryDirectory() as t, ZipFile(EXPORT) as z:
            for field, value in [('GameVersion', 'future-version'), ('Schema', 999)]:
                report = json.loads(z.read('MANIFEST.json'))
                report[field] = value
                bad = Path(t) / (field + '.zip')
                with ZipFile(bad, 'w') as out:
                    for name in z.namelist():
                        out.writestr(name, json.dumps(report) if name == 'MANIFEST.json' else z.read(name))
                with self.assertRaises(ValueError):
                    build_graph(bad, Path(t) / 'addon')
            bad = Path(t) / 'changed.zip'
            with ZipFile(bad, 'w') as out:
                for name in z.namelist():
                    data = z.read(name)
                    out.writestr(name, data + b' ' if name.endswith('/locomotion.agf') else data)
            with self.assertRaisesRegex(ValueError, 'hash mismatch'):
                build_graph(bad, Path(t) / 'addon')

    def test_release_integrity_roll_source_and_no_clip_payloads(self):
        with tempfile.TemporaryDirectory() as t:
            dest = Path(t) / 'release.zip'
            release(EXPORT, dest)
            first = dest.read_bytes()
            release(EXPORT, dest)
            self.assertEqual(first, dest.read_bytes(), 'Release must be reproducible')
            with ZipFile(dest) as z:
                manifest = json.loads(z.read('MANIFEST.json'))
                all_files = {}
                for name, expected in manifest.items():
                    data = build(ROOT / 'GRZ_InstaDropGraph') if name == 'GRZ_InstaDropGraph.pbo' else z.read(name)
                    self.assertEqual(hashlib.sha256(data).hexdigest(), expected)
                    prefix, files = unpack(data)
                    self.assertEqual(prefix + '.pbo', name)
                    self.assertFalse(any(p.lower().endswith('.anm') for p in files))
                    all_files[prefix] = files
                graph = all_files['GRZ_InstaDropGraph']
                self.assertEqual(set(graph), {'config.cpp', 'locomotion.agf', 'player_main.agr'})
                self.assertEqual(graph['locomotion.agf'].decode(), patch_locomotion(self.native)[0])
                root = graph['player_main.agr'].decode()
                restored = root.replace('{4E5A645A1B134796}GRZ_InstaDropGraph/locomotion.agf', '{6CA2ACDCAC1184DC}DZ/anims/workspaces/player/player_main/Locomotion.agf')
                self.assertEqual(restored, self.native_root)
                source = all_files['GRZ_InstaDrop']['scripts/4_World/GRZInstaDropThrowReady.c']
                self.assertEqual(hashlib.sha256(source).digest(), bytes.fromhex('997674e13dc7f42f6bea4e4405d93b052326130b3f926f4e48adb3c9311a7a01'))
                settings = json.loads(all_files['GRZ_InstaDropData']['transitions.json'])
                self.assertFalse(settings['TuneDropInMovement'])
                self.assertNotIn('GRZ_Movement.pbo', z.namelist())
                self.assertNotIn('GRZ_InstaDropGraph.pbo', z.namelist())
                self.assertIn('Build-InstaDropGraph.ps1', z.namelist())
                self.assertFalse(any(n.lower().endswith(('.agr', '.agf', '.asi', '.anm')) for n in z.namelist()))


if __name__ == '__main__':
    unittest.main()
