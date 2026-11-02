"""Validate HUD packaging and technical fill masks, not DayZ runtime rendering."""
import re
from pathlib import Path
from zipfile import ZipFile
from build_pbo import build
from build_hud_fill_masks import decode_top_mip

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
    assert 'icon.SetColor(0xFFFFFFFF);' in script
    assert 'icon.SetMaskProgress(0.03 + 0.94 * GRZFillFraction(name, status));' in script
    assert 'm_GRZHealthTrend = tendency;' in script and 'm_GRZBloodTrend = tendency;' in script
    assert 'key == NTFKEY_BLEEDISH' in script
    for name in ['health','blood']:
        source = decode_top_mip((SOURCE/f'gui/textures/grz_{name}_ca.paa').read_bytes())
        mask = decode_top_mip((SOURCE/f'gui/textures/grz_{name}_fillmask.paa').read_bytes())
        assert source.size == mask.size
        pairs=list(zip(source.get_flattened_data(),mask.get_flattened_data()))
        black=[a for ((r,g,b,opacity),(_,_,_,a)) in pairs if opacity>200 and max(r,g,b)<50]
        white=[(i//source.width,a) for i,((r,g,b,opacity),(_,_,_,a)) in enumerate(pairs) if opacity>200 and min(r,g,b)>220]
        assert black and white
        assert all(a<0.03*255 for a in black),name+' lost contour at empty'
        assert all(a<0.97*255 for _,a in white),name+' incomplete at full'
        assert not any(a<0.03*255 for _,a in white),name+' white remains at empty'
        half=[y for y,a in white if a<0.5*255]
        assert 0<len(half)<len(white)
        assert sum(half)/len(half)>sum(y for y,a in white)/len(white),name+' drains in wrong direction'
        print(f'{name}: {len(black)} black contour pixels retained at empty; white fill passes full/half/empty checks.')
    print('HUD archive matches source. Native mask rendering and value sync still require an in-game check.')


if __name__=='__main__':
    main()
