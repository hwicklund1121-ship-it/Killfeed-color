"""Exercise the actual PowerShell/C# builder with isolated binary PBO/raP fixtures.

Example: python3 tools/test_inventory_builder.py --pwsh /path/to/pwsh
No DayZ archives are bundled. This is not a DayZ gameplay test.
"""
import argparse
import hashlib
import json
import os
import re
import shutil
from pathlib import Path
import struct
import subprocess
import tempfile
from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]


def z(value):
    return value.encode() + b'\0'


def integer(value):
    data = bytearray()
    while value > 127:
        data.append((value & 127) | 128)
        value >>= 7
    data.append(value)
    return bytes(data)


def node(name, parent='', children=(), properties=None):
    return name, parent, list(children), properties or {}


def rap(root):
    data = bytearray(b'\0raP' + struct.pack('<III', 0, 8, 0))

    def body(entry):
        name, parent, children, properties = entry
        offset = len(data)
        data.extend(z(parent) + integer(len(children) + len(properties)))
        for key, value in properties.items():
            if isinstance(value, str):
                data.extend(b'\x01\x00' + z(key) + z(value))
            else:
                data.extend(b'\x02' + z(key) + integer(len(value)))
                for item in value:
                    data.extend(b'\x00' + z(item) if isinstance(item,str) else b'\x02' + struct.pack('<i',item))
        fixes = []
        for child in children:
            data.extend(b'\0' + z(child[0]))
            location = len(data)
            data.extend(bytes(4))
            fixes.append((location, child))
        for location, child in fixes:
            struct.pack_into('<I', data, location, body(child))
        return offset

    body(root)
    return bytes(data)


