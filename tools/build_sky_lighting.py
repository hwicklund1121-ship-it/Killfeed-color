"""Derive native lighting tables, changing only sky/skyAroundSun RGB triplets.

Input: user-supplied lighting_default.txt and lighting_darknight.txt from DayZ.
No unmodified game archives or reference files are bundled in the repository.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARRAY = re.compile(r'(?m)^(\s*(?:sky|skyAroundSun)\[\]\s*=\s*)(\{[^;\n]+\})(;)')
TRIPLET = re.compile(r'\{\s*([-+\d.eE]+)\s*,\s*([-+\d.eE]+)\s*,\s*([-+\d.eE]+)\s*\}')


def recolor(text, rgb):
    def replace(match):
        value = match[2]
        triplet = TRIPLET.search(value)
        if not triplet:
            raise ValueError('Sky array lacks an RGB triplet: ' + value)
        peak = max(float(v) for v in triplet.groups())
        new = '{' + ', '.join(format(peak*c, '.8g') for c in rgb) + '}'
        return match[1] + value[:triplet.start()] + new + value[triplet.end():] + match[3]
    result, count = ARRAY.subn(replace, text)
    assert count >= 40, count
    # Dropping sky declarations must leave every other byte unchanged.
    assert ARRAY.sub('', result) == ARRAY.sub('', text)
    return result, count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--default', type=Path, required=True)
    parser.add_argument('--darknight', type=Path, required=True)
    args = parser.parse_args()
    palette = json.loads((ROOT/'GRZ_SkyColors/data/palette.json').read_text())['PaletteEntries']
    output = ROOT/'GRZ_SkyColors/data/lighting'
    output.mkdir(exist_ok=True)
    manifest = {'source': 'User-supplied DayZ native lighting files; original engine lighting data by Bohemia Interactive.', 'tables': {}}
    for base, path in [('default', args.default), ('darknight', args.darknight)]:
        text = path.read_text(encoding='utf-8-sig')
        baseline = {'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'files': {}}
        for color in palette[1:]:
            result, count = recolor(text, color['RGB'])
            filename = base + '_' + color['Key'] + '.txt'
            (output/filename).write_text(result)
            baseline['files'][filename] = {'changed_sky_fields': count, 'sha256': hashlib.sha256(result.encode()).hexdigest()}
        manifest['tables'][base] = baseline
    (ROOT/'GRZ_SkyColors/data/lighting-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(f'Generated {2 * (len(palette) - 1)} native lighting presets; all non-sky declarations preserved.')


if __name__ == '__main__':
    main()
