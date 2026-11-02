"""Validate packaged files and static UI bindings, not Enforce execution."""
import hashlib
import io
import json
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


def verify_menu_bounds(layout):
    # Absolute swatch children must stay local when top-level controls move.
    stack, nodes = [], []
    pending = None
    for line in layout.splitlines():
        line = line.strip()
        match = re.match(r"\w+WidgetClass\s+(\w+)$", line)
        if match:
            pending = {"name": match[1], "parent": next((n for n in reversed(stack) if n), None)}
            nodes.append(pending)
        elif line == "{":
            stack.append(pending)
            pending = None
        elif line == "}":
            stack.pop()
        elif stack:
            node = next((n for n in reversed(stack) if n), None)
            match = re.match(r"(position|size) ([\d.]+) ([\d.]+)$", line)
            if node and match:
                node[match[1]] = tuple(map(float, match.groups()[1:]))
    for node in nodes:
        parent = node["parent"]
        if not parent or parent["name"] == "GRZKillfeedMenuRoot":
            continue
        x, y = node["position"]
        w, h = node["size"]
        pw, ph = parent["size"]
        assert x >= 0 and y >= 0 and x + w <= pw and y + h <= ph, node["name"]


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
        if not resource.endswith("/"):
            assert resource in files, resource
    types = files["scripts/3_Game/GRZ_Killfeed/GRZKillfeedTypes.c"].decode()
    assert len(set(re.findall(r"= (164728010\d);", types))) == 4
    assert "PALETTE_COUNT = 12" in types and "EFFECT_COUNT = 6" in types
    assert not re.search(r"\b(?:int|float|bool|string)\s+(?:Effect|Weapon)\s*[;=]", text), "Engine type used as variable name"
    server = files["scripts/4_World/GRZ_Killfeed/GRZKillfeedServer.c"].decode()
    player = files["scripts/4_World/GRZ_Killfeed/GRZKillfeedPlayer.c"].decode()
    # Static wire-format alignment only; not a runtime authorization test.
    write_fields = re.findall(r"rpc.Write\(style\.([\w.]+)\)", server)
    read_fields = re.findall(r"ctx.Read\(style\.([\w.]+)\)", player)
    assert write_fields == read_fields == ["ColorA", "ColorB", "AnimationMode", "Speed", "PrefixIcon", "PrefixWord", "PrefixAnimation.ColorA", "PrefixAnimation.ColorB", "PrefixAnimation.AnimationMode", "PrefixAnimation.Speed"]
    row = files["gui/layouts/row.layout"].decode()
    assert "Outline" not in row, "Killfeed entries should have no thorn image"
    assert "color 0 0 0 0.45" in row
    for edge in ["BorderTop", "BorderBottom", "BorderLeft", "BorderRight"]:
        assert f"PanelWidgetClass {edge}" in row
    assert "gui/textures/grz_menu_logo_ca.paa" in files
    assert "gui/textures/grz_menu_outline_ca.paa" not in files
    assert "ItemPreviewWidgetClass GunPreview" in row
    mission = files["scripts/5_Mission/GRZ_Killfeed/GRZKillfeedMission.c"].decode()
    assert 'detail.SetText(message.Distance.ToString() + " m")' in mission
    assert "message.WeaponName" not in text
    assert "KC_F7" not in mission and "inputs.xml" not in files
    assert "TextWidgetClass Arrow" not in row
    assert '"text halign" center' in row
    menu = files["scripts/5_Mission/GRZ_Killfeed/GRZKillfeedMenu.c"].decode()
    request = menu.split("static void RequestMenu()", 1)[1].split("static void TestOffline()", 1)[0]
    assert request.index("OpenLocalPreview();") < request.index("if (CanLocalPreview())")
    opener = menu.split("static void OpenLocalPreview()", 1)[1].split("static void RequestMenu()", 1)[0]
    assert "EnterScriptedMenu(GRZKillfeedMenu.MENU_ID, parent)" in opener
    assert "static ref GRZKillfeedMenu ActiveMenu;" in menu
    factory = files["scripts/5_Mission/GRZ_Killfeed/GRZKillfeedMenuFactory.c"].decode()
    assert "override UIScriptedMenu CreateScriptedMenu(int id)" in factory
    assert "menu.SetID(id);" in factory
    root = files["gui/layouts/menu.layout"].decode().split("PanelWidgetClass MenuPanel", 1)[0]
    assert "hexactsize 0" in root and "vexactsize 0" in root
    assert "g_Game.GetWorkspace().GetScreenSize(width, height)" in menu
    assert "GRZKillfeedClient.ReleaseMenu();" in mission
    assert "size 580 60" in row
    assert "current.UseLocalPreview();" in mission
    assert "if (!Allowed(identity)" in server
    assert "GunWidget.SetItem(null)" in mission and "g_Game.ObjectDelete(PreviewGun)" in mission
    for index in range(12):
        for prefix in ["Swatch", "SwatchFill", "SwatchBorder", "SwatchLabel"]:
            assert prefix + str(index) in declared
    assert "animation.ColorA = i;" in menu and "animation.ColorB = i;" in menu
    assert "row.Killer.SetTextExactSize(24)" in mission
    assert "row.Victim.SetTextExactSize(24)" in mission
    focus = files["scripts/3_Game/GRZ_Killfeed/GRZKillfeedFocus.c"].decode()
    assert "override void OnDeactivateMessage()" in focus
    assert "GRZKillfeedBus.AppDeactivated.Invoke();" in focus
    assert "override void OnVisibilityChanged(bool isVisible)" in menu
    assert "key == KeyCode.KC_ESCAPE || key == KeyCode.KC_TAB" in menu
    lock = menu.split("override void LockControls()", 1)[1].split("override void UnlockControls()", 1)[0]
    unlock = menu.split("override void UnlockControls()", 1)[1].split("override void Update", 1)[0]
    assert "if (m_GRZControlsLocked)" in lock
    assert lock.index("m_GRZControlsLocked = true;") < lock.index("super.LockControls();")
    assert "if (!m_GRZControlsLocked)" in unlock
    assert unlock.index("m_GRZControlsLocked = false;") < unlock.index("super.UnlockControls();")
    closer = menu.split("void CloseSafely()", 1)[1].split("void EnsureVisible()", 1)[0]
    assert "if (m_GRZDismissRequested)" in closer
    assert closer.index("UnlockControls();") < closer.index("Close();")
    assert "GRZKillfeedBus.AppDeactivated.Remove(CloseSafely);" in closer
    assert "ResetGameFocus" not in menu
    menu_layout = files["gui/layouts/menu.layout"].decode()
    verify_menu_bounds(menu_layout)
    assert 'text "GRZ Killfeed"' in menu_layout
    assert "ImageWidgetClass MenuLogo" in menu_layout
    assert "MenuOutline" not in menu_layout
    client_fields = re.findall(r"rpc.Write\(Chosen\.([\w.]+)\)", menu)
    assert client_fields == write_fields
    for widget in ["NameTab", "PrefixTab", "PrefixControls", "PrefixWordInput", "PrefixIconChoice", "PrefixPreviewWord", "PrefixPreviewIcon"]:
        assert widget in declared
    assert "PrefixWord.Length() > 12" in types
    assert "!super.IsValid()" in types and "!PrefixAnimation.IsValid()" in types
    catalog = json.loads(files["gui/prefix_icons.json"])
    assert len(catalog["Icons"]) <= 64
    for icon in catalog["Icons"]:
        assert 0 < len(icon["Label"]) <= 32
        assert icon["Texture"].startswith("GRZ_Killfeed/gui/textures/") and ".." not in icon["Texture"]
        assert icon["Texture"].removeprefix("GRZ_Killfeed/") in files
    assert "KillerStyle.PrefixAnimation.ColorAt(Age)" in mission
    assert "VictimStyle.PrefixAnimation.ColorAt(Age)" in mission
    assert "580 + killerExtra + victimExtra" in mission
    print(f"Verified PBO prefix/checksum and {len(files)} source/asset entries; ZIP payloads match.")
    print("Layout/widget/resource bindings and style wire-field order checked.")
    print("These checks do not compile Enforce Script or test networking, permissions, or DayZ Editor.")


if __name__ == "__main__":
    main()
