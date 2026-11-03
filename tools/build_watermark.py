"""Convert the transparent watermark artwork to a mipmapped DayZ PAA."""
from pathlib import Path
from PIL import Image, ImageOps
from build_status_textures import encode_paa
ROOT = Path(__file__).resolve().parents[1]
image = Image.open(ROOT/'art/grz-watermark.png').convert('RGBA')
image = image.crop(image.getbbox())
texture = ImageOps.pad(image, (512, 256), method=Image.Resampling.LANCZOS, color=(0,0,0,0))
(ROOT/'GRZ_CustomHud/gui/textures/grz_watermark_ca.paa').write_bytes(encode_paa(texture))
print('Built transparent red watermark texture; HUD applies 55% opacity.')
