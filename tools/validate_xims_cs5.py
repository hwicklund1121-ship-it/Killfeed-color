"""Regression check for the CS5 parent hidden-selection mismatch."""
import hashlib,json,sys
from pathlib import Path
from zipfile import ZipFile
from repair_hexgrid import read_pbo
r=Path(__file__).resolve().parents[1];src=r/'GRZ_XimsCS5'
pbo=(r/'downloads/GRZ_XimsCS5.pbo').read_bytes();ext,files=read_pbo(pbo)
assert ext['prefix']=='GRZ_XimsCS5' and hashlib.sha1(pbo[:-21]).digest()==pbo[-20:]
for name,data in files.items():assert (src/name).read_bytes()==data
cfg=files['config.cpp'].decode();assert 'displayName = "XIMS CS5"' in cfg and 'descriptionShort = "Made By Blake"' in cfg
assert 'hiddenSelections[] = {};' in cfg and 'model = "GRZ_XimsCS5\\data\\cs5_xims.p3d";' in cfg
model=files['data/cs5_xims.p3d'];assert model[:4]==b'ODOL'
report=json.loads((r/'downloads/XIMS-CS5-MANIFEST.json').read_text());restored=model
for part in ['cs5_receiver','cs5_stock','cs5_barrel','suppressor']:
 old=('uavweapons\\cs5\\cs5\\data\\'+part+'_co.paa').encode();new=('GRZ_XimsCS5\\data\\reskin_'+part+'_co.paa').encode()
 assert len(old)==len(new) and new in model and old not in model
 assert 'data/reskin_'+part+'_co.paa' in files
 restored=restored.replace(new,old)
assert hashlib.sha256(restored).hexdigest()==report['model_source_sha256']
assert hashlib.sha256(model).hexdigest()==report['model_painted_sha256']
assert len(files)==6 # config + four painted textures + private model
for name in ['XIMS-CS5.zip','GRZ-Arena-Menu-Update.zip']:
 with ZipFile(r/'downloads'/name) as z:assert z.read('GRZ_XimsCS5.pbo')==pbo
print('CS5 corrected direct texture bindings, exact original binary recovery, asset paths, metadata and deliveries passed. In-game appearance remains untested.')
