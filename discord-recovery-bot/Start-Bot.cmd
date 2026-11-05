@echo off
cd /d "%~dp0"
if not exist .env (
  copy .env.example .env >nul
  echo Fill in DISCORD_BOT_TOKEN and SERVER_ID in .env, then run this again.
  notepad .env
  pause
  exit /b 1
)
if not exist .venv\Scripts\python.exe (
  py -3 -m venv .venv
  if errorlevel 1 goto failed
)
.venv\Scripts\python.exe -m pip install -r requirements.txt
if errorlevel 1 goto failed
.venv\Scripts\python.exe bot.py
pause
exit /b
:failed
echo Setup failed. Install Python 3.11 or newer from python.org and try again.
pause
exit /b 1
