"""Add a permanent suppressor and unique visual proxy to the updated SFG folder."""
import hashlib
import json
import re
import tempfile
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

from build_pbo import build
from repair_hexgrid import read_pbo, resource_refs

ROOT = Path(__file__).resolve().parents[1]
PREFIX = r'UAVWEAPONS\SFG50\SFG'
archive_path = ROOT / 'downloads/SFG50-SFG-Sounds.zip'
with ZipFile(archive_path) as archive:
    original = {name[len('SFG50/SFG/'):]: archive.read(name) for name in archive.namelist() if name.startswith('SFG50/SFG/') and not name.endswith('/')}
files = dict(original)
# Avoid a globally shared Proxysuppressor definition being reused by other guns.
# Equal-length model string edits preserve all ODOL offsets, sizes and geometry.
model = files['data/SFG50.p3d']
old_path = rb'\uavweapons\sfg50\sfg\proxy\suppressor'
new_path = rb'\uavweapons\sfg50\sfg\proxy\sfg_muzzle'
assert len(old_path) == len(new_path)
model, edits = re.subn(re.escape(old_path), lambda match: new_path, model, flags=re.I)
assert edits >= 1 or old_path not in model.lower()
assert len(model) == len(files['data/SFG50.p3d']) and model[:8] == files['data/SFG50.p3d'][:8]
files['data/SFG50.p3d'] = model
if 'proxy/suppressor.p3d' in files:
    files['proxy/sfg_muzzle.p3d'] = files.pop('proxy/suppressor.p3d')
config = files['config.cpp'].decode('utf-8-sig')
config = config.replace('class Proxysuppressor:', 'class Proxysfg_muzzle:')
config = config.replace('\\proxy\\suppressor.p3d', '\\proxy\\sfg_muzzle.p3d')
config, chamber_edits = re.subn(r'chamberableFrom\[\]\s*=\s*\{[^}]*\};', 'chamberableFrom[] = {"UAV_Ammo_VULCAN"};', config)
assert chamber_edits == 1
config, scope_edits = re.subn(r'(class\s+UAV_SFG_Sup\s*:\s*SFG_Suppressor_Base\s*\{\s*)scope\s*=\s*\d+;', lambda match: match[1] + 'scope = 1;', config)
assert scope_edits == 1
config = config.replace('units[] = {"UAV_SFG_Sup"};', 'units[] = {};')
files['config.cpp'] = config.encode()
# The supplied root already includes the complete suppressor definitions.
files.pop('proxy/config.cpp', None)
script_path = 'scripts/4_World/Weapons/sfg50.c'
script = files[script_path].decode('utf-8-sig')
script = re.sub(r"(?m)^\s*''\s*$", '', script)
if 'GRZEnsurePermanentSuppressor' not in script:
    end = script.rfind('};')
    assert end >= 0
    script = script[:end] + '''
    protected int m_GRZSFGSuppressorAttempts;

    override void EEInit()
    {
        super.EEInit();
        GRZQueuePermanentSuppressor();
    }

    override void EEOnAfterLoad()
    {
        super.EEOnAfterLoad();
        GRZQueuePermanentSuppressor();
    }

    protected void GRZQueuePermanentSuppressor()
    {
        if (!g_Game.IsServer())
            return;
        m_GRZSFGSuppressorAttempts = 0;
        g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GRZEnsurePermanentSuppressor);
        g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZEnsurePermanentSuppressor, 100, false);
    }

    void GRZEnsurePermanentSuppressor()
    {
        if (!g_Game.IsServer())
            return;
        int slot = InventorySlots.GetSlotIdFromString("UAV_SFG_Suppressor");
        if (slot < 0)
            return;
        EntityAI suppressor = FindAttachmentBySlotName("UAV_SFG_Suppressor");
        if (!suppressor)
        {
            GetInventory().SetSlotLock(slot, false);
            suppressor = GetInventory().CreateAttachmentEx("UAV_SFG_Sup", slot);
        }
        if (suppressor)
        {
            GetInventory().SetSlotLock(slot, true);
            return;
        }
        m_GRZSFGSuppressorAttempts++;
        if (m_GRZSFGSuppressorAttempts < 5)
            g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZEnsurePermanentSuppressor, 250, false);
        else
            Error("SFG permanent suppressor: attachment could not be created. Check UAV_SFG_Sup config and slot.");
    }

    override bool CanReleaseAttachment(EntityAI attachment)
    {
        if (attachment && attachment.IsKindOf("SFG_Suppressor_Base"))
            return false;
        return super.CanReleaseAttachment(attachment);
    }

    override void EEDelete(EntityAI parent)
    {
        if (g_Game)
            g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GRZEnsurePermanentSuppressor);
        super.EEDelete(parent);
    }
''' + script[end:]
if 'override bool CanDisplayAttachmentSlot(int slot_id)' not in script:
    end = script.rfind('};')
    assert end >= 0
    script = script[:end] + '''
    override bool CanDisplayAttachmentSlot(int slot_id)
    {
        if (slot_id == InventorySlots.GetSlotIdFromString("UAV_SFG_Suppressor"))
            return false;
        return super.CanDisplayAttachmentSlot(slot_id);
    }
''' + script[end:]
assert 'return super.CanDisplayAttachmentSlot(slot_id);' in script
if 'override void OnDebugSpawn()' not in script:
    end = script.rfind('};')
    script = script[:end] + '''
    override void OnDebugSpawn()
    {
        super.OnDebugSpawn();
        GRZQueuePermanentSuppressor();
    }
''' + script[end:]
# Try immediately for spawned guns; existing retries still handle inventory initialization.
script = script.replace(
    'g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZEnsurePermanentSuppressor, 100, false);',
    'GRZEnsurePermanentSuppressor();')