def archive(path, root, prefix='Fixture'):
    with tempfile.TemporaryDirectory() as tmp:
        source = Path(tmp)
        (source/'$PBOPREFIX$').write_text(prefix)
        (source/'config.bin').write_bytes(rap(root))
        path.write_bytes(build(source))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pwsh', required=True)
    parser.add_argument('--reference-pbo', type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='grz inventory test ') as tmp:
        temp = Path(tmp)
        game = temp/'DayZ Game'
        (game/'Addons').mkdir(parents=True)
        (game/'dta').mkdir()
        (game/'DayZ_x64.exe').touch()
        # Engine anchor classes have no CfgPatches provider.
        archive(game/'dta/bin.pbo',node('',children=[node('CfgPatches'),node('CfgVehicles',children=[node('Man','All'),node('Clothing','Thing')]),node('CfgWeapons',children=[node('Weapon_Base'),node('DefaultWeapon','Weapon_Base'),node('PistolCore','DefaultWeapon'),node('RifleCore','DefaultWeapon'),node('LauncherCore','DefaultWeapon'),node('Rifle_Base','DefaultWeapon')])]))
        if args.reference_pbo:
            shutil.copyfile(args.reference_pbo, game/'dta/bin.pbo')
        vehicles = [
            node('Man','SurvivorBase'),node('SurvivorBase','Man'),node('Inventory_Base'),
            node('Clothing','Inventory_Base',properties={'inventorySlot':[],'itemsCargoSize':[0,0]}),
            node('Jacket_ColorBase','Clothing',properties={'inventorySlot':['Body'],'itemsCargoSize':[4,3]}),
            node('Jacket_Black','Jacket_ColorBase'),
            node('Shoes_ColorBase','Clothing',properties={'inventorySlot':['Feet']}),node('Shoes_Blue','Shoes_ColorBase'),
            node('Backpack_Base','Clothing',properties={'inventorySlot':['Back'],'itemsCargoSize':[8,10]}),node('AliceBag_Black','Backpack_Base'),
            node('Vest_Base','Clothing',properties={'inventorySlot':['Vest'],'itemsCargoSize':[6,4]}),node('Vest_Black','Vest_Base'),
            node('Pants_Base','Clothing',properties={'inventorySlot':['Legs'],'itemsCargoSize':[4,5]}),node('Pants_Blue','Pants_Base'),
            node('Belt_Base','Clothing',properties={'inventorySlot':'Hips','itemsCargoSize':[3,2]}),node('Belt_Brown','Belt_Base'),
            node('Hat_Base','Clothing',properties={'inventorySlot':['Headgear']}),
            node('Gloves_Base','Clothing',properties={'inventorySlot':['Gloves']}),
            node('Mask_Base','Clothing',properties={'inventorySlot':['Mask']}),
            node('Armband_Base','Clothing',properties={'inventorySlot':['Armband']}),
            node('Hat_From_Jacket','Jacket_ColorBase',properties={'inventorySlot':['Headgear']}),
            node('Grenade','Inventory_Base')]

        weapons = [node('Rifle_Base','RifleCore'),node('M4A1','Rifle_Base'),node('Pistol_Base','PistolCore'),node('Glock19','Pistol_Base'),node('Launcher_Base','LauncherCore'),node('M79','Launcher_Base')]
        archive(game/'Addons/base.pbo',node('',children=[node('CfgPatches',children=[node('DZ_Fixture')]),node('CfgVehicles',children=vehicles),node('CfgWeapons',children=weapons)]))
        # A later patch reopens an existing class without repeating its parent.
        archive(game/'Addons/variant.pbo',node('',children=[node('CfgPatches',children=[node('DZ_Fixture_Variant')]),node('CfgVehicles',children=[node('Jacket_Black')])]))
        environment=os.environ.copy()
        for kind in ['CACHE','CONFIG','DATA']:
            environment['XDG_'+kind+'_HOME']=str(temp/kind.lower())
        output = temp/'Built Mod'
        command=[args.pwsh,'-NoProfile','-File',str(ROOT/'packaging/inventory-builder/Build-Inventory-Mod.ps1'),'-DayZPath',str(game),'-OutputPath',str(output),'-NoExplorer']
        result=subprocess.run(command,env=environment,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        report=json.loads((output/'Inventory-catalog.json').read_text(encoding='utf-8-sig'))
        assert len(report['Clothing'])==8 and len(report['NormalClothing'])==10 and len(report['Guns'])==6
        assert 'Grenade' not in report['Clothing'] and 'Weapon_Base' not in report['Guns']
        assert 'DZ_Fixture' in report['RequiredAddons'] and 'DZ_Fixture_Variant' in report['RequiredAddons']
        config=(output/'config.cpp').read_text()
        assert config.count('itemsCargoSize[] = {10, 10};')==8
        assert config.count('itemSize[] = {10, 2};')==6
        for name in ['Shoes_ColorBase','Shoes_Blue','Hat_Base','Gloves_Base','Mask_Base','Armband_Base','Backpack_Base','AliceBag_Black','Hat_From_Jacket']:
            assert name not in report['Clothing']
        assert 'Belt_Brown' in report['Clothing'] and 'Jacket_Black' in report['Clothing']
        assert re.search(r'class Hat_From_Jacket: Jacket_ColorBase\s*\{\s*itemsCargoSize\[\] = \{4, 3\}',config)
        assert re.search(r'class AliceBag_Black: Backpack_Base\s*\{\s*itemsCargoSize\[\] = \{8, 10\}',config)
        assert 'class Man' not in config and 'class SurvivorBase' not in config
        assert 'class Clothing: Inventory_Base' in config
        assert 'class Rifle_Base: RifleCore' in config
        assert 'class Jacket_Black: Jacket_ColorBase' in config
        assert config.index('class Jacket_ColorBase:')<config.index('class Jacket_Black:')
        pbo=(output/'GRZ_InventorySizes.pbo').read_bytes()
        assert pbo[-21]==0 and hashlib.sha1(pbo[:-21]).digest()==pbo[-20:]
        source=temp/'PBO Verification';source.mkdir()
        (source/'$PBOPREFIX$').write_text('GRZ_InventorySizes')
        (source/'config.cpp').write_bytes((output/'config.cpp').read_bytes())
        assert pbo==build(source),'Generated PBO differs from established Python packer'
        # Explicit extra mod PBOs must override their own concrete gun size.
        extra_dir=temp/'Extra Weapon Mod';extra_dir.mkdir()
        extra=extra_dir/'UAVWEAPONS.pbo'
        archive(extra,node('',children=[node('CfgPatches',children=[node('UAV_Fixture')]),node('CfgWeapons',children=[node('UAV_CS5_BLACK','Rifle_Base',properties={'itemSize':[10,4]})])]))
        extra_output=temp/'Extra Built Mod'
        extra_command=command.copy();extra_command[extra_command.index(str(output))]=str(extra_output)
        extra_command+=['-ExtraPboPaths',str(extra)]
        result=subprocess.run(extra_command,env=environment,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        extra_config=(extra_output/'config.cpp').read_text()
        assert re.search(r'class UAV_CS5_BLACK: Rifle_Base\s*\{\s*itemSize\[\] = \{10, 2\}',extra_config)
        assert 'UAV_Fixture' in json.loads((extra_output/'Inventory-catalog.json').read_text(encoding='utf-8-sig'))['RequiredAddons']
        assert (extra_output/'ExtraConfigs/UAVWEAPONS-config.bin').exists()
        # Windstride keeps definitions in nested configs; include binary and source,
        # independent Clothing descendants, scalar slots and concrete overrides.
        with tempfile.TemporaryDirectory() as ws:
            ws=Path(ws);(ws/'$PBOPREFIX$').write_text('WindstrideClothing')
            (ws/'config.bin').write_bytes(rap(node('',children=[node('CfgPatches',children=[node('WindstrideContainer')])])))
            nested=ws/'Models/Layered_Shirt';nested.mkdir(parents=True)
            (nested/'config.bin').write_bytes(rap(node('',children=[
                node('CfgPatches',children=[node('Layered_Shirt')]),
                node('CfgVehicles',children=[node('Layered_Shirt_Base','Clothing',properties={'inventorySlot':['Body'],'itemsCargoSize':[3,3]}),node('Layered_Shirt_Lily','Layered_Shirt_Base')])
            ])))
            # A source sibling next to a binary config must not be parsed twice.
            (nested/'config.cpp').write_text('#include "not-used-because-binary-exists.hpp"')
            custom=ws/'Models/Custom';custom.mkdir(parents=True)
            (custom/'config.cpp').write_text("""// Source mod metadata, no external preprocessor.
#define _ARMA_
class CfgPatches { class XIMS_Layered_Fixture {}; };
class CfgVehicles {
 class Layered_Shirt_Base;
 class XIMS_SourceTop: Layered_Shirt_Base { inventorySlot = "body"; itemsCargoSize[] = {6,4}; class DamageSystem { class Health { healthLevels[]={{1,{"path\\material.rvmat"}}}; }; }; };
 class WindstrideBag: Clothing { inventorySlot[] = {"Back"}; itemsCargoSize[] = {8,10}; };
};
""")
            windstride=extra_dir/'WindstrideClothing.pbo';windstride.write_bytes(build(ws))
        wind_output=temp/'Windstride Built'
        wind_command=command.copy();wind_command[wind_command.index(str(output))]=str(wind_output)
        wind_command+=['-ExtraPboPaths',str(windstride)]
        result=subprocess.run(wind_command,env=environment,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        wind_cfg=(wind_output/'config.cpp').read_text()
        wind_report=json.loads((wind_output/'Inventory-catalog.json').read_text(encoding='utf-8-sig'))
        for classname in ['Layered_Shirt_Base','Layered_Shirt_Lily','XIMS_SourceTop']:
            assert classname in wind_report['Clothing']
            assert re.search(r'class '+classname+r'(?:: [^{}]+)?\s*\{\s*itemsCargoSize\[\] = \{10, 10\}',wind_cfg)
        assert 'WindstrideBag' in wind_report['NormalClothing'] and 'WindstrideBag' not in wind_report['Clothing']
        assert re.search(r'class WindstrideBag: Clothing\s*\{\s*itemsCargoSize\[\] = \{8, 10\}',wind_cfg)
        assert 'Layered_Shirt' in wind_report['RequiredAddons'] and 'XIMS_Layered_Fixture' in wind_report['RequiredAddons']
        assert (wind_output/'ExtraConfigs/WindstrideClothing-Models_Layered_Shirt_config.bin').exists()
        assert (wind_output/'ExtraConfigs/WindstrideClothing-Models_Custom_config.cpp').exists()
        # Enumerate native animation headers only, excluding actions and prone gait.
        animations=temp/'Native Animations';animations.mkdir()
        (animations/'$PBOPREFIX$').write_text('DZ\\anims')
        names=['p_rfl_erc_runF_low.anm','p_rfl_erc_walkF_low.anm','p_rfl_erc_reload.anm','p_rfl_pne_runF_low.anm','p_rfl_erc_runF_stop.anm']
        for name in names:
            clip=animations/'anm/player/moves/rifles'/name;clip.parent.mkdir(parents=True,exist_ok=True);clip.write_bytes(b'animation fixture')
        animation_pbo=temp/'anims.pbo';animation_pbo.write_bytes(build(animations))
        movement_output=temp/'Movement Built'
        movement_script=temp/'BuildMovement.ps1'
        def psquote(value): return "'"+str(value).replace("'","''")+"'"
        movement_script.write_text('Add-Type -Path '+psquote(ROOT/'packaging/inventory-builder/InventoryBuilder.cs')+'\n[GRZInventoryBuilder]::BuildMovementData([string[]]@('+psquote(animation_pbo)+'),'+psquote(movement_output)+')\n')
        result=subprocess.run([args.pwsh,'-NoProfile','-File',str(movement_script)],env=environment,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        data=json.loads((movement_output/'gaits.json').read_text())
        assert data['Multiplier']==1.10
        assert data['Animations']==['dz/anims/anm/player/moves/rifles/p_rfl_erc_runf_low.anm','dz/anims/anm/player/moves/rifles/p_rfl_erc_walkf_low.anm']
        from repair_hexgrid import read_pbo
        ext,payload=read_pbo((movement_output/'GRZ_MovementSpeedData.pbo').read_bytes())
        assert ext['prefix']=='GRZ_MovementSpeedData' and json.loads(payload['gaits.json'])==data
        assert set(payload)=={'config.cpp','gaits.json'}
        # Unsupported preprocessing still exports the small diagnostic source file.
        cpp_source=temp/'Source Config';cpp_source.mkdir()
        (cpp_source/'$PBOPREFIX$').write_text('SourceFixture')
        source_text='#include "missing-original.hpp"\nclass CfgPatches { class SourceFixture {}; };'
        (cpp_source/'config.cpp').write_text(source_text)
        cpp_pbo=extra_dir/'SourceWeapon.pbo';cpp_pbo.write_bytes(build(cpp_source))
        cpp_output=temp/'Source Export'
        cpp_command=extra_command.copy()
        cpp_command[cpp_command.index(str(extra_output))]=str(cpp_output)
        cpp_command[cpp_command.index(str(extra))]=str(cpp_pbo)
        failed_cpp=subprocess.run(cpp_command,env=environment,capture_output=True,text=True)
        assert failed_cpp.returncode!=0 and not (cpp_output/'GRZ_InventorySizes.pbo').exists()
        assert (cpp_output/'ExtraConfigs/SourceWeapon-config.cpp').read_text()==source_text
        # Inactive conflicting mods must not silently choose an arbitrary parent.
        archive(game/'Addons/conflict.pbo',node('',children=[node('CfgPatches',children=[node('Bad_Fixture')]),node('CfgVehicles',children=[node('Jacket_Black','Inventory_Base')])]))
        bad_output=temp/'Failed Build'
        failure_command=command.copy();failure_command[failure_command.index(str(output))]=str(bad_output)
        failed=subprocess.run(failure_command,env=environment,capture_output=True,text=True)
        assert failed.returncode!=0 and not (bad_output/'GRZ_InventorySizes.pbo').exists()
        error_text = ' '.join(re.sub(r'(?m)^\s*\|\s*', '', failed.stdout+failed.stderr).split())
        assert 'Conflicting base classes' in error_text,error_text
        (game/'Addons/conflict.pbo').unlink()
        archive(game/'Addons/compressed.pbo',node('',children=[node('CfgPatches',children=[node('Compressed_Fixture')])]))
        compressed=bytearray((game/'Addons/compressed.pbo').read_bytes())
        method=compressed.index(b'config.bin\0')+len(b'config.bin\0')
        struct.pack_into('<I',compressed,method,0x43707273)
        (game/'Addons/compressed.pbo').write_bytes(compressed)
        failed=subprocess.run(failure_command,env=environment,capture_output=True,text=True)
        assert failed.returncode!=0 and not (bad_output/'GRZ_InventorySizes.pbo').exists()
        assert 'Compressed config.bin' in failed.stdout+failed.stderr
        print('PowerShell/C# tests passed: normal unrelated engine redefinitions, inherited variants, original parents, unrelated items, restricted clothing slots, restored excluded cargo, exact cargo/footprint fields, provider dependencies, explicit modded-gun override/export, nested Windstride/source configs, native movement header filtering, PBO byte/checksum compatibility, and fail-closed conflict/unsupported compression handling.')


if __name__=='__main__':
    main()
