"""Execute the addon's actual pure decision rules via the shared C++ syntax subset.
This checks numeric rules and source/SDK wiring, not the DayZ VM or multiplayer.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
r=Path(__file__).resolve().parents[1]
source=(r/'GRZ_Audit/scripts/4_World/GRZAlertRules.c').read_text()
# Only adapt the default access specifier; the function bodies are unchanged.
cpp=source.replace('class GRZAlertRules','struct GRZAlertRules')
tests=r'''
#include <cassert>
int main() {
 assert(!GRZAlertRules::ValidSample(499));
 assert(GRZAlertRules::ValidSample(500));
 assert(GRZAlertRules::ValidSample(2500));
 assert(!GRZAlertRules::ValidSample(2501));
 assert(!GRZAlertRules::FastMovement(16.0f,1000,16.0f));
 assert(GRZAlertRules::FastMovement(16.1f,1000,16.0f));
 assert(!GRZAlertRules::FastMovement(1000.0f,3000,16.0f));
 assert(!GRZAlertRules::FastMovement(7.0f,1000,16.0f));
 assert(!GRZAlertRules::FastCadence(6,5000,1000.0f,0.55f));
 assert(!GRZAlertRules::FastCadence(10,100,1000.0f,0.55f));
 assert(!GRZAlertRules::FastCadence(5,2000,1000.0f,0.55f));
 assert(GRZAlertRules::FastCadence(6,2500,1000.0f,0.55f));
 assert(!GRZAlertRules::FastCadence(6,2750,1000.0f,0.55f));
 assert(!GRZAlertRules::FastCadence(6,2500,0.0f,0.55f));
 assert(!GRZAlertRules::FastCadence(30,2500,100.0f,0.55f));
 assert(!GRZAlertRules::ExcessDamage(800.0f,100.0f,8.0f));
 assert(GRZAlertRules::ExcessDamage(801.0f,100.0f,8.0f));
 assert(!GRZAlertRules::ExcessDamage(9999.0f,0.0f,8.0f));
 assert(!GRZAlertRules::ExcessDamage(2000.0f,1000.0f,8.0f));
}
'''
compiler=shutil.which('g++')
assert compiler,'g++ required to execute the pure decision rules'
with tempfile.TemporaryDirectory() as temp:
 p=Path(temp);(p/'rules.cpp').write_text(cpp+tests)
 subprocess.run([compiler,'-std=c++17','-Wall','-Wextra',str(p/'rules.cpp'),'-o',str(p/'rules')],check=True)
 subprocess.run([str(p/'rules')],check=True)
alerts=(r/'GRZ_Audit/scripts/4_World/GRZGameplayAlerts.c').read_text()
assert alerts.index('!g_Game.IsServer()') < alerts.index('LoadFile(')
for contract in ['GetCommand_Vehicle()', 'GetCommand_Ladder()', 'player.IsFalling()',
                 'now < state.GraceUntil', 'Settings.ConsecutiveSpeedSamples',
                 'GetCurrentModeBurstSize(muzzle_index) > 1', 'interval != m_GRZShotInterval',
                 'm_GRZShotCount < 6', 'elapsed < 2000', 'state.DamageSamples < 2',
                 'Settings.CooldownSeconds * 1000', 'DamageType.FIRE_ARM']:
 assert contract in alerts,contract
for addon in ['GRZ_Loadout','GRZ_MapRotation']:
 for path in (r/addon/'scripts/4_World').rglob('*.c'):
  lines=path.read_text().splitlines()
  for n,line in enumerate(lines):
   if '.SetPosition(' in line and ('p.SetPosition' in line or 'player.SetPosition' in line):
    assert 'GRZGameplayAlerts.AllowTeleport' in lines[n-1],(path,line)
# The feature cannot change gameplay or accept fabricated client evidence.
assert 'OnRPC' not in alerts and 'SetPosition(' not in alerts and 'Kick(' not in alerts and 'Ban(' not in alerts
assert 'GetKeyState' not in alerts and 'OnKeyPress' not in alerts
mission=(r/'GRZ_Audit/scripts/5_Mission/GRZGameplayAlertsMission.c').read_text()
assert 'modded class MissionServer' in mission and 'Remove(GRZGameplayAlerts.Tick)' in mission
cfg=(r/'GRZ_Audit/config.cpp').read_text()
assert cfg.index('};\n   class missionScriptModule') > cfg.index('class worldScriptModule')
for p in (r/'GRZ_Audit').rglob('*'):
 if p.suffix in ('.c','.cpp'):
  text=p.read_text();assert text.count('{')==text.count('}'),p
print('19 boundary/normal/anomaly vectors executed from actual rules; server hooks, sampling guards and all GRZ player teleport exemptions verified. DayZ runtime untested.')
