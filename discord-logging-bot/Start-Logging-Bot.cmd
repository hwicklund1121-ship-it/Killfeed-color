@echo off
setlocal
cd /d "%~dp0"
if not exist .env (
 copy /y .env.example .env >nul
 echo Created .env. Fill in DISCORD_BOT_TOKEN, SERVER_ID and AUDIT_DIRECTORY, then run again.
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
if errorlevel 1 goto failed
exit /b 0
:failed
echo Bot stopped. Read the error above. Do not share your token.
pause
exit /b 1
