"""Reproducible v0.3.0 graph-test packaging from an explicitly supplied export."""
import argparse
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from build_instadrop_graph import build_graph
from build_pbo import build

ROOT = Path(__file__).resolve().parents[1]


def release(export, destination):
    build_graph(export, ROOT / 'GRZ_InstaDropGraph')
    files = {name + '.pbo': build(ROOT / name) for name in ['GRZ_InstaDrop', 'GRZ_InstaDropData', 'GRZ_InstaDropGraph']}
    manifest = {name: hashlib.sha256(data).hexdigest() for name, data in files.items()}
    destination.parent.mkdir(parents=True, exist_ok=True)
    with ZipFile(destination, 'w', ZIP_DEFLATED) as z:
        def write(name, data):
            from zipfile import ZipInfo
            info = ZipInfo(name, date_time=(2026, 11, 7, 0, 0, 0))
            info.compress_type = ZIP_DEFLATED
            z.writestr(info, data)
        for name, data in files.items():
            if name != 'GRZ_InstaDropGraph.pbo':
                write(name, data)
        write('MANIFEST.json', (json.dumps(manifest, indent=2) + '\n').encode())
        write('README.md', (ROOT / 'GRZ_InstaDrop' / 'README.md').read_bytes())
        for name in ['GRAPH-CHANGES.json']:
            write(name, (ROOT / name).read_bytes())
        for name in ['Install-InstaDrop.ps1', 'Restore-InstaDrop.ps1']:
            write(name, (ROOT / 'packaging' / 'instadrop-graph-update' / name).read_bytes())
        for name in ['Build-InstaDropGraph.ps1', 'GRZ-DropGraphTools.cs', 'GRZ-DropGraphPatch.cs', 'graph-config.cpp']:
            write(name, (ROOT / 'packaging' / 'instadrop-graph-update' / name).read_bytes())
    return manifest


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--export', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    print(json.dumps(release(a.export, a.output), indent=2))
