"""Convert the supplied loading artworks and pack the standalone addon."""
import argparse
import hashlib
import io
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from PIL import Image
from build_status_textures import encode_paa
from build_pbo import build

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('images', type=Path)
args = parser.parse_args()
source = root / 'GRZ_LoadingScreen'
manifest = []
with ZipFile(args.images) as archive:
    names = sorted(n for n in archive.namelist() if n.lower().endswith('.png'))
    if not names:
        raise SystemExit('Expected PNG artwork')
    for index, name in enumerate(names, 1):
        original = archive.read(name)
        picture = Image.open(io.BytesIO(original)).convert('RGBA')
        # Native background fills the screen: power-of-two storage is rescaled
        # back to screen proportions. No cropping, overlays or artwork changes.
        target = source / 'data' / f'loading_{index}.paa'
        target.write_bytes(encode_paa(picture.resize((2048, 1024), Image.Resampling.LANCZOS), 'DXT1'))
        manifest.append(dict(source=name, original_size=picture.size,
                             source_sha256=hashlib.sha256(original).hexdigest(),
                             texture=str(target.relative_to(source))))
script_path = source / 'scripts/3_Game/GRZLoadingScreen.c'
script = script_path.read_text()
import re
loads = '\n'.join(f'        m_ImageBackground.LoadImageFile({i}, "GRZ_LoadingScreen/data/loading_{i+1}.paa");' for i in range(len(names)))
script = re.sub(r'        m_ImageBackground.LoadImageFile[^\n]+(?:\n        m_ImageBackground.LoadImageFile[^\n]+)*', lambda _: loads, script)
script = re.sub(r'Math.RandomInt\(0, \d+\)', f'Math.RandomInt(0, {len(names)})', script)
script = re.sub(r'>= (?:5|10)\.0', '>= 5.0', script)
script = script.replace('m_GRZSlideIndex = 1 - m_GRZSlideIndex;', f'm_GRZSlideIndex = (m_GRZSlideIndex + 1) % {len(names)};')
script_path.write_text(script)
for old in (source / 'data').glob('loading_*.paa'):
    if old.name not in {f'loading_{i+1}.paa' for i in range(len(names))}:
        old.unlink()
output = root / 'downloads' 
pbo = output / 'GRZ_LoadingScreen.pbo'
pbo.write_bytes(build(source))
notes = ('GRZ rotating loading screens — Made By Blake\n\n'
         'Copy GRZ_LoadingScreen.pbo into @GRZ_Local/Addons on BOTH client and server.\n'
         'Restart both. The client must load @GRZ_Local in the launcher.\n'
         'Supplied artworks rotate every 5 seconds; first slide is random.\n'
         'Short loads may finish before the second slide appears.\n'
         'Script updates can pause during blocking engine loading operations.\n'
         'Native progress bar retained. Artwork is converted, not redrawn.\n'
         'Unsigned PBO; sign with your server key if signature verification is enabled.\n'
         'Checked against DayZ 1.30 script signatures; not compiled or tested in DayZ.\n')
(root / 'LOADING-SCREEN.md').write_text(notes)
(output / 'GRZ-LOADING-SCREEN-MANIFEST.json').write_text(json.dumps(dict(slides=manifest, interval_seconds=5, pbo_sha256=hashlib.sha256(pbo.read_bytes()).hexdigest()), indent=2)+'\n')
with ZipFile(output / 'GRZ_LoadingScreen.zip', 'w', ZIP_DEFLATED) as archive:
    archive.write(pbo, 'GRZ_LoadingScreen.pbo')
    archive.writestr('INSTALL.txt', notes)
print(f'Built {pbo}: {pbo.stat().st_size} bytes; {len(names)} slides')
