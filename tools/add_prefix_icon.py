"""Package a transparent custom icon and append it to the shared icon catalog.

Example: python3 tools/add_prefix_icon.py --label Skull --slug skull --image /path/skull.png
Then rebuild with tools/build_killfeed.py. Catalog order is persistent: append,
do not reorder existing entries, because saved player selections use indices.
"""
import argparse
import json
import re
from pathlib import Path
from PIL import Image, ImageOps
from build_status_textures import encode_paa

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--label', required=True)
    parser.add_argument('--slug', required=True)
    parser.add_argument('--image', required=True, type=Path)
    args = parser.parse_args()
    if not 0 < len(args.label) <= 32 or not re.fullmatch(r'[a-z0-9_]{1,32}', args.slug):
        parser.error('Use a label of 1–32 characters and a lowercase slug of letters, digits or _.')
    source = Image.open(args.image).convert('RGBA')
    if source.getextrema()[3][0] == 255:
        parser.error('Artwork must have a transparent background.')
    bounds = source.getbbox()
    if not bounds:
        parser.error('Artwork is empty.')
    # Texture encoding only: preserve the artist-supplied color and alpha.
    fitted = ImageOps.contain(source, (128, 128), Image.Resampling.LANCZOS)
    texture = Image.new('RGBA', (128, 128))
    texture.alpha_composite(fitted, ((128 - fitted.width) // 2, (128 - fitted.height) // 2))
    relative = f'gui/textures/prefix_{args.slug}_ca.paa'
    catalog_path = ROOT / 'GRZ_Killfeed/gui/prefix_icons.json'
    catalog = json.loads(catalog_path.read_text())
    entries = catalog['Icons']
    full_path = f'GRZ_Killfeed/{relative}'
    existing = next((e for e in entries if e['Texture'] == full_path), None)
    if existing is None:
        if len(entries) >= 64:
            parser.error('Catalog supports at most 64 custom icons.')
        entries.append({'Label': args.label, 'Texture': full_path})
    else:
        existing['Label'] = args.label
    (ROOT / 'GRZ_Killfeed' / relative).write_bytes(encode_paa(texture))
    catalog_path.write_text(json.dumps(catalog, indent=2) + '\n')
    print(f'Packaged {args.label}; {len(entries)} custom icons in catalog.')


if __name__ == '__main__':
    main()
