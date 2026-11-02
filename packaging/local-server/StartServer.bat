@echo off
cd /d "%~dp0"
if not exist "DayZServer_x64.exe" (
 echo Put these files in your installed DayZ Server folder beside DayZServer_x64.exe.
 pause
 exit /b 1
)
DayZServer_x64.exe -config=serverDZ.cfg -port=2302 "-profiles=profiles" "-mod=@GRZ_Local" -dologs -adminlog
pause
