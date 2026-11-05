"""Check supplied ammo values and packaged armour calculations, not DayZ physics."""
import re
from pathlib import Path
from zipfile import ZipFile

from hexgrid_balance import AMMO, HEALTH_MULTIPLIER, VEST_HITPOINTS, report
from repair_hexgrid import read_pbo, section

ROOT = Path(__file__).resolve().parents[1]


def body(text, name):
    start, _, _, end = section(text, name)
    return text[start:end]


def scalar(text, name):
    return float(re.search(r'\b' + name + r'\s*=\s*([^;]+);', text)[1])


for filename, classname in [('408', 'UAV_Bullet_408'), ('50cal', 'UAV_Bullet_50CAL'), ('vulcan', 'UAV_Bullet_VULCAN')]:
    source = (ROOT / 'reference/hexgrid-ammo' / (filename + '.cpp')).read_text()
    # Each reference file defines its projectile's DamageApplied section once.
    health = body(body(source, 'DamageApplied'), 'Health')
    assert scalar(health, 'damage') == AMMO[classname]['damage']
    effective = scalar(health, 'damage') * HEALTH_MULTIPLIER
    hits = AMMO[classname]['shots']
    assert 100 - effective * (hits - 1) > 0
    assert 100 - effective * hits <= 0

with ZipFile(ROOT / 'downloads/GRZ-Customs.zip') as archive:
    assert archive.testzip() is None
    _, base = read_pbo(archive.read('@GRZ_Customs/Addons/HEXGRID.pbo'))
    config = base['config.cpp'].decode()
    armour = body(config, 'GlobalArmor')
    assert 'GlobalHEXGRID' not in config
    assert scalar(body(body(armour, 'Projectile'), 'Health'), 'damage') == HEALTH_MULTIPLIER
    assert scalar(body(body(config, 'GlobalHealth'), 'Health'), 'hitpoints') == VEST_HITPOINTS
    assert scalar(body(body(armour, 'Projectile'), 'Shock'), 'damage') == 0.1
    _, custom = read_pbo(archive.read('@GRZ_Customs/Addons/GRZ_Customs.pbo'))
    assert 'class GRZ_GunRunnerZ_Hexgrid: HEXGRID_Base' in custom['config.cpp'].decode()
    custom_config = custom['config.cpp'].decode().replace('class GRZ_GunRunnerZ_Hexgrid: HEXGRID_Base', 'class GRZ_GunRunnerZ_Hexgrid')
    assert 'DamageSystem' not in body(custom_config, 'GRZ_GunRunnerZ_Hexgrid')
    assert not any(path.lower().endswith('.cpp') and b'class CfgAmmo' in data for path, data in base.items())
    assert base == read_pbo((ROOT / 'downloads/HEXGRID.pbo').read_bytes())[1]

report()
print('Supplied ammo values and packed armour match; calculated .408/.50/VULCAN lethal counts are 3/2/1 from 100 HP.')
print('Shared-base inheritance, native shock coefficient and package integrity checked. DayZ damage behaviour is not executed.')
