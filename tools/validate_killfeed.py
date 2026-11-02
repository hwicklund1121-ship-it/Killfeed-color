"""Validate packaged files and static UI bindings, not Enforce execution."""
import hashlib
import io
import re
import struct
from pathlib import Path
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "GRZ_Killfeed"


def cstring(stream):
    result = bytearray()
    while True:
        byte = stream.read(1)
        if not byte:
            raise ValueError("Truncated PBO string")
        if byte == b"\0":
            return result.decode("utf-8")
        result.extend(byte)


def verify_pbo(path):
    data = path.read_bytes()
    assert data[-21] == 0
    assert hashlib.sha1(data[:-21]).digest() == data[-20:]
    stream = io.BytesIO(data[:-21])
    assert cstring(stream) == ""
    assert struct.unpack("<5I", stream.read(20)) == (0x56657273, 0, 0, 0, 0)
    extensions = {}
    while True:
        key = cstring(stream)
        if not key:
            break
        extensions[key] = cstring(stream)
    assert extensions["prefix"] == "GRZ_Killfeed"
    entries = []
    while True:
        name = cstring(stream)
        method, size, reserved, timestamp, stored = struct.unpack("<5I", stream.read(20))
        if not name:
            break
        assert method == 0 and size == stored and reserved == 0
        entries.append((name.replace("\\", "/"), stored))
    files = {name: stream.read(size) for name, size in entries}
    assert not stream.read()
    expected = {str(p.relative_to(SOURCE)).replace("\\", "/") for p in SOURCE.rglob("*") if p.is_file() and p.name != "$PBOPREFIX$"}
    assert set(files) == expected
    for name, payload in files.items():
        assert payload == (SOURCE / name).read_bytes(), name
    return data, files


def main():
    data, files = verify_pbo(ROOT / "downloads/GRZ_Killfeed.pbo")
    with ZipFile(ROOT / "downloads/GRZ_Killfeed-mod.zip") as archive:
        assert archive.testzip() is None
        assert archive.read("@GRZ_Killfeed/Addons/GRZ_Killfeed.pbo") == data
        assert archive.read("@GRZ_Killfeed/mod.cpp") == (ROOT / "packaging/GRZ_Killfeed/mod.cpp").read_bytes()
    declared = set()
    scripts = []
    for name, payload in files.items():
        if name.endswith(".layout"):
            text = payload.decode()
            assert text.count("{") == text.count("}"), name
            declared.update(re.findall(r"\w+WidgetClass\s+(\w+)", text))
        if name.endswith(".c"):
            text = payload.decode()
            assert text.count("{") == text.count("}"), name
            scripts.append(text)
    text = "\n".join(scripts)
    # InputEditBoxWidget belongs to vanilla's ChatInputMenu layout.
    referenced = set(re.findall(r'FindAnyWidget\("(\w+)"\)', text)) - {"InputEditBoxWidget"}
    assert not referenced - declared, referenced - declared
    for resource in re.findall(r'"GRZ_Killfeed/(gui/[^"\n]+)"', text):
        assert resource in files, resource
    types = files["scripts/3_Game/GRZ_Killfeed/GRZKillfeedTypes.c"].decode()
    assert len(set(re.findall(r"= (164728010\d);", types))) == 4
    assert "PALETTE_COUNT = 12" in types and "EFFECT_COUNT = 6" in types
    server = files["scripts/4_World/GRZ_Killfeed/GRZKillfeedServer.c"].decode()
    player = files["scripts/4_World/GRZ_Killfeed/GRZKillfeedPlayer.c"].decode()
    # Static wire-format alignment only; not a runtime authorization test.
    write_fields = re.findall(r"rpc.Write\(style\.(\w+)\)", server)
    read_fields = re.findall(r"ctx.Read\(style\.(\w+)\)", player)
    assert write_fields == read_fields == ["ColorA", "ColorB", "Effect", "Speed"]
    print(f"Verified PBO prefix/checksum and {len(files)} source/asset entries; ZIP payloads match.")
    print("Layout/widget/resource bindings and style wire-field order checked.")
    print("These checks do not compile Enforce Script or test networking, permissions, or DayZ Editor.")


if __name__ == "__main__":
    main()
