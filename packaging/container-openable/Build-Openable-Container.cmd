@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Build-Openable-Container.ps1" %*
pause
