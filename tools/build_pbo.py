"""Build an unsigned, uncompressed DayZ PBO and installable mod ZIP.

Uses the standard Vers extension header, prefix, file table, and SHA-1 trailer.
This packs source scripts; it does not compile Enforce Script or sign the addon.
"""
import argparse
import hashlib
import struct
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


def header(name, method=0, size=0):
    return name.encode("utf-8") + b"\0" + struct.pack("<5I", method, size, 0, 0, size)


def build(source):
    prefix = (source / "$PBOPREFIX$").read_text().strip()
    if not prefix or "\0" in prefix:
        raise ValueError("Invalid PBO prefix")
    files = sorted(
        (p for p in source.rglob("*") if p.is_file() and p.name != "$PBOPREFIX$"),
        key=lambda p: str(p.relative_to(source)).lower(),
    )
    entries = [(str(p.relative_to(source)).replace("/", "\\"), p.read_bytes()) for p in files]
    table = header("", 0x56657273) + b"prefix\0" + prefix.encode() + b"\0\0"
    table += b"".join(header(name, size=len(data)) for name, data in entries)
    body = table + header("") + b"".join(data for _, data in entries)
    return body + b"\0" + hashlib.sha1(body).digest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output_dir.resolve()
    if output == root / "GRZ_CustomHud" or root / "GRZ_CustomHud" in output.parents:
        raise SystemExit("Build output must be outside addon source")
    output.mkdir(parents=True, exist_ok=True)
    pbo = output / "GRZ_CustomHud.pbo"
    pbo.write_bytes(build(root / "GRZ_CustomHud"))
    install_notes = (
        "GRZ Custom Hud - unsigned development build\n\n"
        "Extract @GRZ_CustomHud into your DayZ folder and add it as a local mod in the launcher.\n"
        "Load with -mod=@GRZ_CustomHud. Includes weapon/ammo panel, reload prompts, stance, health and blood.\n"
        "No DayZ runtime compilation or gameplay testing has been performed.\n"
        "For signature-checking servers, sign the PBO with your own key using DayZ Tools,\n"
        "place the .bisign beside the PBO, and install the public .bikey on the server.\n"
        "No private signing key is provided.\n"
    )
    archive = output / "GRZ_CustomHud-mod.zip"
    with ZipFile(archive, "w", ZIP_DEFLATED) as z:
        z.write(pbo, "@GRZ_CustomHud/Addons/GRZ_CustomHud.pbo")
        z.write(root / "mod.cpp", "@GRZ_CustomHud/mod.cpp")
        z.writestr("@GRZ_CustomHud/INSTALL.txt", install_notes)
    print(f"Built {pbo} ({pbo.stat().st_size} bytes)")
    print(f"Built {archive}")


if __name__ == "__main__":
    main()
