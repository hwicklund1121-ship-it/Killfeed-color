"""Build a DayZ 1.30 gun-lift drop graph from the owner's verified native export.

Builds authored drop/roll routes and a matched native-pose look-angle blend.
Native graph data is supplied locally; no animation clips are packaged.
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


ROLL_ROUTES = [
 ('{A619000000000001}', 'Throwing', 'PneOnBackRoll', 'IsCommand(CMD_Roll) && Stance == 2 && ThrowingOnBack && !IsTag("TagThrowingIn") && !IsTag("TagThrowingOut") && !IsTag("TagThrowingCancel")', '0.1'),
 ('{A619000000000002}', 'Throwing', 'ProneRoll', 'IsCommand(CMD_Roll) && Stance == 2 && !ThrowingOnBack && !IsTag("TagThrowingIn") && !IsTag("TagThrowingOut") && !IsTag("TagThrowingCancel")', '0.1'),
 ('{A619000000000003}', 'ProneRoll', 'IdleLow', 'Stance == 0 && !Raised && GetLowerTime() > 0.12', '0.16'),
 ('{A619000000000004}', 'ProneRoll', 'IdleRas', 'Stance == 0 && Raised && GetLowerTime() > 0.12', '0.16'),
 ('{A619000000000005}', 'PneOnBackRoll', 'IdleLow', 'Stance == 0 && !Raised && GetLowerTime() > 0.12', '0.16'),
 ('{A619000000000006}', 'PneOnBackRoll', 'IdleRas', 'Stance == 0 && Raised && GetLowerTime() > 0.12', '0.16'),
]

def roll_route_text():
    result = []
    for guid, origin, target, condition, duration in ROLL_ROUTES:
        result.append('AnimSrcNodeTransition ' + json.dumps(guid) + ' {\n'
                      '       FromState ' + json.dumps(origin) + '\n'
                      '       ToState ' + json.dumps(target) + '\n'
                      '       Duration ' + json.dumps(duration) + '\n'
                      '       StartTime ""\n'
                      '       Condition ' + json.dumps(condition) + '\n'
                      '       BlendFn S\n       MotionVecBlend 0x33 0\n      }\n      ')
    return ''.join(result)


DIRECTION_BODY = 'AnimSrcNodePose2 GRZDropDirectionalPose {\n     Source "Locomotion.PneRas.AimToBack"\n     ValueX "AimX"\n     ValueY "AimY"\n     Table "Y(-45): X(-180) = 0, X(-90) = 10, X(0) = 20, X(90) = 30, X(180) = 40"\\\n     "Y(0): X(-180) = 41, X(-90) = 51, X(0) = 61, X(90) = 71, X(180) = 81"\\\n     "Y(90): X(-180) = 82, X(-90) = 92, X(0) = 102, X(90) = 112, X(180) = 122"\n    }'
DIRECTION_STRENGTH = 'AnimSrcNodeBlendN ThrowStrenghPoBB {\n     BlendWeight "AimX"\n     BlendFn S\n     Thresholds {\n      -180 0 180\n     }\n     Children {\n      "ThrowStrengthPoBP" "ThrowStrengthPneP" "ThrowStrengthPoBP"\n     }\n     IsCyclic 0\n    }'

def patch_direction(source):
    a, b = named_block(source, 'AnimSrcNodeStateMachine', 'ThrowIdleStanceSTM')
    machine = source[a:b]
    for state, before in [('Pne', 'ThrowPneIdleG'), ('PoB', 'PoBAimB')]:
        c, d = named_block(machine, 'AnimSrcNodeState', state)
        block = change_property(machine[c:d], 'Child', before, 'GRZDropDirectionalPose')
        machine = machine[:c] + block + machine[d:]
    result = source[:a] + machine + source[b:]
    a, b = named_block(result, 'AnimSrcNodeBlendT', 'ThrowStrenghPoBB')
    # Both layers use the same native look angle and interpolation thresholds.
    result = result[:a] + DIRECTION_STRENGTH + result[b:]
    a, b = named_block(result, 'AnimSrcNodeBlendT', 'ThrowStrenghB')
    result = result[:a] + change_property(result[a:b], 'BlendTime', '0.3', '0.14') + result[b:]
    a, _ = named_block(result, 'AnimSrcNodeBlend', 'ThrowAimB')
    return result[:a] + DIRECTION_BODY + '\n    ' + result[a:]


def patch_locomotion(source):
    start, end = named_block(source, 'AnimSrcNodeStateMachine', 'ThrowIdleStanceSTM')
    machine = source[start:end]
    changes = []
    for guid, origin, target, destination, condition, blend in [
        ('{6930A8DE26987051}', 'Erc', 'ErcToPne', 'Pne', 'Stance == 2', '0.14'),
        ('{6930A8DE26987067}', 'Cro', 'CroToPne', 'Pne', 'Stance == 2', '0.14'),
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
    result = source[:start] + machine + source[end:]
    a, b = named_block(result, 'AnimSrcNodeStateMachine', 'StanceMaster')
    master = result[a:b]
    anchor, _ = named_block(master, 'AnimSrcNodeTransition', '{6930A8DE26987785}')
    master = master[:anchor] + roll_route_text() + master[anchor:]
    result = patch_direction(result[:a] + master + result[b:])
    a,b = named_block(result, 'AnimSrcNodeBlend', 'ThrowStrengthIdleB')
    result = result[:a] + change_property(result[a:b], 'BlendWeight', '1.0', 'clamp(ActionProgress * 1000, 0, 1)') + result[b:]
    return result, changes


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
    new_resource = '{E5500310E0494CA1}' + PREFIX + '/locomotion.agf'
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
        Version='0.3.1', TargetDayZ=TARGET_VERSION,
        SourceHashes={p: hashlib.sha256(b).hexdigest() for p, b in files.items()},
        Changes=changes,
        RollRoutes=[dict(Transition=g, From=f, To=t, Condition=c, BlendSeconds=float(d)) for g,f,t,c,d in ROLL_ROUTES],
        DirectionBlend=dict(BodySource='Locomotion.PneRas.AimToBack', BodyValueX='AimX', BodyValueY='AimY', BodyNode='GRZDropDirectionalPose', WeaponNode='ThrowStrenghPoBB', StanceWeaponBlendSeconds=0.14, TargetMapping='look right -> left ground side; look left -> right ground side; anatomy requires native verification'),
        PoseFix=dict(Baseline='0.2.1', ChangedNode='ThrowStrengthIdleB', PreparedWeight=0, StandRollTiming='unchanged', ClipRates='unchanged'),
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
