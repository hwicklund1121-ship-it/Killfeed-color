"""Build a DayZ 1.30 gun-lift drop graph from the owner's verified native export.

Does not use third-party graph data. Only four drop/stand edges of the
throw-ready stance machine are changed. Native clips remain in the game.
"""
import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re
from zipfile import ZipFile

BASE = 'dz/anims/workspaces/player/player_main/'
PREFIX = 'GRZ_InstaDropGraph'
TARGET_VERSION = '1.30.164.289'
TOKEN = re.compile(r'"(?:\\.|[^"\\])*"|[{}]|[^\s{}"]+')


def named_block(text, kind, name):
    tokens = list(TOKEN.finditer(text))
    hits = []
    for i in range(len(tokens) - 2):
        if tokens[i].group() == kind and tokens[i + 1].group().strip('"') == name and tokens[i + 2].group() == '{':
            depth = 1
            for j in range(i + 3, len(tokens)):
                value = tokens[j].group()
                depth += (value == '{') - (value == '}')
                if depth == 0:
                    hits.append((tokens[i].start(), tokens[j].end()))
                    break
            else:
                raise ValueError('Unclosed graph block: ' + name)
    if len(hits) != 1:
        raise ValueError(f'Expected one {kind} {name}, found {len(hits)}')
    return hits[0]


def change_property(block, key, before, after):
    pattern = re.compile(r'(?m)^(\s*' + re.escape(key) + r'\s+)"' + re.escape(before) + r'"(?=\s*$)')
    result, count = pattern.subn(lambda m: m[1] + '"' + after + '"', block)
    if count != 1:
        raise ValueError(f'Expected exactly one {key}={before!r}, found {count}')
    return result


def patch_locomotion(source):
    start, end = named_block(source, 'AnimSrcNodeStateMachine', 'ThrowIdleStanceSTM')
    machine = source[start:end]
    changes = []
    for guid, origin, target, destination, condition, blend in [
        ('{6930A8DE26987051}', 'Erc', 'ErcToPne', 'Pne', 'Stance == 2', '0.1'),
        ('{6930A8DE26987067}', 'Cro', 'CroToPne', 'Pne', 'Stance == 2', '0.1'),
        ('{6930A8DE26987073}', 'Pne', 'PneToErc', 'Erc', 'Stance == 0', '0.16'),
        ('{6930A8DE26987090}', 'PoB', 'PoBToErc', 'Erc', 'Stance == 0', '0.16'),
    ]:
        a, b = named_block(machine, 'AnimSrcNodeTransition', guid)
        edge = machine[a:b]
        for key, expected in [('FromState', origin), ('ToState', target), ('Duration', '0.2'), ('Condition', condition)]:
            if not re.search(r'(?m)^\s*' + key + r'\s+"' + re.escape(expected) + r'"\s*$', edge):
                raise ValueError(f'Unsupported {origin} downward transition: {key}')
        # Keep input conditions and physical native stance decisions unchanged.
        # Enter the requested native gun-lift idle directly, avoiding clip waits.
        patched = change_property(edge, 'ToState', target, destination)
        patched = change_property(patched, 'Duration', '0.2', blend)
        machine = machine[:a] + patched + machine[b:]
        changes.append(dict(Transition=guid, From=origin, Before=target, After=destination, BlendSeconds=float(blend)))
    return source[:start] + machine + source[end:], changes


def build_graph(export, output):
    with ZipFile(export) as z:
        if len(z.namelist()) != len(set(z.namelist())):
            raise ValueError('Duplicate archive members')
        report = json.loads(z.read('MANIFEST.json'))
        if report.get('Schema') != 1 or report.get('GameVersion') != TARGET_VERSION:
            raise ValueError('This patch targets the supplied DayZ ' + TARGET_VERSION + ' export only')
        files = {}
        for path in [BASE + 'player_main.agr', BASE + 'locomotion.agf']:
            rows = [r for r in report['Graphs'] if r['Path'] == path]
            if len(rows) != 1:
                raise ValueError('Missing or duplicate native graph: ' + path)
            data = z.read('native/' + path)
            if len(data) != rows[0]['Bytes'] or hashlib.sha256(data).hexdigest() != rows[0]['SHA256']:
                raise ValueError('Export hash mismatch: ' + path)
            files[path] = data
    source = files[BASE + 'locomotion.agf'].decode('utf-8')
    patched, changes = patch_locomotion(source)
    root = files[BASE + 'player_main.agr'].decode('utf-8')
    original_resource = '{6CA2ACDCAC1184DC}DZ/anims/workspaces/player/player_main/Locomotion.agf'
    new_resource = '{58DC0C845B1A47E2}' + PREFIX + '/locomotion.agf'
    if root.count(original_resource) != 1:
        raise ValueError('Native locomotion reference not uniquely located')
    root = root.replace(original_resource, new_resource)
    output.mkdir(parents=True, exist_ok=True)
    (output / '$PBOPREFIX$').write_text(PREFIX + '\n')
    (output / 'player_main.agr').write_text(root, encoding='utf-8')
    (output / 'locomotion.agf').write_text(patched, encoding='utf-8')
    (output / 'config.cpp').write_text('''class CfgPatches
{
 class GRZ_InstaDropGraph
 {
  units[] = {};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Characters", "GRZ_InstaDrop"};
 };
};
class CfgVehicles
{
 class Man;
 class SurvivorBase: Man
 {
  class enfAnimSys
  {
   graphName = "GRZ_InstaDropGraph/player_main.agr";
  };
 };
};
''')
    evidence = dict(
        Version='0.1.7', TargetDayZ=TARGET_VERSION,
        SourceHashes={p: hashlib.sha256(b).hexdigest() for p, b in files.items()},
        Changes=changes,
        LocomotionSHA256=hashlib.sha256(patched.encode()).hexdigest(),
        Limits='Source/package checks only; native graph loading and gameplay require DayZ testing.',
    )
    (output.parent / 'GRAPH-CHANGES.json').write_text(json.dumps(evidence, indent=2) + '\n')
    (output.parent / 'GRAPH-CHANGES.diff').write_text(''.join(difflib.unified_diff(source.splitlines(True), patched.splitlines(True), fromfile='vanilla/locomotion.agf', tofile='GRZ_InstaDropGraph/locomotion.agf')))
    return evidence


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--export', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    print(json.dumps(build_graph(args.export, args.output), indent=2))
