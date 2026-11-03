"""Validate HUD packaging and outline/fill layers, not DayZ runtime rendering."""
import re
from pathlib import Path
from zipfile import ZipFile
from build_pbo import build
from build_hud_fill_layers import decode_top_mip

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'GRZ_CustomHud'


def main():
    data = (ROOT/'downloads/GRZ_CustomHud.pbo').read_bytes()
    assert data == build(SOURCE)
    with ZipFile(ROOT/'downloads/GRZ_CustomHud-mod.zip') as archive:
        assert archive.testzip() is None
        assert archive.read('@GRZ_CustomHud/Addons/GRZ_CustomHud.pbo') == data
    script = '\n'.join(p.read_text() for p in SOURCE.rglob('*.c'))
    declared = set()
    for p in SOURCE.rglob('*.layout'):
        text = p.read_text()
        assert text.count('{') == text.count('}')
        declared.update(re.findall(r'\w+WidgetClass\s+(\w+)',text))
    # These are already-loaded native HUD widgets, reparented into our status panel.
    native = {'Bleeding','BleedingCount','ia_item','ia_item_subdesc','ia_interact','ia_continuous_interact','ia_single','ia_continuous'}
    refs=set(re.findall(r'FindAnyWidget\("(\w+)"\)',script))
    assert not refs-declared-native,refs-declared-native
    assert 'values.GetHealth()' in script and 'values.GetBlood()' in script
    assert 'TransferValues.BLOOD_THRESHOLD_LOW' in script
    assert 'outline.Show(true);' in script
    assert 'LoadMaskTexture' not in script and 'SetMaskProgress' not in script
    assert 'clip.Show(fraction > 0);' in script
    assert 'fill.SetPos(0, -cutoff / visibleHeight);' in script
    assert 'fill.SetSize(1, 1 / visibleHeight);' in script
    assert 'm_GRZHealthTrend = tendency;' in script and 'm_GRZBloodTrend = tendency;' in script
    assert 'key == NTFKEY_BLEEDISH' in script
    for name in ['health','blood']:
        source=decode_top_mip((SOURCE/f'gui/textures/grz_{name}_ca.paa').read_bytes())
        outline=decode_top_mip((SOURCE/f'gui/textures/grz_{name}_outline_ca.paa').read_bytes())
        fill=decode_top_mip((SOURCE/f'gui/textures/grz_{name}_fill_ca.paa').read_bytes())
        assert source.size==outline.size==fill.size
        pixels=list(zip(source.get_flattened_data(),outline.get_flattened_data(),fill.get_flattened_data()))
        black=[(a,oa) for (r,g,b,a),(_,_,_,oa),_ in pixels if a>200 and max(r,g,b)<20]
        white=[oa for (r,g,b,a),(_,_,_,oa),_ in pixels if a>200 and min(r,g,b)>245]
        assert black and white
        assert all(oa/a>0.8 for a,oa in black),name+' outline lost'
        assert all(oa<20 for oa in white),name+' empty interior not transparent'
        error=0;count=0
        for (r,g,b,a),(_,_,_,oa),(_,_,_,fa) in pixels:
            if a<25:
                continue
            expected=r/255*a/255
            combined=fa/255*(1-oa/255)
            error+=abs(expected-combined)
            count+=1
        assert error/count<0.03,(name,error/count)
        assert outline.getpixel((0,0))[3]==0 and fill.getpixel((0,0))[3]==0
        print(f'{name}: outline retained at empty; full layered reconstruction mean error {error/count:.4f}.')
    print('HUD archive matches source. Native clipping and value sync still require an in-game check.')


if __name__=='__main__':
    main()
