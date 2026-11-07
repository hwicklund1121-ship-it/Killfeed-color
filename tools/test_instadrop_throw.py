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
assert 'return super.CanChangeStance(previousStance, newStance);' in s
assert 'return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);' in s
assert s.index('if (!GRZInstaDropThrowPolicy.ShouldCancel')<s.index('throwing.CancelThrowing')
assert 'ForceStance(' not in s and 'ForceStanceUp(' not in s and 'SetPosition(' not in s
with ZipFile(ROOT/'downloads/GRZ-InstaDrop-Prototype.zip') as z:
 _,files=read_pbo(z.read('GRZ_InstaDrop.pbo'))
 assert files['scripts/4_World/GRZInstaDropThrowReady.c']==s.encode()
print('Actual throw-cancel guard vectors and both native-super hook contracts passed; packaged source matches. DayZ compilation and input/callback timing untested.')
