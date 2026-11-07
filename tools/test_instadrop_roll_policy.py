"""Exercise the actual Enforce pure-policy source using a host C++ compiler.

This does not compile the native PlayerBase wrapper or run DayZ.
"""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'GRZ_InstaDrop/scripts/4_World/GRZInstaDropRoll.c').read_text()
stand_source = (root / 'GRZ_InstaDrop/scripts/4_World/GRZInstaDropThrowReady.c').read_text()
stand_policy = stand_source.split('modded class PlayerBase', 1)[0].replace('class GRZInstaDropThrowPolicy', 'struct GRZInstaDropThrowPolicy')
policy = source.split('modded class PlayerBase', 1)[0].replace('class GRZInstaDropRollPolicy', 'struct GRZInstaDropRollPolicy')
test = r'''
#include <cassert>
#include <array>
bool evaluate(std::array<bool,12> x) {
 return GRZInstaDropRollPolicy::AllowOnBack(x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],x[9],x[10],x[11]);
}
int main() {
 assert(!GRZInstaDropThrowPolicy::ShouldForceStand(false,true));
 assert(!GRZInstaDropThrowPolicy::ShouldForceStand(true,false));
 assert(GRZInstaDropThrowPolicy::ShouldForceStand(true,true));
 assert(GRZInstaDropThrowPolicy::RecentInput(100,750));
 assert(!GRZInstaDropThrowPolicy::RecentInput(751,750));
 const std::array<bool,12> eligible = {true,true,true,true,true,true,true,true,true,false,false,false};
 assert(evaluate(eligible));
 // Every eligibility prerequisite must independently reject the exception.
 for (int i=0;i<9;++i) { auto x=eligible; x[i]=false; assert(!evaluate(x)); }
 // Emotes, actual wind-up and release must independently reject it too.
 for (int i=9;i<12;++i) { auto x=eligible; x[i]=true; assert(!evaluate(x)); }
 // No combination of blocked actions can turn a forbidden context eligible.
 for (int i=0;i<9;++i) for (int mask=0;mask<8;++mask) {
  auto x=eligible; x[i]=false;
  for(int k=0;k<3;++k) x[k+9]=(mask&(1<<k))!=0;
  assert(!evaluate(x));
 }
}
'''
with tempfile.TemporaryDirectory() as t:
    p = Path(t)
    (p / 'roll.cpp').write_text(stand_policy + policy + test)
    subprocess.run(['c++', '-std=c++17', '-Wall', '-Werror', str(p / 'roll.cpp'), '-o', str(p / 'roll')], check=True)
    subprocess.run([str(p / 'roll')], check=True)
print('PASS: actual roll policy accepts eligible on-back preparation and rejects each required guard and blocked-action combination.')
