"""Regression checks against the supplied native export; no DayZ runtime claim."""
import hashlib
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from zipfile import ZipFile
from build_instadrop_graph import BASE, named_block, patch_locomotion, build_graph, roll_route_text, ROLL_ROUTES, DIRECTION_BODY, DIRECTION_STRENGTH
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

    def test_exactly_four_edges_change_and_roll_graphs_preserved(self):
        patched, changes = patch_locomotion(self.native)
        self.assertEqual(len(changes), 4)
        self.assertEqual(patched.count(roll_route_text()), 1)
        restored = patched.replace(roll_route_text(), "", 1)
        restored = restored.replace(DIRECTION_BODY + '\n    ', '', 1)
        a, b = named_block(self.native, 'AnimSrcNodeBlendT', 'ThrowStrenghPoBB')
        restored = restored.replace(DIRECTION_STRENGTH, self.native[a:b], 1)
        a, b = named_block(restored, 'AnimSrcNodeBlendT', 'ThrowStrenghB')
        restored = restored[:a] + restored[a:b].replace('BlendTime "0.14"', 'BlendTime "0.3"') + restored[b:]
        a,b = named_block(restored, 'AnimSrcNodeStateMachine', 'ThrowIdleStanceSTM')
        machine = restored[a:b]
        for state, before in [('Pne','ThrowPneIdleG'), ('PoB','PoBAimB')]:
            c,d = named_block(machine, 'AnimSrcNodeState', state)
            machine = machine[:c] + machine[c:d].replace('Child "GRZDropDirectionalPose"','Child "'+before+'"') + machine[d:]
        restored = restored[:a] + machine + restored[b:]

        for row in changes:
            a, b = named_block(restored, 'AnimSrcNodeTransition', row['Transition'])
            edge = restored[a:b]
            self.assertIn('ToState "' + row['After'] + '"', edge)
            self.assertIn('Duration "' + str(row['BlendSeconds']) + '"', edge)
            self.assertIn('Condition "Stance == ' + ('2' if row['After'] == 'Pne' else '0') + '"', edge)
            edge = edge.replace('ToState "' + row['After'] + '"', 'ToState "' + row['Before'] + '"').replace('Duration "' + str(row['BlendSeconds']) + '"', 'Duration "0.2"')
            restored = restored[:a] + edge + restored[b:]
        self.assertEqual(restored, self.native)

    def test_native_roll_routes_and_recovery_requirements(self):
        patched, _ = patch_locomotion(self.native)
        for guid, origin, target, condition, blend in ROLL_ROUTES:
            a, b = named_block(patched, 'AnimSrcNodeTransition', guid)
            edge = patched[a:b]
            self.assertIn('FromState "' + origin + '"', edge)
            self.assertIn('ToState "' + target + '"', edge)
            named_block(patched, 'AnimSrcNodeStateMachine', 'StanceMaster')
            if origin == 'Throwing':
                self.assertIn('IsCommand(CMD_Roll)', condition)
                self.assertIn('Stance == 2', condition)
                self.assertLess(a, named_block(patched, 'AnimSrcNodeTransition', '{6930A8DE26987785}')[0])
            else:
                self.assertIn('Stance == 0', condition)
                self.assertIn('GetLowerTime() > 0.12', condition)
            self.assertIn('AnimSrcNodeState ' + origin + ' {', patched)
            self.assertIn('AnimSrcNodeState ' + target + ' {', patched)

    def test_signed_native_pose_samples_keep_the_view_direction(self):
        patched, _ = patch_locomotion(self.native)
        a,b = named_block(patched,'AnimSrcNodePose2','GRZDropDirectionalPose')
        body = patched[a:b]
        self.assertIn('Source "Locomotion.PneRas.AimToBack"',body)
        self.assertIn('ValueX "AimX"',body)
        self.assertIn('ValueY "AimY"',body)
        # Opposite directions must use distinct recorded native pose columns.
        self.assertIn('X(-90) = 10',body)
        self.assertIn('X(90) = 30',body)
        self.assertIn('X(-90) = 51',body)
        self.assertIn('X(90) = 71',body)
        self.assertNotIn('"PoBAimB" "ThrowPneIdleG" "PoBAimB"',body)
        # Keep the native signed view/aim relationship, avoiding an aim inversion.
        a,b = named_block(self.native,'AnimSrcNodePose2','PneRasPose')
        native_pose=self.native[a:b]
        self.assertIn('ValueX "AimX"',native_pose)
        for column in ['X(-90) = 10','X(90) = 30','X(-90) = 51','X(90) = 71']:
            self.assertIn(column.replace(' ', ''),native_pose.replace(' ', ''))
        a,b = named_block(patched,'AnimSrcNodeBlendN','ThrowStrenghPoBB')
        self.assertIn('BlendWeight "AimX"',patched[a:b])
        a,b = named_block(patched,'AnimSrcNodeBlendT','ThrowStrenghB')
        self.assertIn('BlendTime "0.14"',patched[a:b])

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
                restored = root.replace('{E5500210E0494CA1}GRZ_InstaDropGraph/locomotion.agf', '{6CA2ACDCAC1184DC}DZ/anims/workspaces/player/player_main/Locomotion.agf')
                self.assertEqual(restored, self.native_root)
                source = all_files['GRZ_InstaDrop']['scripts/4_World/GRZInstaDropThrowReady.c']
                self.assertEqual(source, (ROOT / 'GRZ_InstaDrop/scripts/4_World/GRZInstaDropThrowReady.c').read_bytes())
                settings = json.loads(all_files['GRZ_InstaDropData']['transitions.json'])
                self.assertFalse(settings['TuneDropInMovement'])
                self.assertNotIn('GRZ_Movement.pbo', z.namelist())
                self.assertNotIn('GRZ_InstaDropGraph.pbo', z.namelist())
                self.assertIn('Build-InstaDropGraph.ps1', z.namelist())
                self.assertFalse(any(n.lower().endswith(('.agr', '.agf', '.asi', '.anm')) for n in z.namelist()))


if __name__ == '__main__':
    unittest.main()
