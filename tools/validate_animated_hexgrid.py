"""Regression checks for the Hexgrid config boot failure and packaged addon."""
import sys,hashlib,re
from pathlib import Path
from zipfile import ZipFile
from repair_hexgrid import read_pbo,section
from build_pbo import build
r=Path(__file__).resolve().parents[1]
s=r/'GRZ_AnimatedHexgrid';cfg=(s/'config.cpp').read_text()
for name in ['DamageSystem','GlobalHealth','Health']:
 assert not re.search(r'class\s+'+name+r'\s*:\s*'+name+r'\b',cfg),'Unresolved nested self-base: '+name
_,base=read_pbo((r/'downloads/HEXGRID.pbo').read_bytes())
original=base['config.cpp'].decode();a,_,_,b=section(original,'GlobalArmor');c,_,_,d=section(cfg,'GlobalArmor')
assert re.sub(r'\s+','',original[a:b])==re.sub(r'\s+','',cfg[c:d]),'Armour settings changed'
assert 'hitpoints = 10000;' in cfg and cfg.count('flat_black.rvmat')==6
pbo=(r/'downloads/GRZ_AnimatedHexgrid.pbo').read_bytes();assert build(s)==pbo
assert hashlib.sha1(pbo[:-21]).digest()==pbo[-20:]
for archive in ['GRZ-Animated-Hexgrid.zip','GRZ-Arena-Menu-Update.zip','GRZ-SFG-Native-Muzzle-Update.zip']:
 with ZipFile(r/'downloads'/archive) as z:assert z.read('GRZ_AnimatedHexgrid.pbo')==pbo
print('Nested inheritance regression, unchanged base armour, flat material stages and all packaged PBOs passed. DayZ startup remains untested.')
