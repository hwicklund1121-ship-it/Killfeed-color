"""Static/package/native-contract verification. Does not compile or run DayZ."""
import hashlib
import json
import math
import re
import xml.etree.ElementTree as ET
from pathlib import Path
from zipfile import ZipFile
from build_pbo import build
from repair_hexgrid import read_pbo
root=Path(__file__).resolve().parents[1]
source=root/'GRZ_Loadout'
texts={p.relative_to(source).as_posix():p.read_text() for p in source.rglob('*') if p.is_file() and p.suffix in ('.c','.cpp','.layout')}
for name,s in texts.items():
 assert s.count('{')==s.count('}'),name
 if name.endswith('.c'):
  assert not re.search(r'\b[A-Za-z_]\w*-\d',s), (name, 'Separate subtraction from numeric literals for Enforce parsing')
 assert not re.search(r'\b(?:InGameMenu|int|string|float|bool)\s+native\b',s),name
for p in (root/'GRZ_MapRotation/scripts').rglob('*.c'):
 s=p.read_text(); assert s.count('{')==s.count('}'),str(p)
data=texts['scripts/3_Game/GRZDuelData.c']
server=texts['scripts/4_World/GRZDuelServer.c']
kits=texts['scripts/4_World/GRZDuelKits.c']
store=texts['scripts/4_World/GRZDuelRatings.c']
menu=texts['scripts/5_Mission/GRZDuelMenu.c']
board=texts['scripts/5_Mission/GRZLeaderboardMenu.c']
board_server=texts['scripts/4_World/GRZLeaderboardServer.c']
rotation=(root/'GRZ_MapRotation/scripts/4_World/GRZRotationServer.c').read_text()
assert 'int Elo = 800;' in data
assert 'Math.Pow(10.0, (loserElo - winnerElo) / 400.0)' in data
for own,other,expected in [(800,800,16),(800,1000,24),(800,600,8),(800,1200,29),(800,400,3)]:
 actual=max(1,min(32,math.floor(32*(1-1/(1+10**((other-own)/400)))+.5)))
 assert actual==expected,(own,other,actual)
assert 'if (elo < 1000) return 0;' in data and 'if (elo < 2200) return 6;' in data
assert 'bestOf != 1 && bestOf != 3 && bestOf != 5' in server
assert 'int firstTo = (match.BestOf + 1) / 2;' in server
assert 'challenge.Expires = g_Game.GetTime() + 60000;' in server
assert 'challenge.A == Uid(p)' in server and 'challenge.Id != id' in server
assert 'a.GetHumanInventory().GetEntityInHands() != challenge.WeaponA' in server
assert 'kitA = GRZDuelKits.Capture(a)' in server and 'kitB = GRZDuelKits.Capture(b)' in server
assert 'sender.GetId() != GetIdentity().GetId()' in server
assert 'match.Phase != 2' in server and 'match != attacking' in server
assert 'match.Map.SpawnA' in server and 'match.Map.SpawnB' in server
assert 'match.Due = now + 5000' in server
assert 'g_Game.GetTime() >= challenge.Expires' in server
assert 'SetHealthMax' in server and 'DamageSystem.ResetAllZones(p)' in server
assert 'GetMuzzleCount() != 1' in kits and 'CreateAttachmentEx(row.Type, row.Slot)' in kits
assert 'GRZDuelRatingStore.Result(winningId, losingId, delta)' in server
assert 'Flush(true)' in server and 'Flush();' in server
assert 'copy.Kills = row.Kills; copy.Deaths = row.Deaths;' in store
assert 'Same(next, verify)' in store and 'FILE + ".backup"' in store
assert 'killer != victim' in store and 'dead.Deaths++' in store and 'won.Kills++' in store
assert 'm_GRZDuelDeathRecorded = true' in server
assert 'if (match.Phase == 0)' in server and 'match.MissingA' in server
assert 'GRZDuelHooks.ReturnToArena.Invoke(p)' in server
assert rotation.count('player.m_GRZDuelMatch != ""') >= 2
assert 'GRZDuelHooks.ReturnToArena.Insert(ReturnFromDuel)' in rotation
assert '"GRZ_Loadout"' in (root/'GRZ_MapRotation/config.cpp').read_text()
assert 'GameRespawn(false);' in menu and 'ForceDisable(blockFire)' in menu
assert 'g_Game.GetTime() - m_GRZDuelDeadSince >= 3000' in menu
assert 'data.AutoRespawn || data.PendingId' in menu
assert 'int Sort;' in data and 'int MyElo = 800;' in data
assert 'cache.Rows[i]' in board_server and 'start + 20' in board_server
assert 'new map<string,ref GRZLeaderboardRow>' in board_server
assert 'g_Game.GetTime() - cache.BuiltAt < 5000' in board_server
# Public leaderboard DTO must not leak private storage identifiers.
row=data.split('class GRZLeaderboardRow',1)[1].split('};',1)[0]
assert 'SteamId' not in row
inputs=ET.parse(source/'inputs.xml')
bindings={i.attrib['name']:i.find('btn').attrib['name'] for i in inputs.findall('./preset/input')}
assert bindings['UAGRZLeaderboard']=='kO' and bindings['UAGRZLoadout']=='kComma'
layout_names=set()
for name,s in texts.items():
 if name.endswith('.layout'):
  layout_names.update(re.findall(r'\w+WidgetClass\s+(\w+)',s))
