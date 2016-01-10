@echo off

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1

SET OutDir=%WORKSPACE%\Output\%CHTextVersion%
mkdir "%OutDir%"

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" "%WORKSPACE%\scripts\setup.iss" /o"%OutDir%"
