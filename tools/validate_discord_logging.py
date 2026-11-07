"""Check authoritative audit hooks, routes and delivered package contents (no game runtime)."""
from pathlib import Path
import json
import re
import sys
from zipfile import ZipFile
from build_pbo import build
r=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(r/'discord-logging-bot'))
from engine import ROUTES, CHANNELS
modules=['GRZ_Audit','GRZ_Loadout','GRZ_MapRotation','GRZ_Killfeed']
kinds=set()
for addon in modules:
 for p in (r/addon/'scripts/4_World').rglob('*.c'):
  text=p.read_text()
  assert text.count('{')==text.count('}'),p
  kinds.update(re.findall(r'GRZAudit.Record\("([a-z_]+)"',text))
  kinds.update(re.findall(r'\.Kind = "([a-z_]+)"',text))
assert kinds == set(ROUTES),(kinds-set(ROUTES),set(ROUTES)-kinds)
assert len(CHANNELS)==19
for addon in modules[1:]:
 assert '"GRZ_Audit"' in (r/addon/'config.cpp').read_text()
audit=(r/'GRZ_Audit/scripts/4_World/GRZAudit.c').read_text()
assert audit.index('!g_Game.IsServer()') < audit.index('OpenFile(')
assert 'WriteToString(entry, false, encoded)' in audit and 'FileMode.APPEND' in audit
assert 'GetYearMonthDayUTC' in audit and 'GetHourMinuteSecondUTC' in audit
assert '$profile:GRZ_Logs/events-' in audit
assert not re.search(r'HTTP|RestApi|DISCORD_BOT_TOKEN',audit)
weapon=(r/'GRZ_Loadout/scripts/4_World/GRZLoadoutServer.c').read_text()
assert weapon.index('GRZAudit.Record("weapon_change"') > weapon.index('DeleteStaged(oldItems);')
clothes=(r/'GRZ_Loadout/scripts/4_World/GRZLoadoutClothingServer.c').read_text()
assert clothes.index('GRZAudit.Record("clothing_change"') > clothes.index('GRZLoadoutServer.DeleteStaged(old);')
ratings=(r/'GRZ_Loadout/scripts/4_World/GRZDuelRatings.c').read_text()
result=ratings.split('static bool Result(',1)[1]
assert result.index('if (!Commit(next)) return false;') < result.index('audit.Kind = "leaderboard_update"')
assert result.index('if (!Commit(next)) return false;') < result.index('promotion.Kind = "rank_up"')
with ZipFile(r/'downloads/GRZ-Arena-Menu-Update.zip') as z:
 manifest=json.loads(z.read('MANIFEST.json'))
 assert len(manifest['pbo'])==14
 for addon in modules:
  assert z.read(addon+'.pbo')==build(r/addon),addon
 assert z.read('DISCORD-LOGGING.md')==(r/'DISCORD-LOGGING.md').read_bytes()
with ZipFile(r/'downloads/GRZ-Discord-Logging-Bot.zip') as z:
 assert z.testzip() is None
 for name in z.namelist():
  rel=Path(name).relative_to('GRZ-Discord-Logging-Bot')
  assert '.env' not in rel.parts and 'logging_data' not in rel.parts and '.venv' not in rel.parts
  assert not name.endswith(('.sqlite3','.db','.lock'))
  assert z.read(name)==(r/'discord-logging-bot'/rel).read_bytes()
print('All 27 audit event kinds map to the 19 channels; server authority, post-commit hooks, 14-addon package and credential-free bot ZIP verified. DayZ runtime untested.')
