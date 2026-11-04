"""Pack additive reskins under one root config and one asset mount."""
import hashlib
import re
import tempfile
from pathlib import Path

from build_pbo import build
from repair_hexgrid import read_pbo, section


def consolidate(addons):
    sources = []
    for addon in addons:
        extension, files = read_pbo(addon)
        sources.append((extension['prefix'], files))
    mapping = {prefix: 'GRZ_Customs/' + prefix.replace('\\', '/').split('/')[-1] for prefix, _ in sources}
    files, parents, classes, dependencies, definitions = {}, set(), [], set(), []
    for prefix, entries in sources:
        config = entries['config.cpp'].decode('utf-8-sig')
        _, opening, closing, _ = section(config, 'CfgVehicles')
        vehicles = config[opening + 1:closing]
        parents.update(re.findall(r'class\s+(\w+)\s*;', vehicles))
        vehicles = re.sub(r'\bclass\s+\w+\s*;', '', vehicles)
        classes.extend(re.findall(r'\bclass\s+(\w+)\s*:', vehicles))
        definitions.append(vehicles)
        for value in re.findall(r'requiredAddons\[\]\s*=\s*\{([^}]+)\}', config):
            dependencies.update(re.findall(r'"([^"]+)"', value))
        for name, data in entries.items():
            if name == 'config.cpp':
                continue
            target = mapping[prefix].split('/', 1)[1] + '/' + name
            assert target not in files
            files[target] = data
    units = ','.join('"' + name + '"' for name in classes)
    required = ','.join('"' + name + '"' for name in sorted(dependencies))
    config = ('class CfgPatches { class GRZ_Customs_Items { units[] = {' + units + '}; weapons[] = {}; '
              'requiredVersion = 0.1; requiredAddons[] = {' + required + '}; }; };\n'
              'class CfgMods { class GRZ_Customs { dir = "GRZ_Customs"; name = "GRZ Customs"; type = "mod"; '
              'dependencies[] = {"Mission"}; class defs { class missionScriptModule { value = ""; '
              'files[] = {"GRZ_Customs/scripts/5_Mission"}; }; }; }; };\n'
              'class CfgVehicles {\n' + ''.join('class ' + p + ';\n' for p in sorted(parents)) +
              '\n'.join(definitions) + '\n};\n')
    files['config.cpp'] = config.encode()
    # Only text references move; original binary artwork is untouched.
    for name, data in list(files.items()):
        if name.endswith(('.cpp', '.rvmat')):
            text = data.decode('utf-8-sig')
            for old, new in sorted(mapping.items(), key=lambda item: -len(item[0])):
                old = old.replace('\\', '/')
                for separator in ['/', '\\']:
                    text = text.replace(old.replace('/', separator) + separator,
                                        new.replace('/', separator) + separator)
            files[name] = text.encode()
    names = '\n'.join('        items.Insert("' + name + '");' for name in classes)
    files['scripts/5_Mission/GRZCustomsRegistration.c'] = ('''
class GRZCustomsRegistration
{
    protected static bool s_Checked;
    static void Check()
    {
        if (s_Checked)
            return;
        s_Checked = true;
        array<string> items = new array<string>;
ITEM_NAMES
        int loaded;
        string report = "GRZ Customs 0.1.9 registration check\\n";
        foreach (string itemName : items)
        {
            string path = "CfgVehicles " + itemName;
            if (g_Game.ConfigIsExisting(path) && g_Game.ConfigGetInt(path + " scope") == 2)
                loaded++;
            else
                report += "MISSING: " + itemName + "\\n";
        }
        report += "Registered " + loaded.ToString() + "/" + items.Count().ToString() + " custom items.\\n";
        Print(report);
        FileHandle output = OpenFile("$profile:GRZ_Customs_registration.txt", FileMode.WRITE);
        if (output)
        {
            FPrint(output, report);
            CloseFile(output);
        }
    }
}
modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        GRZCustomsRegistration.Check();
    }
}
modded class MissionGameplay
{
    override void OnInit()
    {
        super.OnInit();
        GRZCustomsRegistration.Check();
    }
}
'''.replace('ITEM_NAMES', names)).encode()
    assert len(classes) == len(set(classes)) == 55
    mounted = {'grz_customs/' + name.lower() for name in files}
    for name, data in files.items():
        if name.endswith(('.cpp', '.rvmat')):
            for ref in re.findall(r'"([^"\r\n]+\.(?:paa|rvmat))"', data.decode(), re.I):
                if ref.lower().startswith('grz_customs'):
                    assert ref.replace('\\', '/').lower() in mounted, ref
    with tempfile.TemporaryDirectory(prefix='grz-customs-consolidated-') as directory:
        source = Path(directory)
        (source / '$PBOPREFIX$').write_text('GRZ_Customs')
        for name, data in files.items():
            target = source / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        pbo = build(source)
    assert read_pbo(pbo)[1] == files
    return pbo, {'prefix': 'GRZ_Customs', 'sha256': hashlib.sha256(pbo).hexdigest(),
                 'files': len(files), 'item_classes': classes}
