"""Convert the generated white menu border into a transparent DayZ PAA."""
from pathlib import Path
from PIL import Image

from build_status_textures import encode_paa


root = Path(__file__).resolve().parents[1]
image = Image.open(root / "art/killfeed-menu-outline.png").convert("RGBA")
texture = image.resize((1024, 1024), Image.Resampling.LANCZOS)
assert texture.getpixel((512, 512))[3] == 0
output = root / "GRZ_Killfeed/gui/textures/grz_menu_outline_ca.paa"
output.write_bytes(encode_paa(texture))
print(output)
