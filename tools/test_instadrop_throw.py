"""Check real guard code and native hook contracts without claiming Enforce compilation."""
from pathlib import Path
import re,subprocess,tempfile
from repair_hexgrid import read_pbo
from zipfile import ZipFile
ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'GRZ_InstaDrop/scripts/4_World/GRZInstaDropThrowReady.c').read_text()
# The bool-only Enforce helper is also valid C++; execute that exact function.
helper=s[s.index('class GRZInstaDropThrowPolicy'):s.index('modded class PlayerBase')].replace(' static bool',' public: static bool')
checks='''
// Direct keys must work even when the combined controller flag is false.
assert(P::HasStanceInput(false,false,true,false,false));
assert(P::HasStanceInput(false,false,false,true,false));
assert(P::HasStanceInput(false,false,false,false,true));
assert(P::HasStanceInput(false,true,false,false,false));
assert(P::HasStanceInput(true,false,false,false,false));
assert(!P::HasStanceInput(false,false,false,false,false));
assert(P::RecentInput(1250,1250));
assert(!P::RecentInput(1251,1250));
assert(!P::RecentInput(0,-1));

// Ready, charged, and native-mode-before-script-state cases.
assert(P::ShouldCancel(true,true,true,true,false,true,false));
assert(P::ShouldCancel(true,true,true,true,true,true,false));
assert(P::ShouldCancel(true,true,true,false,false,true,false));
// Release, passive probes, non-firearms and non-movement commands must survive.
assert(!P::ShouldCancel(true,true,true,true,true,true,true));
assert(!P::ShouldCancel(false,true,true,true,true,true,false));
assert(!P::ShouldCancel(true,false,true,true,true,true,false));
assert(!P::ShouldCancel(true,true,false,true,true,true,false));
assert(!P::ShouldCancel(true,true,true,false,false,false,false));
'''
with tempfile.TemporaryDirectory(prefix='grz-throw-guard-') as t:
 p=Path(t);(p/'test.cpp').write_text('#include <cassert>\n'+helper+'\nint main(){using P=GRZInstaDropThrowPolicy;\n'+checks+'\n}')
 subprocess.run(['g++','-std=c++11',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
assert 'throwing.CancelThrowing(controls, weapons);' in s
assert 'bool allowed = super.CanChangeStance(previousStance, newStance);' in s
assert s.count('super.CanChangeStance(')==1 and 'return allowed;' in s
assert 'return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);' in s
assert s.index('if (!GRZInstaDropThrowPolicy.ShouldCancel')<s.index('throwing.CancelThrowing')
for name in ['UAStand','UACrouch','UAProne','UAStance']:assert 'SyncedPress("'+name+'")' in s
assert 'm_GRZStanceInputUntil = now + 250' in s
assert 'v0.1.3 cancelled firearm throw preparation' in s
assert 'nativeAllowed=' in s
assert 'ForceStance(' not in s and 'ForceStanceUp(' not in s and 'SetPosition(' not in s
with ZipFile(ROOT/'downloads/GRZ-InstaDrop-Prototype.zip') as z:
 _,files=read_pbo(z.read('GRZ_InstaDrop.pbo'))
 assert files['scripts/4_World/GRZInstaDropThrowReady.c']==s.encode()
print('Actual throw-cancel guard vectors and both native-super hook contracts passed; packaged source matches. DayZ compilation and input/callback timing untested.')

# Compatibility branch must return before the optional native rate tuning.
runtime=(ROOT/'GRZ_InstaDrop/scripts/4_World/GRZInstaDrop.c').read_text()
assert 'bool OverrideTransitionSpeeds = false;' in runtime
branch=runtime.split('if (!settings.OverrideTransitionSpeeds)',1)[1].split('if (settings.Drop.Count()',1)[0]
assert 'Ready = true;' in branch and 'return;' in branch
assert 'ApplyClips(' not in branch and 'DebugOverrideAnimationSpeed' not in branch
import json
with ZipFile(ROOT/'downloads/GRZ-InstaDrop-Prototype.zip') as z:
 _,files=read_pbo(z.read('GRZ_InstaDropData.pbo'))
 assert json.loads(files['transitions.json'])['OverrideTransitionSpeeds'] is False
 assert 'OverrideAnimation' not in s and 'OverrideMovementSpeed(' not in s
print('Default package is input-only; rate tuning is behind an explicit opt-in, and native movement overrides are absent from the input fix.')