assert not re.search(r"(?m)^\s*'{1,2}\s*$", script)
files[script_path] = script.encode()
assert files['proxy/sfg_muzzle.p3d'] == original.get('proxy/suppressor.p3d', original.get('proxy/sfg_muzzle.p3d'))
assert 'class Proxysfg_muzzle:' in config and 'class Proxysuppressor:' not in config
mounted = {(PREFIX + '/' + name).replace('\\', '/').lower() for name in files}
for name, data in files.items():
    if name.endswith(('.cpp', '.rvmat')):
        refs = re.findall(r'"([^"\r\n]+\.(?:p3d|paa|rvmat))"', data.decode('utf-8-sig'), re.I)
    elif name.endswith('.p3d'):
        refs = resource_refs(data)
    else:
        continue
    for ref in refs:
        ref = ref.replace('\\', '/').lstrip('/').lower()
        if ref.startswith('uavweapons/sfg50/sfg/'):
            assert ref in mounted, (name, ref)
with tempfile.TemporaryDirectory(prefix='sfg-permanent-check-') as directory:
    source = Path(directory)
    (source / '$PBOPREFIX$').write_text(PREFIX)
    for name, data in files.items():
        target = source / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    packed = build(source)
assert read_pbo(packed)[1] == files
report = {'suppressor_scope': 1, 'hidden_from_spawn_menu': True, 'hidden_inventory_slot': 'UAV_SFG_Suppressor', 'chamberable_pile': 'UAV_Ammo_VULCAN', 'fired_projectile': 'UAV_Bullet_VULCAN',
          'permanent_suppressor_class': 'UAV_SFG_Sup', 'slot': 'UAV_SFG_Suppressor',
          'unique_proxy_class': 'Proxysfg_muzzle', 'equal_length_model_reference_edits': edits,
          'validation_pbo_sha256': hashlib.sha256(packed).hexdigest(),
          'audio_sha256': {name: hashlib.sha256(data).hexdigest() for name, data in files.items() if name.endswith('.ogg')}}
(ROOT / 'SFG-PERMANENT-SUPPRESSOR-MANIFEST.json').write_text(json.dumps(report, indent=2) + '\n')
with ZipFile(archive_path, 'w', ZIP_DEFLATED) as archive:
    for name, data in files.items():
        archive.writestr('SFG50/SFG/' + name, data)
    archive.write(ROOT / 'SFG-REPLACEMENT.md', 'INSTALL.md')
    archive.write(ROOT / 'SFG-PERMANENT-SUPPRESSOR-MANIFEST.json', 'MANIFEST.json')
with ZipFile(archive_path) as archive:
    assert archive.testzip() is None
    for name, data in files.items():
        assert archive.read('SFG50/SFG/' + name) == data
print('Permanent-suppressor spawn/load hooks, unique matching proxy references, unchanged model size/header and suppressor geometry, retained audio, PBO round-trip and ZIP verified. In-game attachment/rendering still requires testing.')
