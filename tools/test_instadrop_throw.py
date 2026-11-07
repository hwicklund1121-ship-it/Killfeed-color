"""Exercise actual pure Enforce guard code as C++; check packaged native hooks.
This is not an Enforce compiler or a DayZ gameplay test.
"""
from pathlib import Path
from zipfile import ZipFile
import subprocess,tempfile,json
from repair_hexgrid import read_pbo
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'GRZ_InstaDrop/scripts/4_World/GRZInstaDropThrowReady.c').read_text()
helper=s[s.index('class GRZInstaDropThrowPolicy'):s.index('modded class PlayerBase')].replace(' static bool',' public: static bool')
checks='''
assert(P::HasStanceInput(false,false,false,false,true));
assert(P::HasStanceInput(false,false,true,false,false));
assert(!P::HasStanceInput(false,false,false,false,false));
assert(P::RecentInput(250,250));assert(!P::RecentInput(251,250));assert(!P::RecentInput(0,-1));
// Keep the lifted firearm when dropping; interrupt roll only for standing.
assert(P::AllowLegacy(true,true,true,true,true,false,true,false,false,false));
assert(P::AllowLegacy(true,true,true,true,false,true,false,true,false,false));
// Non-user queries, non-firearms, non-movement, or a blocked action cannot bypass.
assert(!P::AllowLegacy(false,true,true,true,true,false,true,false,false,false));
assert(!P::AllowLegacy(true,false,true,true,true,false,true,false,false,false));
assert(!P::AllowLegacy(true,true,false,true,true,false,true,false,false,false));
assert(!P::AllowLegacy(true,true,true,false,true,false,true,false,false,false));
// Charge and actual release remain locked; unrelated stance changes stay native.
assert(!P::AllowLegacy(true,true,true,true,true,false,true,false,true,false));
assert(!P::AllowLegacy(true,true,true,true,true,false,true,false,false,true));
assert(!P::AllowLegacy(true,true,true,true,true,true,false,false,false,false));
assert(!P::AllowLegacy(true,true,true,true,false,false,true,false,false,false));
'''
with tempfile.TemporaryDirectory(prefix='grz-legacy-stance-') as t:
 p=Path(t);(p/'test.cpp').write_text('#include <cassert>\n'+helper+'\nint main(){using P=GRZInstaDropThrowPolicy;'+checks+'}')
 subprocess.run(['g++','-std=c++11',str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
assert 'CancelThrowing(' not in s and 'SetThrowingMode(' not in s and 'ResetThrowingMode(' not in s
assert s.count('super.CanChangeStance(')==1 and 'return allowed;' in s
assert 'return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);' in s
assert 'PlayerCanChangeStance(this, DayZPlayerConstants.STANCEIDX_ERECT, true)' in s
assert s.index('PlayerCanChangeStance(this,')<s.index('movement.ForceStanceUp(')
assert 'movement.IsInRoll() || movement.IsOnBack()' in s
assert 'ForceStance(' not in s and 'SetPosition(' not in s
for name in ['UAStand','UACrouch','UAProne','UAStance']:assert 'SyncedPress("'+name+'")' in s
runtime=(ROOT/'GRZ_InstaDrop/scripts/4_World/GRZInstaDrop.c').read_text()
assert 'DebugOverrideAnimationSpeed(' not in runtime
assert 'ApplyClips(' not in runtime
# Keep the exact working roll/stance source from the user-confirmed release.
previous=subprocess.check_output(['git','show','6aaac54d5c163e10a384e91002c55f1ec351bc9d:GRZ_InstaDrop/scripts/4_World/GRZInstaDropThrowReady.c'],cwd=ROOT)
assert s.encode()==previous
with ZipFile(ROOT/'downloads/GRZ-InstaDrop-Prototype.zip') as z:
 _,f=read_pbo(z.read('GRZ_InstaDrop.pbo'));assert f['scripts/4_World/GRZInstaDropThrowReady.c']==s.encode()
 _,f=read_pbo(z.read('GRZ_InstaDropData.pbo'));data=json.loads(f['transitions.json'])
 assert data['OverrideTransitionSpeeds'] is False and data['TuneDropInMovement'] is True and data['DropSpeed']==4.0
 _,movement=read_pbo(z.read('GRZ_Movement.pbo'))
 boost=movement['scripts/4_World/GRZMovementDropBoost.c'].decode()
 assert 'settings.Drop)' in boost and 'settings.Recover' not in boost
 assert '_trans_pne_' in boost and '_transmove_pne_' in boost
 assert 'GRZMovementDropBoost.Apply();' in movement['scripts/4_World/GRZMovement.c'].decode()
 assert 'float Multiplier = 1.50;' in movement['scripts/4_World/GRZMovementGaitBoost.c'].decode()
print('Actual legacy permission guard cases, preserved lift mode, guarded native stand-up request, native-super hooks, single animation-rate owner, unchanged working roll source and packaged source passed. Enforce compilation, multiplayer and mid-roll graph interruption untested.')
