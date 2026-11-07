"""Build the separate GRZ Killfeed PBO and installable mod folder ZIP."""
import argparse
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile

from build_pbo import build


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output_dir.resolve()
    source = root / "GRZ_Killfeed"
    if output == source or source in output.parents:
        raise SystemExit("Build output must be outside addon source")
    output.mkdir(parents=True, exist_ok=True)
    pbo = output / "GRZ_Killfeed.pbo"
    pbo.write_bytes(build(source))
    archive = output / "GRZ_Killfeed-mod.zip"
    with ZipFile(archive, "w", ZIP_DEFLATED) as z:
        z.write(pbo, "@GRZ_Killfeed/Addons/GRZ_Killfeed.pbo")
        z.writestr("@GRZ_Killfeed/Addons/GRZ_Audit.pbo", build(root / "GRZ_Audit"))
        z.write(root / "packaging/GRZ_Killfeed/mod.cpp", "@GRZ_Killfeed/mod.cpp")
        z.write(root / "KILLFEED.md", "@GRZ_Killfeed/INSTALL.md")
        z.writestr("@GRZ_Killfeed/server-config-example.json", '{\n    "AllowedSteamIds": []\n}\n')
    print(f"Built {pbo} ({pbo.stat().st_size} bytes)")
    print(f"Built {archive}")


if __name__ == "__main__":
    main()
