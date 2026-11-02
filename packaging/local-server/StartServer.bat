@echo off
cd /d "%~dp0"
DayZServer_x64.exe -config=serverDZ.cfg -port=2302 "-profiles=profiles" "-mod=@GRZ_Killfeed;@GRZ_CustomHud" -dologs -adminlog
pause
