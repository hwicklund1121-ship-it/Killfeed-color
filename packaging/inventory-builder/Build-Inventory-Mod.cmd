@echo off
cd /d "%~dp0"
powershell -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0Build-Inventory-Mod.ps1"
pause
