"""Package the DayZ addon source without pretending to produce a compiled PBO."""
import argparse
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    paths = sorted((root / "GRZ_CustomHud").rglob("*"))
    paths += [root / "mod.cpp", root / "README.md", root / "previews/health-blood.svg", Path(__file__).resolve()]
    required = [root / "GRZ_CustomHud/config.cpp", root / "GRZ_CustomHud/$PBOPREFIX$"]
    for path in required:
        if not path.is_file():
            raise SystemExit(f"Missing addon source: {path}")
    if output in paths:
        raise SystemExit("Output must not overwrite a source file")
    output.parent.mkdir(parents=True, exist_ok=True)
    with ZipFile(output, "w", ZIP_DEFLATED) as archive:
        for path in paths:
            if path.is_file() and "__pycache__" not in path.parts:
                archive.write(path, Path("GRZ_CustomHud-source") / path.relative_to(root))
    print(f"Created source archive: {output}")


if __name__ == "__main__":
    main()
