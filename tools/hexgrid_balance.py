"""Native armour configuration, calibrated against the supplied UAV ammo.

Calculation assumes 100 player health and direct covered torso hits. Native
range/hitzone effects and other clothing/mods still require DayZ validation.
"""
import math
import re

from repair_hexgrid import section

HEALTH_MULTIPLIER = 0.045
VEST_HITPOINTS = 10000
AMMO = {
    'UAV_Bullet_408': {'damage': 1000, 'shots': 3},
    'UAV_Bullet_50CAL': {'damage': 1300, 'shots': 2},
    'UAV_Bullet_VULCAN': {'damage': 5000, 'shots': 1},
}


def balance_hexgrid(files):
    result = dict(files)
    config = result['config.cpp'].decode('utf-8-sig')
    start, _, _, end = section(config, 'GlobalHEXGRID')
    armour = config[start:end].replace('class GlobalHEXGRID', 'class GlobalArmor', 1)
    p_start, _, _, p_end = section(armour, 'Projectile')
    projectile = armour[p_start:p_end]
    h_start, _, _, h_end = section(projectile, 'Health')
    health, count = re.subn(r'\bdamage\s*=\s*[^;]+;', f'damage = {HEALTH_MULTIPLIER};', projectile[h_start:h_end])
    assert count == 1
    projectile = projectile[:h_start] + health + projectile[h_end:]
    armour = armour[:p_start] + projectile + armour[p_end:]
    config = config[:start] + armour + config[end:]
    start, _, _, end = section(config, 'GlobalHealth')
    durability, count = re.subn(r'\bhitpoints\s*=\s*150\s*;', f'hitpoints = {VEST_HITPOINTS};', config[start:end])
    assert count == 1
    config = config[:start] + durability + config[end:]
    config = config.replace('Hexgrid has 95% Damage Reduction And Is A TIER 5 Categorised Vest.', 'Heavy ballistic vest.')
    result['config.cpp'] = config.encode()
    assert 'class GlobalHEXGRID' not in config
    return result


def report():
    values = {}
    for name, ammo in AMMO.items():
        effective = ammo['damage'] * HEALTH_MULTIPLIER
        assert math.ceil(100 / effective) == ammo['shots']
        assert (ammo['shots'] - 1) * effective < 100 <= ammo['shots'] * effective
        values[name] = {'base_health_damage': ammo['damage'], 'calculated_damage': effective,
                        'target_shots_from_100_health': ammo['shots']}
    return {'health_multiplier': HEALTH_MULTIPLIER, 'vest_hitpoints': VEST_HITPOINTS,
            'affected_base': 'HEXGRID_Base', 'applies_to': ['UAV_HEXGRID', 'GRZ_GunRunnerZ_Hexgrid'],
            'ammo': values,
            'assumptions': '100 player health, covered direct torso hits; native hitzone/range, other gear and mods require in-game testing. Other projectiles also use the native armour multiplier.'}
