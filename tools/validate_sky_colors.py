"""Check sky table isolation, settings bindings and archive payloads.

This does not compile Enforce Script or render DayZ skies.
"""
import hashlib
import json
import re
from pathlib import Path
from zipfile import ZipFile
from build_pbo import build
from build_sky_lighting import ARRAY, TRIPLET
ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'GRZ_SkyColors'


def main():
    palette = json.loads((SOURCE/'data/palette.json').read_text())['Colors']
    assert len(palette)==11 and palette[0]['Key']=='default'
    assert len({c['Key'] for c in palette})==len(palette)
    for color in palette:
        assert re.fullmatch('[a-z]+',color['Key'])
        assert len(color['RGB'])==3 and all(0<=x<=1 for x in color['RGB'])
    manifest = json.loads((SOURCE/'data/lighting-manifest.json').read_text())
    for base in ['default','darknight']:
        non_sky = None
        classes = None
        for color in palette[1:]:
            name = base+'_'+color['Key']+'.txt'
            data = (SOURCE/'data/lighting'/name).read_bytes()
            entry = manifest['tables'][base]['files'][name]
            assert hashlib.sha256(data).hexdigest()==entry['sha256']
            text = data.decode()
            assert text.count('{')==text.count('}')
            profiles = re.findall(r'class\s+(\w+)',text)
            if classes is None: classes=profiles
            assert classes==profiles
            unchanged = ARRAY.sub('',text)
            if non_sky is None: non_sky=unchanged
            assert non_sky==unchanged, name+' modifies non-sky/cloud values'
            fields = list(ARRAY.finditer(text))
            assert len(fields)==entry['changed_sky_fields'] and len(fields)>=40
            for field in fields:
                rgb=[float(v) for v in TRIPLET.search(field[2]).groups()]
                if color['Key']=='black': assert rgb==[0,0,0]
                elif max(rgb)>0:
                    peak=max(rgb)
                    assert all(abs(a/peak-b)<1e-6 for a,b in zip(rgb,color['RGB']))
        assert 'diffuseCloud[]' in non_sky and 'ambientCloud[]' in non_sky
        print(base+': 10 presets retain identical non-sky/cloud tables.')
    widgets=set()
    for p in SOURCE.rglob('*.layout'):
        text=p.read_text();assert text.count('{')==text.count('}')
        widgets.update(re.findall(r'\w+WidgetClass\s+(\w+)',text))
    script='\n'.join(p.read_text() for p in SOURCE.rglob('*.c'))
    refs=set(re.findall(r'FindAnyWidget\("(\w+)"\)',script))
    assert not refs-widgets-{'game_settings_root','game_settings_scroll'},refs-widgets
    assert 'section.SetSort(210)' in script
    for method in ['Apply','Revert','SetToDefaults','OnHide']:
        assert re.search(r'override (?:bool|void) '+method+r'\(',script)
    assert 'GRZSkyRuntime.Preview(index)' in script
    assert 'GRZSkyRuntime.EndPreview()' in script
    data=(ROOT/'downloads/GRZ_SkyColors.pbo').read_bytes()
    assert data==build(SOURCE)
    with ZipFile(ROOT/'downloads/GRZ_SkyColors-mod.zip') as z:
        assert z.testzip() is None
        assert z.read('@GRZ_SkyColors/Addons/GRZ_SkyColors.pbo')==data
    print('Sky ZIP, palette, UI bindings and save/revert hooks checked. Runtime/UI rendering remains untested.')

if __name__ == '__main__':
    main()