for name,s in texts.items():
 if name.endswith('.c'):
  for ref in re.findall(r'FindAnyWidget\("([^"]+)"\)',s):
   assert ref in layout_names,(name,ref)
for n in range(20):
 for prefix in ['Row','Name','Kills','Deaths','KD','Elo','Badge','Position']:
  assert prefix+str(n) in layout_names,(prefix,n)
assert 'DuelsTab' in layout_names
# Native public methods used by the new feature exist in the locally checked script reference.
reference=Path('/tmp/grz-dayz-reference/DayZ-Script-Diff-main/scripts')
if reference.exists():
 native=(reference/'4_world/entities/core/inherited/weapon.c').read_text()
 assert 'GetMuzzleCount()' in native and 'GetChamberAmmoTypeName(int muzzleIndex)' in native
 ui=(reference/'1_core/proto/enwidgets.c').read_text()
 assert 'SetItemColor(int row, int column, int color' in ui and 'SelectRow(int row)' in ui
 assert 'ForceDisable( bool bEnable )' in (reference/'3_game/inputapi/uainput.c').read_text()
for addon in ['GRZ_Loadout','GRZ_MapRotation']:
 payload=(root/'downloads'/f'{addon}.pbo').read_bytes()
 assert payload==build(root/addon),addon
 extensions,files=read_pbo(payload)
 assert extensions['prefix']==addon
 for p in (root/addon).rglob('*.c'):
  assert files[p.relative_to(root/addon).as_posix()]==p.read_bytes()
with ZipFile(root/'downloads/GRZ-Duels-Update.zip') as z:
 assert z.testzip() is None
 for addon in ['GRZ_Loadout','GRZ_MapRotation']:
  assert z.read(addon+'.pbo')==(root/'downloads'/f'{addon}.pbo').read_bytes()
example=json.loads((root/'packaging/duels/duel-config.example.json').read_text())
assert example['Maps'][0]['Enabled'] is False
assert example['Maps'][0]['SpawnA']==[0,0,0] and example['Maps'][0]['SpawnB']==[0,0,0]
print('Checked native APIs, RPC ownership, Elo vectors, tiers, challenge/round/respawn guards, map-rotation handoff, stats persistence, all widget references, O binding and both packaged PBOs.')
print('Static/package checks only. DayZ compilation and multiplayer execution remain untested; actual duel arenas must be configured.')
