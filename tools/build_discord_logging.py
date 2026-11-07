"""Package only allowlisted bot source and templates, never local credentials/state."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
r = Path(__file__).resolve().parents[1]
source = r / 'discord-logging-bot'
files = ['bot.py', 'engine.py', 'requirements.txt', 'Start-Logging-Bot.cmd',
         '.env.example', 'channel-overrides.example.json', 'README.md',
         'tests/test_engine.py', 'tests/test_bot.py']
with ZipFile(r / 'downloads/GRZ-Discord-Logging-Bot.zip', 'w', ZIP_DEFLATED) as z:
    for name in files:
        z.write(source / name, 'GRZ-Discord-Logging-Bot/' + name)
print('Built downloads/GRZ-Discord-Logging-Bot.zip (no credentials or runtime state)')
