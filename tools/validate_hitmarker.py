"""Check hitmarker package, native option bindings, RPC contract and audio.
Does not compile Enforce Script or execute DayZ damage/UI callbacks.
"""
import io
import re
import struct
import wave
from pathlib import Path
from zipfile import ZipFile
from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'GRZ_Hitmarker'


def main():
    data = (ROOT/'downloads/GRZ_Hitmarker.pbo').read_bytes()
    assert data == build(SOURCE)
    with ZipFile(ROOT/'downloads/GRZ_Hitmarker-mod.zip') as archive:
        assert archive.testzip() is None
        assert archive.read('@GRZ_Hitmarker/Addons/GRZ_Hitmarker.pbo') == data
        assert archive.read('@GRZ_Hitmarker/mod.cpp') == (ROOT/'packaging/GRZ_Hitmarker/mod.cpp').read_bytes()
    files = {p.relative_to(SOURCE).as_posix():p.read_bytes() for p in SOURCE.rglob('*') if p.is_file()}
    widgets = set()
    scripts = []
    for name,payload in files.items():
        if name.endswith(('.layout','.c','.cpp')):
            text = payload.decode()
            assert text.count('{') == text.count('}'),name
            if name.endswith('.layout'):
                widgets.update(re.findall(r'\w+WidgetClass\s+(\w+)',text))
            if name.endswith('.c'):
                scripts.append(text)
    script='\n'.join(scripts)
    refs=set(re.findall(r'FindAnyWidget\("(\w+)"\)',script))
    assert not refs-widgets-{'game_settings_root','game_settings_scroll'},refs-widgets
    for ref in re.findall(r'"GRZ_Hitmarker/(gui/[^"\n]+)"',script):
        assert ref in files,ref
    options=files['scripts/5_Mission/GRZ_Hitmarker/GRZHitmarkerOptions.c'].decode()
    assert 'modded class OptionsMenuGame' in options
    assert 'GetLayoutName' not in options, 'Must append to the existing options page'
    assert 'FindAnyWidget("game_settings_root")' in options
    assert all('override '+method in options for method in ['bool IsChanged()', 'void Apply()', 'void Revert()', 'void SetToDefaults()'])
    names=re.findall(r'AddSelector\("(\w+)"',options)
    assert names==['ShowValue','DamageValue','SoundValue','KillsValue','BodyColorValue','HeadColorValue','SizeValue','VolumeValue']
    assert set(names)<=widgets
    assert all(f'm_Selectors[{i}].SetValue' in options for i in range(8))
    assert all(f'm_Selectors[{i}].GetValue' in options for i in range(8))
    server=files['scripts/4_World/GRZ_Hitmarker/GRZHitmarkerServer.c'].decode()
    assert re.findall(r'rpc.Write\((\w+)\)',server)==['damage','head','killed']
    assert re.findall(r'ctx.Read\(hit\.(\w+)\)',server)==['DamageAmount','IsHeadHit','IsKill']
    assert 'g_Game.IsServer() || sender || this != PlayerBase.Cast(g_Game.GetPlayer())' in server
    assert 'attacker == target' in server
    assert 'allowed && IsAlive() && GRZHitmarkerServer.IsLivingTarget(this)' in server
    assert 'beforeHealth - target.GetHealth("", "Health")' in server
    assert 'attacker.GetIdentity());' in server
    visual=files['scripts/5_Mission/GRZ_Hitmarker/GRZHitmarkerVisual.c'].decode()
    assert '.Play2D(' in visual and '.Play3D(' not in visual
    assert 'SetVolumeRelative' in visual and 'Wave.Play();' in visual
    assert 's_Voices.Count() >= 12' in visual and '.Wave.Stop()' in visual
    assert 'HUD_HIDE_FLAGS' in visual and 'GRZHitmarkerBus.Hits.Remove' in visual
    for name in ['body_hit','headshot']:
        with wave.open(io.BytesIO(files[f'sounds/{name}.wav'])) as audio:
            assert (audio.getnchannels(),audio.getsampwidth(),audio.getframerate()) == (1,2,48000)
            assert audio.getnframes()>100
            pcm=audio.readframes(audio.getnframes())
            samples=struct.unpack('<'+'h'*(len(pcm)//2),pcm)
            peak=max(abs(sample) for sample in samples)/32768
            assert 0.05<peak<0.995,(name,peak)
    cfg=files['config.cpp'].decode()
    assert 'spatial = 0;' in cfg
    for name in ['body_hit','headshot']:
        assert f'GRZ_Hitmarker/sounds/{name}' in cfg
    assert files['$PBOPREFIX$'].decode().strip()=='GRZ_Hitmarker'
    print(f'Hitmarker PBO/ZIP match source ({len(files)-1} files); native options, selectors and RPC fields checked.')
    print('Both WAVs are non-silent mono PCM, 48 kHz, below clipping; 2D playback and volume binding checked.')
    print('Static checks only: DayZ compilation, option layout, hit callback ordering and sound mixing require in-game tests.')


if __name__=='__main__':
    main()
