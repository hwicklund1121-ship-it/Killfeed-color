"""Build a bot source package; never include .env, tokens or local environments."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
root = Path(__file__).resolve().parents[1]
source = root / 'discord-recovery-bot'
names = ['bot.py','protection.py','test_bot.py','requirements.txt','README.md','Start-Bot.cmd','.env.example','.gitignore']
output = root / 'downloads/GRZ-Discord-Recovery-Bot.zip'
with ZipFile(output,'w',ZIP_DEFLATED) as z:
    for name in names:
        z.write(source / name, 'GRZ-Discord-Recovery-Bot/' + name)
with ZipFile(output) as z:
    assert z.testzip() is None
    assert len(z.namelist()) == len(names)
    assert not any(n.endswith('/.env') or '__pycache__' in n or '/.venv/' in n for n in z.namelist())
print(f'Built {output.name}: {output.stat().st_size} bytes; source/config templates only, no credentials.')
